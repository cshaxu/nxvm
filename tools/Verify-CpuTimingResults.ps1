param(
    [Parameter(Mandatory = $true)]
    [string]$ResultPath
)

$ErrorActionPreference = "Stop"
$catalog = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
if (-not (Test-Path -LiteralPath $ResultPath)) {
    throw "CPU timing result file not found: $ResultPath"
}
$document = Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json
$results = if ($null -ne $document.results) { @($document.results) } else { @($document) }
if ($results.Count -ne $catalog.Count) {
    throw "CPU timing result count mismatch: actual=$($results.Count) expected=$($catalog.Count)"
}

$expected = @{}
foreach ($record in $catalog) { $expected[[string]$record.key_id] = $record }
$seen = @{}
foreach ($result in $results) {
    foreach ($field in @("key_id", "ticks", "formula_inputs", "form_id",
            "retirement_origin", "source_timing_unallocated", "passed")) {
        if ($null -eq $result.$field) {
            throw "CPU timing result has no $field for key $($result.key_id)"
        }
    }
    $key = [string]$result.key_id
    if (-not $expected.ContainsKey($key)) { throw "CPU timing result has unknown key: $key" }
    if ($seen.ContainsKey($key)) { throw "CPU timing result is duplicated: $key" }
    if ($result.ticks -lt 0 -or [string]::IsNullOrWhiteSpace([string]$result.form_id) -or
        [string]::IsNullOrWhiteSpace([string]$result.retirement_origin) -or
        $result.source_timing_unallocated -or -not $result.passed) {
        throw "CPU timing result is not conforming: $key"
    }
    $seen[$key] = $true
}
foreach ($key in $expected.Keys) {
    if (-not $seen.ContainsKey($key)) { throw "CPU timing result is missing key: $key" }
}
"CPU timing results verified: conforming_keys=$($results.Count)"
