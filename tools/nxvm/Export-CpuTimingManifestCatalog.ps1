param(
    [Parameter(Mandatory = $true)]
    [string]$OutPath,
    [string]$MetadataOutPath
)

$ErrorActionPreference = "Stop"
$repositoryRoot = Split-Path -Parent $PSScriptRoot
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
$lines = @("/* Generated from the four T435 S2 manifests. Do not edit. */")
foreach ($record in $records) {
    $key = ([string]$record.key_id).Replace('"', '\"')
    $lines += "    `"$key`","
}
[System.IO.File]::WriteAllLines($OutPath, $lines,
    [System.Text.UTF8Encoding]::new($false))

if (-not [string]::IsNullOrWhiteSpace($MetadataOutPath)) {
    $metadataDirectory = Split-Path -Parent $MetadataOutPath
    if (-not [string]::IsNullOrWhiteSpace($metadataDirectory)) {
        New-Item -ItemType Directory -Force -Path $metadataDirectory | Out-Null
    }
    $metadataLines = @("/* Generated from the T435 S2 manifests. Do not edit. */")
    foreach ($record in $records) {
        $key = ([string]$record.key_id).Replace('"', '\"')
        $profile = ([string]$record.profile).Replace('"', '\"')
        $level = ([string]$record.level).Replace('"', '\"')
        $rule = ([string]$record.source_rule).Replace('"', '\"')
        $context = ([string]$record.context).Replace('"', '\"')
        $metadataLines += "    { `"$key`", `"$profile`", `"$level`", `"$rule`", `"$context`" },"
    }
    [System.IO.File]::WriteAllLines($MetadataOutPath, $metadataLines,
        [System.Text.UTF8Encoding]::new($false))
}
