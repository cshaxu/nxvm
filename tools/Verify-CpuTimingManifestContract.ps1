param(
    [string[]]$ManifestPath = @(
        "docs/etc/cpu-timing/t435-s2-8086-timing-manifest.json",
        "docs/etc/cpu-timing/t512-s5-8088-timing-manifest.json",
        "docs/etc/cpu-timing/t435-s2-80186-timing-manifest.json",
        "docs/etc/cpu-timing/t435-s2-80286-timing-manifest.json",
        "docs/etc/cpu-timing/t435-s2-80386-timing-manifest.json"
    ),
    [switch]$EmitCanonicalKeys
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)
    $records = @([pscustomobject]@{ key_id = [string]$Template.id; axes = @{} })
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides", "base_selector", "id_suffix", "phase", "kind", "bytes", "count", "default_status", "string_status")) { continue }
        if ($property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($record in $records) {
            foreach ($value in $property.Value) {
                $axes = @{}; foreach ($name in $record.axes.Keys) { $axes[$name] = $record.axes[$name] }
                $axes[$property.Name] = [string]$value
                $next += [pscustomobject]@{ key_id = $record.key_id.Replace("{" + $property.Name + "}", [string]$value); axes = $axes }
            }
        }
        $records = $next
    }
    return $records
}

function New-CanonicalRecord {
    param($Profile, $Key, $Level, $SourceRule, $Route, $Status, $Batch, $Test, $Context)
    if ([string]::IsNullOrWhiteSpace($Key) -or [string]::IsNullOrWhiteSpace($Level) -or
        [string]::IsNullOrWhiteSpace($SourceRule) -or [string]::IsNullOrWhiteSpace($Route) -or
        [string]::IsNullOrWhiteSpace($Status) -or [string]::IsNullOrWhiteSpace($Batch) -or
        [string]::IsNullOrWhiteSpace($Test) -or [string]::IsNullOrWhiteSpace($Context)) {
        throw "Canonical key has an empty required field: $Key"
    }
    [pscustomobject]@{
        key_id = $Key
        profile = $Profile
        level = $Level
        encoding = "decoder-form:$Key"
        context = $Context
        source_rule = $SourceRule
        current_route = $Route
        current_ticks = "not-observed"
        source_timing_unallocated = ($Status -eq "unallocated")
        implementation_batch = $Batch
        regression_id = $Test.Replace("{id}", $Key)
        status = $Status
    }
}

$allRecords = @()
$derivedManifests = @()
foreach ($path in $ManifestPath) {
    if (-not (Test-Path -LiteralPath $path)) { throw "Manifest not found: $path" }
    $manifest = Get-Content -Raw -LiteralPath $path | ConvertFrom-Json
    if ($manifest.schema -eq "nxvm.cpu-timing-derived-manifest.v1") {
        if ([string]::IsNullOrWhiteSpace([string]$manifest.profile) -or
            [string]::IsNullOrWhiteSpace([string]$manifest.base_profile) -or
            [string]::IsNullOrWhiteSpace([string]$manifest.key_prefix_from) -or
            [string]::IsNullOrWhiteSpace([string]$manifest.key_prefix_to)) {
            throw "Unexpected derived manifest: $path"
        }
        $derivedManifests += $manifest
        continue
    }
    if ($manifest.schema -ne "nxvm.cpu-timing-manifest.v1" -or [string]::IsNullOrWhiteSpace([string]$manifest.profile)) { throw "Unexpected manifest: $path" }
    $profileRecords = @()
    $base = @{}
    foreach ($template in $manifest.base_templates) {
        foreach ($field in @("id", "level", "source_rule", "route", "status", "batch", "test")) {
            if ([string]::IsNullOrWhiteSpace([string]$template.$field)) { throw "Base template missing ${field}: $($template.id)" }
        }
        foreach ($expanded in (Expand-Template $template)) {
            if ($base.ContainsKey($expanded.key_id)) { throw "Duplicate base key: $($expanded.key_id)" }
            $status = [string]$template.status
            $override = if ($null -ne $template.overrides) {
                $template.overrides.PSObject.Properties[$expanded.key_id]
            } else { $null }
            if ($null -ne $override) { $status = [string]$override.Value }
            if ($manifest.status_values -notcontains $status) { throw "Unknown status for $($expanded.key_id)" }
            $record = New-CanonicalRecord $manifest.profile $expanded.key_id $template.level $template.source_rule $template.route $status $template.batch $template.test "BASE"
            $base[$expanded.key_id] = $record; $profileRecords += $record; $allRecords += $record
        }
    }
    if ($base.Count -ne [int]$manifest.expected.base) { throw "$($manifest.profile) base count mismatch: $($base.Count)" }
    foreach ($propertyName in @("context_key_sets", "combination_context_sets")) {
        if ($null -eq $manifest.$propertyName) { continue }
        foreach ($set in $manifest.$propertyName) {
            $defaults = if ($propertyName -eq "combination_context_sets") { $manifest.combination_defaults } else { $null }
            foreach ($field in @("id_suffix", "base_selector")) {
                if ([string]::IsNullOrWhiteSpace([string]$set.$field)) { throw "$($manifest.profile) context missing $field" }
            }
            foreach ($field in @("source_rule", "route", "batch", "test")) {
                if ([string]::IsNullOrWhiteSpace([string]$set.$field) -and [string]::IsNullOrWhiteSpace([string]$defaults.$field)) { throw "$($manifest.profile) context missing $field" }
            }
            $suffixes = @([string]$set.id_suffix)
            if ($null -ne $set.phase) { $suffixes = @($set.phase | ForEach-Object { $set.id_suffix.Replace("{phase}", [string]$_) }) }
            if ($null -ne $set.kind) { $suffixes = @($set.kind | ForEach-Object { $set.id_suffix.Replace("{kind}", [string]$_) }) }
            if ($null -ne $set.bytes) { $suffixes = @($set.bytes | ForEach-Object { $set.id_suffix.Replace("{bytes}", [string]$_) }) }
            $selector = [regex]$set.base_selector
            foreach ($baseRecord in $base.Values) {
                if (-not $selector.IsMatch($baseRecord.key_id)) { continue }
                foreach ($suffix in $suffixes) {
                    $key = "$($baseRecord.key_id)-$suffix"
                    if ($allRecords.key_id -contains $key) { throw "Duplicate generated key: $key" }
                    $status = if ($null -ne $set.status) { [string]$set.status } elseif ($baseRecord.key_id -match "-(STRING|REP)-") { [string]$set.string_status } else { [string]$set.default_status }
                    if ($manifest.status_values -notcontains $status) { throw "Unknown context status for $key" }
                    $sourceRule = if ($null -ne $set.source_rule) { $set.source_rule } else { $defaults.source_rule }
                    $route = if ($null -ne $set.route) { $set.route } else { $defaults.route }
                    $batch = if ($null -ne $set.batch) { $set.batch } else { $defaults.batch }
                    $test = if ($null -ne $set.test) { $set.test } else { $defaults.test }
                    $record = New-CanonicalRecord $manifest.profile $key $baseRecord.level $sourceRule $route $status $batch $test $suffix
                    $profileRecords += $record; $allRecords += $record
                }
            }
        }
    }
    $expectedTotal = [int]$manifest.expected.base + [int]$manifest.expected.contexts + [int]$manifest.expected.combinations
    if ($profileRecords.Count -ne $expectedTotal) {
        throw "$($manifest.profile) canonical count mismatch: actual=$($profileRecords.Count) expected=$expectedTotal"
    }
}

