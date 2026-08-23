param(
    [string]$ManifestPath = "docs/etc/cpu-timing/t435-s2-80386-timing-manifest.json"
)

$ErrorActionPreference = "Stop"

function Expand-Template {
    param($Template)
    $keys = @([string]$Template.id)
    foreach ($property in $Template.PSObject.Properties) {
        if ($property.Name -in @("id", "level", "source_rule", "route", "status", "batch", "test", "overrides") -or $property.Value -isnot [System.Array]) { continue }
        $next = @()
        foreach ($key in $keys) { foreach ($value in $property.Value) { $next += $key.Replace("{" + $property.Name + "}", [string]$value) } }
        $keys = $next
    }
    return $keys
}

function Get-Owner {
    param([string]$Key)
    if ($Key -match '^I386-(STRING|REP)-' -or $Key -match '^I386-(IN|OUT)-') { return 'S4' }
    if ($Key -match '^I386-(JCC|SETCC|INTO-NOT|STACK)-' -or $Key -match '^I386-(CALL|JMP)-.*(NEAR|FAR-REAL)' -or $Key -match '^I386-RET-(NEAR|FAR-REAL)' -or $Key -match '^I386-(INT3|INT-IMM|INTO)-REAL$') { return 'S5' }
    if ($Key -match '^I386-(CALL|JMP|RET|IRET)-' -or $Key -match '^I386-(INT3|INT-IMM|INTO)-') { return 'S6' }
    if ($Key -match '^I386-MOV-(SREG|LDS|LES|LFS|LGS|LSS|R32-(CR|DR|TR)|CR|DR|TR)' -or $Key -match '^I386-(ARPL|SYSTEM)-') { return 'S7' }
    return 'S3'
}

function Add-Canonical {
    param([hashtable]$Canonical, [string]$Key, [string]$Owner)
    if ($Canonical.ContainsKey($Key)) { throw "Duplicate canonical key: $Key" }
    $Canonical[$Key] = $Owner
}

function Add-ContextKeys {
    param($Sets, [string[]]$BaseKeys, [hashtable]$Canonical, [hashtable]$Owners)
    foreach ($set in $Sets) {
        $suffixes = @([string]$set.id_suffix)
        if ($null -ne $set.phase) { $suffixes = @($set.phase | ForEach-Object { $set.id_suffix.Replace('{phase}', [string]$_) }) }
        if ($null -ne $set.kind) { $suffixes = @($set.kind | ForEach-Object { $set.id_suffix.Replace('{kind}', [string]$_) }) }
        $selector = [regex]$set.base_selector
        foreach ($base in $BaseKeys) {
            if (-not $selector.IsMatch($base)) { continue }
            foreach ($suffix in $suffixes) { Add-Canonical $Canonical "$base-$suffix" $Owners[$base] }
        }
    }
}

if (-not (Test-Path -LiteralPath $ManifestPath)) { throw "Manifest not found: $ManifestPath" }
$manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
if ($manifest.schema -ne 'nxvm.cpu-timing-manifest.v1' -or $manifest.profile -ne '80386DX') { throw 'Unexpected manifest schema or profile' }
$baseKeys = @(); $owners = @{}; $canonical = @{}
foreach ($template in $manifest.base_templates) {
    foreach ($key in Expand-Template $template) {
        if ($owners.ContainsKey($key)) { throw "Duplicate base key: $key" }
        $owner = Get-Owner $key; $owners[$key] = $owner; $baseKeys += $key; Add-Canonical $canonical $key $owner
    }
}
if ($baseKeys.Count -ne $manifest.expected.base) { throw "Base count mismatch: $($baseKeys.Count)" }
Add-ContextKeys $manifest.context_key_sets $baseKeys $canonical $owners
if ($canonical.Count -ne ($manifest.expected.base + $manifest.expected.contexts)) { throw "Canonical count mismatch: $($canonical.Count)" }
$baseCounts = @{}; $canonicalCounts = @{}
foreach ($owner in @('S3', 'S4', 'S5', 'S6', 'S7')) {
    $baseCounts[$owner] = @($owners.Values | Where-Object { $_ -eq $owner }).Count
    $canonicalCounts[$owner] = @($canonical.Values | Where-Object { $_ -eq $owner }).Count
    if ($canonicalCounts[$owner] -eq 0) { throw "Empty implementation partition: $owner" }
}
$serialized = @($canonical.Keys | Sort-Object | ForEach-Object { "$_=$($canonical[$_])" })
if (($serialized | Select-Object -Unique).Count -ne $serialized.Count) { throw 'Partition ownership is not one-to-one' }
"M5:T437:S1:I386-PARTITION-COMPLETE:PASS:canonical=$($canonical.Count):S3=$($canonicalCounts.S3):S4=$($canonicalCounts.S4):S5=$($canonicalCounts.S5):S6=$($canonicalCounts.S6):S7=$($canonicalCounts.S7)"
"M5:T437:S1:I386-PARTITION-NO-OVERLAP:PASS:base=$($baseKeys.Count):S3=$($baseCounts.S3):S4=$($baseCounts.S4):S5=$($baseCounts.S5):S6=$($baseCounts.S6):S7=$($baseCounts.S7)"
