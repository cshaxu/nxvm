param(
    [Parameter(Mandatory = $true)]
    [string]$ResultPath
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$records = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
if ($records.Count -eq 1 -and $records[0] -is [System.Array]) { $records = @($records[0]) }
$i86 = @($records | Where-Object { $_.profile -eq "8086" })
if ($i86.Count -ne 651) { throw "8086 canonical corpus is $($i86.Count), expected 651" }
foreach ($key in @("I86-XLAT", "I86-XLAT-SEGMENT")) {
    if (@($i86 | Where-Object { $_.key_id -eq $key }).Count -ne 1) {
        throw "corrective decoder key missing or duplicated: $key"
    }
}
$source = Get-Content -Raw -LiteralPath (Join-Path $root "src/core/machine/machine.c")
$ledger = Get-Content -Raw -LiteralPath (Join-Path $root "docs/etc/cpu-timing/t435-s1-8086-ledger.md")
$hasOpcode = $source -match 'case 0xd7u:'
$hasForm = $source -match 'CORE_MACHINE_SOURCE_TIMING_8086_XLAT'
$hasTicks = $source -match '\*out_ticks = 11u'
$hasLedger = $ledger -match '\| `XLAT source-table` \| 11 \| exact'
if (-not ($hasOpcode -and $hasForm -and $hasTicks -and $hasLedger)) {
    throw "XLAT decoder, L3 source rule, and timing selector are not jointly present"
}
if (-not (Test-Path -LiteralPath $ResultPath)) { throw "result artifact not found: $ResultPath" }
$results = @((Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json).results)
foreach ($key in @("I86-XLAT", "I86-XLAT-SEGMENT")) {
    $result = @($results | Where-Object { $_.key_id -eq $key })
    if ($result.Count -ne 1 -or -not $result[0].passed -or $result[0].source_timing_unallocated) {
        throw "corrective decoder key lacks a classified runtime result: $key"
    }
}
Write-Output "M5:T435:S5:I86-LEDGER-DIFF:PASS:0"
