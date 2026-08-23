param(
    [Parameter(Mandatory = $true)]
    [string]$ResultPath
)

$ErrorActionPreference = "Stop"
$catalog = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
if ($catalog.Count -eq 1 -and $catalog[0] -is [System.Array]) {
    $catalog = @($catalog[0])
}
if (-not (Test-Path -LiteralPath $ResultPath)) {
    throw "CPU timing result file not found: $ResultPath"
}
$document = Get-Content -Raw -LiteralPath $ResultPath | ConvertFrom-Json
$results = if ($null -ne $document.results) { @($document.results) } else { @($document) }
if ($results.Count -eq 0) { throw "CPU timing result is empty: $ResultPath" }
$profile = if ($null -ne $document.profile) { [string]$document.profile } else {
    [string]$results[0].profile
}
$catalog = @($catalog | Where-Object { [string]$_.profile -eq $profile })
if ($catalog.Count -eq 0) { throw "CPU timing result has unknown profile: $profile" }
if ($results.Count -ne $catalog.Count) {
    throw "CPU timing result count mismatch: actual=$($results.Count) expected=$($catalog.Count) profile=$profile"
}

$expected = @{}
foreach ($record in $catalog) { $expected[[string]$record.key_id] = $record }
$seen = @{}
foreach ($result in $results) {
    foreach ($field in @("key_id", "profile", "level", "source_rule", "context",
            "ticks", "formula_inputs", "form_id", "retirement_origin",
            "source_timing_unallocated", "passed")) {
        if ($null -eq $result.PSObject.Properties[$field]) {
            throw "CPU timing result has no $field for key $($result.key_id)"
        }
    }
    $key = [string]$result.key_id
    if (-not $expected.ContainsKey($key)) { throw "CPU timing result has unknown key: $key" }
    if ($seen.ContainsKey($key)) { throw "CPU timing result is duplicated: $key" }
    foreach ($field in @("profile", "level", "source_rule", "context")) {
        if ([string]$result.$field -ne [string]$expected[$key].$field) {
            throw "CPU timing result has wrong $field for key $key"
        }
    }
    $timingDomain = if ($null -eq $result.timing_domain) { "cpu" } else {
        [string]$result.timing_domain
    }
    if ($timingDomain -notin @("cpu", "mcp")) {
        throw "CPU timing result has unknown timing domain: $key"
    }
    if ($timingDomain -eq "mcp") {
        if ($key -ne "I386-ESC" -or $result.ticks -ne $null -or
            [string]$result.handoff_kind -ne "CPU_FPU_COMMAND" -or
            [string]$result.coprocessor_profile -ne "80387" -or
            $result.escape_opcode -ne 216 -or $result.escape_modrm -ne 192 -or
            $null -eq $result.coprocessor_ticks_min -or
            $null -eq $result.coprocessor_ticks_max -or
            $result.coprocessor_ticks_min -le 0 -or
            $result.coprocessor_ticks_max -lt $result.coprocessor_ticks_min -or
            $result.source_timing_unallocated -or -not $result.passed) {
            throw "CPU timing MCP-domain result is not conforming: $key"
        }
        $seen[$key] = $true
        continue
    }
    if ($key -eq "I386-ESC" -or $result.ticks -lt 0 -or
        [string]::IsNullOrWhiteSpace([string]$result.form_id) -or
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
