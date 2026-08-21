param(
    [Parameter(Mandatory = $true)]
    [string]$OutPath
)

$ErrorActionPreference = "Stop"
$records = @(& (Join-Path $PSScriptRoot "Verify-CpuTimingManifestContract.ps1") `
    -EmitCanonicalKeys | ConvertFrom-Json)
if ($records.Count -eq 1 -and $records[0] -is [System.Array]) {
    $records = @($records[0])
}
$directory = Split-Path -Parent $OutPath
if (-not [string]::IsNullOrWhiteSpace($directory)) {
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
}
$lines = @("/* Generated from the four T435 S2 manifests. Do not edit. */")
foreach ($record in $records) {
    $key = ([string]$record.key_id).Replace('"', '\"')
    $lines += "    `"$key`","
}
[System.IO.File]::WriteAllLines($OutPath, $lines,
    [System.Text.UTF8Encoding]::new($false))
