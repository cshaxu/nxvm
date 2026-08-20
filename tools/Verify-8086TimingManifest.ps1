param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-8086-timing-manifest.json"
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)

    $keys = @($Template.id)
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides")) {
            continue
        }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($key in $keys) {
            foreach ($value in $property.Value) {
                $next += $key.Replace("{" + $property.Name + "}", [string]$value)
            }
        }
        $keys = $next
    }
    return $keys
}

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
if ($manifest.schema -ne "nxvm.cpu-timing-manifest.v1" -or $manifest.profile -ne "8086") {
    throw "Unexpected manifest schema or profile"
}

$seen = @{}
$counts = @{ L3 = 0; L2 = 0; conforming = 0; 'wrong-value' = 0; unallocated = 0; 'missing-input' = 0; 'missing-test' = 0 }
foreach ($template in $manifest.base_templates) {
    foreach ($field in @("id", "level", "source_rule", "route", "status", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$template.$field)) {
            throw "Template missing ${field}: $($template.id)"
        }
    }
    if ($template.level -ne "L3" -and $template.level -ne "L2:G3") {
        throw "Unknown level: $($template.level)"
    }
    foreach ($key in (Expand-Template $template)) {
        if ($seen.ContainsKey($key)) { throw "Duplicate key: $key" }
        $status = [string]$template.status
        if ($null -ne $template.overrides -and $null -ne $template.overrides.$key) {
            $status = [string]$template.overrides.$key
        }
        if ($manifest.status_values -notcontains $status) {
            throw "Unknown status $status for $key"
        }
        $seen[$key] = $true
        if ($template.level -eq "L3") { ++$counts.L3 } else { ++$counts.L2 }
        ++$counts[$status]
    }
}

if ($seen.Count -ne 244 -or $counts.L3 -ne 228 -or $counts.L2 -ne 16) {
    throw "Base-key count mismatch: total=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2)"
}
foreach ($template in $manifest.context_templates) {
    foreach ($field in @("id", "base", "level", "source_rule", "route", "status", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$template.$field)) {
            throw "Context template missing ${field}: $($template.id)"
        }
    }
}

"8086 timing manifest: base=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2) conforming=$($counts.conforming) wrong=$($counts['wrong-value']) unallocated=$($counts.unallocated) missing_input=$($counts['missing-input']) missing_test=$($counts['missing-test'])"