foreach ($manifest in $derivedManifests) {
    $parentRecords = @($allRecords | Where-Object { $_.profile -eq $manifest.base_profile })
    $expectedTotal = [int]$manifest.expected.base + [int]$manifest.expected.contexts +
        [int]$manifest.expected.combinations

    if ($parentRecords.Count -ne $expectedTotal) {
        throw "$($manifest.profile) parent corpus mismatch: actual=$($parentRecords.Count) expected=$expectedTotal"
    }
    foreach ($parent in $parentRecords) {
        $key = [string]$parent.key_id
        if (-not $key.StartsWith([string]$manifest.key_prefix_from,
                [System.StringComparison]::Ordinal)) {
            throw "$($manifest.profile) key does not have the derived prefix: $key"
        }
        $status = [string]$parent.status
        foreach ($override in @($manifest.status_overrides)) {
            if ($key -match [string]$override.key_pattern) {
                $status = [string]$override.status
                break
            }
        }
        $allRecords += [pscustomobject]@{
            key_id = ([string]$manifest.key_prefix_to) + $key.Substring(
                ([string]$manifest.key_prefix_from).Length)
            profile = [string]$manifest.profile
            level = [string]$parent.level
            encoding = [string]$parent.encoding
            context = [string]$parent.context
            source_rule = [string]$parent.source_rule
            current_route = [string]$parent.current_route
            current_ticks = "not-observed"
            source_timing_unallocated = ($status -eq "unallocated")
            implementation_batch = [string]$parent.implementation_batch
            regression_id = ([string]$parent.regression_id).Replace(
                [string]$manifest.key_prefix_from, [string]$manifest.key_prefix_to)
            status = $status
        }
    }
}

$seen = @{}
foreach ($record in $allRecords) {
    if ($seen.ContainsKey($record.key_id)) { throw "Duplicate cross-profile key: $($record.key_id)" }
    $seen[$record.key_id] = $true
}
if ($EmitCanonicalKeys) {
    $allRecords | ConvertTo-Json -Depth 5
    exit 0
}
foreach ($profile in ($allRecords.profile | Sort-Object -Unique)) {
    "$profile canonical_keys=$($allRecords.Where({ $_.profile -eq $profile }).Count)"
}
"CPU timing manifest contract: profiles=$($ManifestPath.Count) canonical_keys=$($allRecords.Count) base=$($allRecords.Where({ $_.context -eq 'BASE' }).Count) nonconforming=$($allRecords.Where({ $_.status -ne 'conforming' }).Count)"
