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
        $seen[$key] = [string]$template.level
        if ($template.level -eq "L3") { ++$counts.L3 } else { ++$counts.L2 }
        ++$counts[$status]
    }
}

if ($seen.Count -ne $manifest.expected.base -or $counts.L3 -ne $manifest.expected.L3 -or $counts.L2 -ne $manifest.expected.L2) {
    throw "Base-key count mismatch: total=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2)"
}
foreach ($template in $manifest.context_templates) {
    foreach ($field in @("id", "base", "level", "source_rule", "route", "status", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$template.$field)) {
            throw "Context template missing ${field}: $($template.id)"
        }
    }
}

$contextSeen = @{}
$contextCounts = @{ total = 0; L3 = 0; L2 = 0; unallocated = 0; 'missing-input' = 0 }
foreach ($set in $manifest.context_key_sets) {
    foreach ($field in @("id_suffix", "base_selector", "source_rule", "route", "batch", "test")) {
        if ([string]::IsNullOrWhiteSpace([string]$set.$field)) {
            throw "Context key set missing ${field}: $($set.id_suffix)"
        }
    }
    $selector = [regex]$set.base_selector
    foreach ($base in $seen.Keys) {
        if ($set.base_selector -ne "all" -and -not $selector.IsMatch($base)) { continue }
        $key = "$base-$($set.id_suffix)"
        if ($contextSeen.ContainsKey($key)) { throw "Duplicate context key: $key" }
        $status = if ($set.base_selector -eq "all" -and
            ($base.StartsWith("I86-STRING-") -or $base.StartsWith("I86-REP-"))) {
            [string]$set.string_status
        } elseif ($null -ne $set.status) {
            [string]$set.status
        } else {
            [string]$set.default_status
        }
        if ($manifest.status_values -notcontains $status) {
            throw "Unknown context status $status for $key"
        }
        $contextSeen[$key] = $true
        ++$contextCounts.total
        if ($seen[$base] -eq "L3") { ++$contextCounts.L3 } else { ++$contextCounts.L2 }
        if ($contextCounts.ContainsKey($status)) { ++$contextCounts[$status] }
    }
}
if ($contextSeen.Count -ne $manifest.expected.contexts -or $contextCounts.L3 -ne $manifest.expected.context_L3 -or $contextCounts.L2 -ne $manifest.expected.context_L2) {
    throw "Context-key count mismatch: total=$($contextSeen.Count) unallocated=$($contextCounts.unallocated) missing_input=$($contextCounts['missing-input'])"
}

$combinationSeen = @{}
$combinationCounts = @{ total = 0; L3 = 0; L2 = 0; unallocated = 0; 'missing-input' = 0; 'missing-test' = 0 }
foreach ($field in @("level", "source_rule", "route", "batch", "test")) {
    if ([string]::IsNullOrWhiteSpace([string]$manifest.combination_defaults.$field)) {
        throw "Combination defaults missing ${field}"
    }
}
foreach ($set in $manifest.combination_context_sets) {
    foreach ($field in @("id_suffix", "base_selector", "count")) {
        if ([string]::IsNullOrWhiteSpace([string]$set.$field)) {
            throw "Combination context set missing ${field}: $($set.id_suffix)"
        }
    }
    $suffixes = @($set.id_suffix)
    if ($null -ne $set.phase) {
        $suffixes = @()
        foreach ($phase in $set.phase) {
            $suffixes += $set.id_suffix.Replace("{phase}", [string]$phase)
        }
    }
    $setSeen = 0
    $selector = [regex]$set.base_selector
    foreach ($base in $seen.Keys) {
        if (-not $selector.IsMatch($base)) { continue }
        foreach ($suffix in $suffixes) {
            $key = "$base-$suffix"
            if ($combinationSeen.ContainsKey($key)) { throw "Duplicate combination key: $key" }
            $status = if ($null -ne $set.status) { [string]$set.status } elseif
                ($base.StartsWith("I86-STRING-") -or $base.StartsWith("I86-REP-")) {
                [string]$set.string_status
            } else { [string]$set.default_status }
            if ($manifest.status_values -notcontains $status) {
                throw "Unknown combination status $status for $key"
            }
            $combinationSeen[$key] = $true
            ++$combinationCounts.total
            if ($seen[$base] -eq "L3") { ++$combinationCounts.L3 } else { ++$combinationCounts.L2 }
            ++$combinationCounts[$status]
            ++$setSeen
        }
    }
    if ($setSeen -ne [int]$set.count) {
        throw "Combination count mismatch for $($set.id_suffix): actual=$setSeen expected=$($set.count)"
    }
}
if ($combinationSeen.Count -ne $manifest.expected.combinations -or $combinationCounts.L3 -ne $manifest.expected.combination_L3 -or $combinationCounts.L2 -ne $manifest.expected.combination_L2) {
    throw "Combination-key count mismatch: total=$($combinationSeen.Count) unallocated=$($combinationCounts.unallocated) missing_input=$($combinationCounts['missing-input']) missing_test=$($combinationCounts['missing-test'])"
}

"8086 timing manifest: base=$($seen.Count) L3=$($counts.L3) L2=$($counts.L2) conforming=$($counts.conforming) wrong=$($counts['wrong-value']) unallocated=$($counts.unallocated) missing_input=$($counts['missing-input']) missing_test=$($counts['missing-test'])"
"8086 timing contexts: total=$($contextCounts.total) L3=$($contextCounts.L3) L2=$($contextCounts.L2) unallocated=$($contextCounts.unallocated) missing_input=$($contextCounts['missing-input'])"
"8086 timing combinations: total=$($combinationCounts.total) L3=$($combinationCounts.L3) L2=$($combinationCounts.L2) unallocated=$($combinationCounts.unallocated) missing_input=$($combinationCounts['missing-input']) missing_test=$($combinationCounts['missing-test'])"
