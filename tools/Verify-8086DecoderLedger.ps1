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
if ($i86.Count -ne 1053) { throw "8086 canonical corpus is $($i86.Count), expected 1053" }
foreach ($key in @("I86-XLAT", "I86-XLAT-SEGMENT", "I86-POP-SEG-CS")) {
    if (@($i86 | Where-Object { $_.key_id -eq $key }).Count -ne 1) {
        throw "corrective decoder key missing or duplicated: $key"
    }
}
$source = Get-Content -Raw -LiteralPath (Join-Path $root "src/core/machine/machine.c")
$ledger = Get-Content -Raw -LiteralPath (Join-Path $root "docs/etc/cpu-timing/t435-s1-8086-ledger.md")
$contract = Get-Content -Raw -LiteralPath (Join-Path $root "docs/etc/cpu-timing/t435-s5-8086-decoder-reconciliation-contract.md")
$decoderInventoryPath = Join-Path $root "docs/etc/cpu-timing/t435-s5-8086-decoder-inventory.json"
$requiredContractTerms = @(
    '227 direct', 'six segment/repeat prefix bytes',
    '8086-only `POP CS`', '`00`--`05`', '`F6`, `F7` `/2`--`/7`',
    '`26`, `2E`, `36`, and `3E`', '`F0` selects the +2 LOCK term'
)
foreach ($term in $requiredContractTerms) {
    if (-not $contract.Contains($term)) { throw "decoder contract is missing: $term" }
}
if (-not (Test-Path -LiteralPath $decoderInventoryPath)) {
    throw "decoder inventory artifact not found"
}
$decoderInventory = Get-Content -Raw -LiteralPath $decoderInventoryPath | ConvertFrom-Json
$expectedLexemeOpcodes = @(
    ((0x00..0x5f) + (0x70..0xbf) + (0xc2..0xc7) + (0xca..0xcf) +
        (0xd0..0xd5) + (0xd7..0xdf) + (0xe0..0xef) + (0xf2..0xff)) |
        ForEach-Object { '{0:X2}' -f $_ }
)
$actualLexemeOpcodes = @($decoderInventory.lexeme_primary_opcodes)
if ($decoderInventory.lexeme_opcode_modrm_candidates -ne 57926 -or
        $actualLexemeOpcodes.Count -ne 233 -or
        (Compare-Object -ReferenceObject $expectedLexemeOpcodes -DifferenceObject $actualLexemeOpcodes)) {
    throw "8086 decoder lexeme opcode universe differs from the reconciliation contract"
}
if (@($decoderInventory.semantic_only_prefixes).Count -ne 1 -or
        $decoderInventory.semantic_only_prefixes[0] -ne 'F0') {
    throw "8086 decoder LOCK-prefix inventory differs from the reconciliation contract"
}
$hasOpcode = $source -match 'case 0xd7u:'
$hasForm = $source -match 'CORE_MACHINE_SOURCE_TIMING_8086_XLAT'
$hasTicks = $source -match '\*out_ticks = 11u'
$hasLedger = $ledger -match '\| `XLAT source-table` \| 11 \| exact'
$hasPopCs = $source -match 'case 0x07u: case 0x0fu: case 0x17u: case 0x1fu:'
$hasPopCsLedger = $ledger -match '8086-only `POP CS` \(`0F`\)'
if (-not ($hasOpcode -and $hasForm -and $hasTicks -and $hasLedger -and $hasPopCs -and $hasPopCsLedger)) {
    throw "XLAT and 8086 POP CS decoder, L3 source rules, and timing selectors are not jointly present"
}
if (-not (Test-Path -LiteralPath $ResultPath)) { throw "result artifact not found: $ResultPath" }
$results = @((Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json).results)
$resultKeys = @($results | ForEach-Object { $_.key_id })
$canonicalKeys = @($i86 | ForEach-Object { $_.key_id })
if ($results.Count -ne $i86.Count -or
        (@($resultKeys | Sort-Object -Unique).Count -ne $resultKeys.Count) -or
        (Compare-Object -ReferenceObject ($canonicalKeys | Sort-Object) `
            -DifferenceObject ($resultKeys | Sort-Object))) {
    throw "8086 decoder/S1/S2 canonical key set differs from the runtime result set"
}
foreach ($result in $results) {
    if (-not $result.passed -or $result.source_timing_unallocated -or
            [string]::IsNullOrWhiteSpace($result.source_rule) -or
            $result.source_rule -notmatch '^S1:') {
        throw "decoder key lacks a classified S1-backed runtime result: $($result.key_id)"
    }
}
if (-not $contract.Contains('M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:OK')) {
    throw "decoder contract does not record its zero-difference closure"
}
Write-Output "M5:T435:S5:I86-XLAT-CORRECTION-SLICE:PASS"
Write-Output "M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053"
