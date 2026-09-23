param(
    [Parameter(Mandatory = $true)]
    [string]$ResultPath
)

$ErrorActionPreference = "Stop"

$records = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
if ($records.Count -eq 1 -and $records[0] -is [System.Array]) {
    $records = @($records[0])
}
$expected = @($records | Where-Object { $_.profile -eq "80286" })
if ($expected.Count -eq 0) {
    throw "80286 manifest has no canonical keys"
}
if (-not (Test-Path -LiteralPath $ResultPath)) {
    throw "80286 timing result file not found: $ResultPath"
}

$document = Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json
if ([string]$document.schema -ne "nxvm.cpu-timing-results.v1" -or
        [string]$document.profile -ne "80286") {
    throw "80286 timing result document has an invalid schema or profile"
}
$results = @($document.results)
if ($results.Count -ne $expected.Count) {
    throw "80286 timing result count mismatch: actual=$($results.Count) expected=$($expected.Count)"
}

$expectedByKey = @{}
foreach ($record in $expected) { $expectedByKey[[string]$record.key_id] = $record }
$seen = @{}
foreach ($result in $results) {
    foreach ($field in @("key_id", "profile", "level", "source_rule", "context",
            "ticks", "formula_inputs", "form_id", "retirement_origin",
            "source_timing_unallocated", "passed")) {
        if ($null -eq $result.$field) {
            throw "80286 timing result has no $field for key $($result.key_id)"
        }
    }
    $key = [string]$result.key_id
    if (-not $expectedByKey.ContainsKey($key)) {
        throw "80286 timing result has unknown key: $key"
    }
    if ($seen.ContainsKey($key)) { throw "80286 timing result is duplicated: $key" }
    $record = $expectedByKey[$key]
    if ($result.profile -ne "80286" -or $result.level -ne $record.level -or
            $result.source_rule -ne $record.source_rule -or
            $result.context -ne $record.context) {
        throw "80286 timing result provenance mismatch: $key"
    }
    if ($result.ticks -lt 0 -or [string]::IsNullOrWhiteSpace([string]$result.form_id) -or
            [string]::IsNullOrWhiteSpace([string]$result.retirement_origin) -or
            $result.source_timing_unallocated -or -not $result.passed) {
        throw "80286 timing result is not conforming: $key"
    }
    $requiredInputs = 0
    if ($result.context -match "LOCK") { $requiredInputs = $requiredInputs -bor 32 }
    if ($result.context -match "SEGMENT") { $requiredInputs = $requiredInputs -bor 128 }
    if ($result.context -match "EA-BID") { $requiredInputs = $requiredInputs -bor 64 }
    if ($result.context -match "ODD-WORD" -and $result.context -notmatch "REP-PHASE-ZERO") {
        $requiredInputs = $requiredInputs -bor 256
    }
    if ($result.context -match "REP-PHASE") {
        $requiredInputs = $requiredInputs -bor 4 -bor 512
    }
    if ($result.context -match "NEXT-BYTE") { $requiredInputs = $requiredInputs -bor 2 }
    if (($result.formula_inputs -band $requiredInputs) -ne $requiredInputs) {
        throw "80286 timing result is missing required formula inputs: $key"
    }
    $seen[$key] = $true
}
foreach ($key in $expectedByKey.Keys) {
    if (-not $seen.ContainsKey($key)) { throw "80286 timing result is missing key: $key" }
}
"80286 timing results verified: conforming_keys=$($results.Count)"
