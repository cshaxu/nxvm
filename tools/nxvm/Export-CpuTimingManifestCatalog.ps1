param(
    [Parameter(Mandatory = $true)]
    [string]$OutPath
)

$ErrorActionPreference = "Stop"
$repositoryRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Push-Location -LiteralPath $repositoryRoot
try {
    $records = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
        -EmitCanonicalKeys | ConvertFrom-Json)
} finally {
    Pop-Location
}
if ($records.Count -eq 1 -and $records[0] -is [System.Array]) {
    $records = @($records[0])
}
$directory = Split-Path -Parent $OutPath
if (-not [string]::IsNullOrWhiteSpace($directory)) {
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
}
$lines = @("/* Generated from the five canonical CPU timing manifests. Do not edit. */")
foreach ($record in $records) {
    $key = ([string]$record.key_id).Replace('"', '\"')
    $profile = ([string]$record.profile).Replace('"', '\"')
    $level = ([string]$record.level).Replace('"', '\"')
    $rule = ([string]$record.source_rule).Replace('"', '\"')
    $context = ([string]$record.context).Replace('"', '\"')
    $lines += "    { `"$key`", `"$profile`", `"$level`", `"$rule`", `"$context`" },"
}
[System.IO.File]::WriteAllLines($OutPath, $lines,
    [System.Text.UTF8Encoding]::new($false))
