param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80186-timing-manifest.json"
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)
    $keys = @($Template.id)
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides", "base_selector", "id_suffix")) { continue }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($key in $keys) {
            foreach ($value in $property.Value) { $next += $key.Replace("{" + $property.Name + "}", [string]$value) }
        }
        $keys = $next
    }
    return $keys
}

if (-not (Test-Path -LiteralPath $ManifestPath)) { throw "Manifest not found: $ManifestPath" }
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
if ($manifest.schema -ne "nxvm.cpu-timing-manifest.v1" -or $manifest.profile -ne "80186") { throw "Unexpected manifest schema or profile" }

$seen = @{}
$counts = @{ L3 = 0; L2 = 0; conforming = 0; 'wrong-value' = 0; unallocated = 0; 'missing-input' = 0; 'missing-test' = 0 }
foreach ($template in $manifest.base_templates) {
    foreach ($field in @("id", "level", "source_rule", "route", "status", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$template.$field)) { throw "Template missing ${field}: $($template.id)" }
    }
    if ($template.level -ne "L3" -and $template.level -ne "L2:midpoint") { throw "Unknown level: $($template.level)" }
    foreach ($key in (Expand-Template $template)) {
        if ($seen.ContainsKey($key)) { throw "Duplicate key: $key" }
        $status = if ($null -ne $template.overrides -and $null -ne $template.overrides.$key) { [string]$template.overrides.$key } else { [string]$template.status }
        if ($manifest.status_values -notcontains $status) { throw "Unknown status $status for $key" }
        $seen[$key] = [string]$template.level
        if ($template.level -eq "L3") { ++$counts.L3 } else { ++$counts.L2 }
        ++$counts[$status]
    }
}
if ($seen.Count -ne 281 -or $counts.L3 -ne 256 -or $counts.L2 -ne 25) { throw "Base-key count mismatch: total=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2)" }
if ($manifest.expected.base -ne $seen.Count -or $manifest.expected.L3 -ne $counts.L3 -or $manifest.expected.L2 -ne $counts.L2) { throw "Expected counts disagree with expansion" }

$contexts = @{}
foreach ($set in $manifest.context_key_sets) {
    foreach ($field in @("id_suffix", "base_selector", "source_rule", "route", "status", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$set.$field)) { throw "Context set missing ${field}: $($set.id_suffix)" }
    }
    $suffixes = @($set.id_suffix)
    if ($null -ne $set.phase) { $suffixes = @($set.phase | ForEach-Object { $set.id_suffix.Replace("{phase}", [string]$_) }) }
    $selector = [regex]$set.base_selector
    foreach ($base in $seen.Keys) {
        if (-not $selector.IsMatch($base)) { continue }
        foreach ($suffix in $suffixes) {
            $key = "$base-$suffix"
            if ($contexts.ContainsKey($key)) { throw "Duplicate context key: $key" }
            $contexts[$key] = $true
        }
    }
}
"80186 timing manifest: base=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2) conforming=$($counts.conforming) wrong=$($counts['wrong-value']) unallocated=$($counts.unallocated) missing_input=$($counts['missing-input']) missing_test=$($counts['missing-test']) contexts=$($contexts.Count)"
