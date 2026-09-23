param(
    [Parameter(Mandatory = $true)]
    [string]$BaseResultPath,
    [Parameter(Mandatory = $true)]
    [string]$ResultPath
)

$ErrorActionPreference = "Stop"

function Read-TimingResults([string]$Path, [string]$Profile) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "$Profile timing result file not found: $Path" }
    $document = Get-Content -Raw -LiteralPath $Path | ConvertFrom-Json
    if ([string]$document.schema -ne "nxvm.cpu-timing-results.v1" -or
        [string]$document.profile -ne $Profile) {
        throw "$Profile timing result document has an invalid schema or profile"
    }
    return @($document.results)
}

$records = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
$records = if ($records.Count -eq 1 -and $records[0] -is [System.Array]) {
    @($records[0])
} else {
    $records
}
$expected = @($records | Where-Object { $_.profile -eq "8088" })
if ($expected.Count -ne 1053) { throw "8088 canonical-key count mismatch: $($expected.Count)" }
$baseResults = Read-TimingResults $BaseResultPath "8086"
$results = Read-TimingResults $ResultPath "8088"
if ($baseResults.Count -ne $expected.Count -or $results.Count -ne $expected.Count) {
    throw "8088/8086 result corpus count mismatch"
}

$base = @{}
foreach ($result in $baseResults) { $base[[string]$result.key_id] = $result }
$expectedByKey = @{}
foreach ($record in $expected) { $expectedByKey[[string]$record.key_id] = $record }
$oddPeer = @{}
foreach ($key in $base.Keys) {
    if ($key -notmatch "-ODD-WORD(?:-|$)") { continue }
    $normal = $key.Replace("-ODD-WORD", "")
    if (-not $oddPeer.ContainsKey($normal)) { $oddPeer[$normal] = @() }
    $oddPeer[$normal] += $base[$key]
}
# Table 2-21's explicit T column for forms without a separately exercised
# 8086 odd-address peer.  These are source values, not Core transfer plans.
$manualTransferCountByForm = @{
    "90" = 1; "91" = 1; "92" = 2; "93" = 2; "94" = 4; "97" = 1; "99" = 2
    "101" = 1; "102" = 1; "103" = 2; "104" = 2; "105" = 1; "106" = 2
    "108" = 1; "109" = 2; "112" = 1; "113" = 1; "119" = 5; "121" = 5
    "122" = 5; "123" = 3; "133" = 1
}
$manualOddTransferCountByForm = @{
    "92" = 1; "94" = 2; "97" = 1; "99" = 2; "106" = 1; "109" = 1
}

$seen = @{}
foreach ($result in $results) {
    $key = [string]$result.key_id
    if (-not $expectedByKey.ContainsKey($key) -or $seen.ContainsKey($key)) {
        throw "8088 timing result has an unknown or duplicate key: $key"
    }
    $baseKey = "I86-" + $key.Substring(4)
    if (-not $base.ContainsKey($baseKey)) { throw "8088 result has no 8086 peer: $key" }
    $expectedRecord = $expectedByKey[$key]
    if ($result.profile -ne "8088" -or $result.level -ne $expectedRecord.level -or
        $result.source_rule -ne $expectedRecord.source_rule -or
        $result.context -ne $expectedRecord.context -or
        $result.source_timing_unallocated -or -not $result.passed -or
        [int]$result.retirement_origin -eq 11) {
        throw "8088 timing result is not source-owned and conforming: $key"
    }
    $expectedTicks = [UInt64]$base[$baseKey].ticks
    $manualBaseKey = $baseKey.Replace("-ODD-WORD", "")
    $normalBaseKey = $baseKey.Replace("-ODD-WORD", "")
    if ($baseKey -match "^I86-INTO-NOT") {
        # Table 2-21's five transfers belong only to the taken interrupt path.
    } elseif ($baseKey -match "^I86-ESC-M") {
        if (-not $base.ContainsKey($manualBaseKey)) { throw "8088 manual base is missing: $baseKey" }
        $expectedTicks = [UInt64]$base[$manualBaseKey].ticks + 4
    } elseif ($manualTransferCountByForm.ContainsKey([string]$result.form_id)) {
        $form = [string]$result.form_id
        $totalTransfers = [UInt64]$manualTransferCountByForm[$form]
        $oddTransfers = if (($result.formula_inputs -band 256) -ne 0 -and
            $manualOddTransferCountByForm.ContainsKey($form)) {
            [UInt64]$manualOddTransferCountByForm[$form]
        } else { 0 }
        $expectedTicks = [UInt64]$base[$baseKey].ticks +
            4 * ($totalTransfers - $oddTransfers)
    } elseif ($oddPeer.ContainsKey($normalBaseKey)) {
        $peers = @($oddPeer[$normalBaseKey] | Where-Object {
            [string]$_.form_id -eq [string]$result.form_id
        })
        $peerTicks = @($peers | ForEach-Object { [UInt64]$_.ticks } |
            Sort-Object -Unique)
        if ($peerTicks.Count -gt 1) { throw "8086 odd-word peer is ambiguous: $baseKey" }
        if ($peerTicks.Count -eq 1) { $expectedTicks = $peerTicks[0] }
    }
    if ([UInt64]$result.ticks -ne $expectedTicks) {
        throw "8088 Table 2-21 transfer mismatch: $key actual=$($result.ticks) expected=$expectedTicks"
    }
    $seen[$key] = $true
}
foreach ($key in $expectedByKey.Keys) {
    if (-not $seen.ContainsKey($key)) { throw "8088 timing result is missing key: $key" }
}
"8088 timing results verified: Table-2-21-keys=$($results.Count)"
