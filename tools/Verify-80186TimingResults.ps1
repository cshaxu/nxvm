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
$expectedRecords = @($records | Where-Object { $_.profile -eq "80186" })
if ($expectedRecords.Count -ne 616) {
    throw "80186 canonical-key count mismatch: $($expectedRecords.Count)"
}
if (-not (Test-Path -LiteralPath $ResultPath)) {
    throw "80186 timing result file not found: $ResultPath"
}

$document = Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json
if ([string]$document.schema -ne "nxvm.cpu-timing-results.v1" -or
        [string]$document.profile -ne "80186") {
    throw "80186 timing result document has an invalid schema or profile"
}
$results = if ($null -ne $document.results) { @($document.results) } else { @($document) }
if ($results.Count -ne $expectedRecords.Count) {
    throw "80186 timing result count mismatch: actual=$($results.Count) expected=$($expectedRecords.Count)"
}

$expected = @{}
foreach ($record in $expectedRecords) { $expected[[string]$record.key_id] = $record }
$seen = @{}
foreach ($result in $results) {
    foreach ($field in @("key_id", "profile", "level", "source_rule", "context", "ticks",
            "formula_inputs", "form_id", "retirement_origin", "source_timing_unallocated",
            "passed")) {
        if ($null -eq $result.$field) {
            throw "80186 timing result has no $field for key $($result.key_id)"
        }
    }
    $key = [string]$result.key_id
    if (-not $expected.ContainsKey($key)) {
        throw "80186 timing result has unknown key: $key"
    }
    if ($seen.ContainsKey($key)) {
        throw "80186 timing result is duplicated: $key"
    }
    $record = $expected[$key]
    if ($result.profile -ne "80186" -or $result.level -ne $record.level -or
            $result.source_rule -ne $record.source_rule -or
            $result.context -ne $record.context) {
        throw "80186 timing result provenance mismatch: $key"
    }
    if ($result.ticks -lt 0 -or [string]::IsNullOrWhiteSpace([string]$result.form_id) -or
            [string]::IsNullOrWhiteSpace([string]$result.retirement_origin) -or
            $result.source_timing_unallocated -or -not $result.passed) {
        throw "80186 timing result is not conforming: $key"
    }
    $requiredInputs = 0
    if ($key -match "-LOCK(?:-|$)") { $requiredInputs = $requiredInputs -bor 32 }
    if ($key -match "-SEGMENT(?:-|$)") { $requiredInputs = $requiredInputs -bor 128 }
    # REP zero-count performs setup only: the manual's odd-address term belongs
    # to a 16-bit transfer and therefore is not an input for that phase.
    if ($key -match "-ODD-WORD(?:-|$)" -and
            $key -notmatch "-REP-PHASE-ZERO$") {
        $requiredInputs = $requiredInputs -bor 256
    }
    if ($key -match "^I186-REP-") { $requiredInputs = $requiredInputs -bor 4 -bor 512 }
    if (($result.formula_inputs -band $requiredInputs) -ne $requiredInputs) {
        throw "80186 timing result is missing required formula inputs: $key"
    }
    $seen[$key] = $true
}
foreach ($key in $expected.Keys) {
    if (-not $seen.ContainsKey($key)) { throw "80186 timing result is missing key: $key" }
}
"80186 timing results verified: conforming_keys=$($results.Count)"
