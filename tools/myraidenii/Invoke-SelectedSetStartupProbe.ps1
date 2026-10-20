param(
    [Parameter(Mandatory = $true)]
    [string]$Inventory,
    [Parameter(Mandatory = $true)]
    [string]$Archive,
    [Parameter(Mandatory = $true)]
    [string]$Probe,
    [Parameter(Mandatory = $true)]
    [string]$TemporaryRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Require([bool]$Condition, [string]$Message)
{
    if (-not $Condition) { throw $Message }
}

$verifier = Join-Path $PSScriptRoot 'Verify-RomInventory.ps1'
$inventoryPath = [System.IO.Path]::GetFullPath($Inventory)
$archivePath = [System.IO.Path]::GetFullPath($Archive)
$probePath = [System.IO.Path]::GetFullPath($Probe)
$rootPath = [System.IO.Path]::GetFullPath($TemporaryRoot)
Require (Test-Path -LiteralPath $verifier -PathType Leaf) 'ROM inventory verifier is missing'
Require (Test-Path -LiteralPath $probePath -PathType Leaf) 'Startup probe executable is missing'

# The verifier owns identity validation; its structured success record has no ROM payload.
& $verifier -Inventory $inventoryPath -Archive $archivePath | Out-Null

$manifest = Get-Content -Raw -LiteralPath $inventoryPath | ConvertFrom-Json
$requiredRoles = @(
    'program-even', 'program-odd', 'characters', 'background-1', 'background-2',
    'sprites-1', 'sprites-2', 'sprites-3', 'sprites-4', 'sound-program'
)
$required = @()
foreach ($role in $requiredRoles) {
    $matches = @($manifest.runtime_images | Where-Object { $_.role -ceq $role })
    Require ($matches.Count -eq 1) "Inventory does not define exactly one $role image"
    $required += $matches[0]
}
[System.IO.Directory]::CreateDirectory($rootPath) | Out-Null
$stageName = 'selected-set-startup-' + [Guid]::NewGuid().ToString('N')
$stagePath = [System.IO.Path]::Combine($rootPath, $stageName)
Require ($stagePath.StartsWith($rootPath + [System.IO.Path]::DirectorySeparatorChar,
    [System.StringComparison]::OrdinalIgnoreCase)) 'Unsafe temporary path'
[System.IO.Directory]::CreateDirectory($stagePath) | Out-Null

try {
    $archiveStream = [System.IO.File]::Open($archivePath, [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
    try {
        $zip = [System.IO.Compression.ZipArchive]::new($archiveStream,
            [System.IO.Compression.ZipArchiveMode]::Read, $true)
        try {
            $images = @{}
            foreach ($image in $required) {
                $entry = $zip.GetEntry([string]$image.name)
                Require ($null -ne $entry -and $entry.FullName -ceq $image.name) `
                    "Missing verified image: $($image.name)"
                $destination = [System.IO.Path]::Combine($stagePath, [string]$image.name)
                $input = $entry.Open()
                try {
                    $output = [System.IO.File]::Open($destination, [System.IO.FileMode]::CreateNew,
                        [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
                    try { $input.CopyTo($output) } finally { $output.Dispose() }
                }
                finally { $input.Dispose() }
                Require ((Get-Item -LiteralPath $destination).Length -eq $image.bytes) `
                    "Staged image length mismatch: $($image.name)"
                $images[$image.role] = $destination
            }
        }
        finally { $zip.Dispose() }
    }
    finally { $archiveStream.Dispose() }
    & $probePath $images['program-even'] $images['program-odd'] $images['characters'] `
        $images['background-1'] $images['background-2'] $images['sprites-1'] `
        $images['sprites-2'] $images['sprites-3'] $images['sprites-4'] `
        $images['sound-program']
    if ($LASTEXITCODE -ne 0) { throw "Startup probe failed: $LASTEXITCODE" }
}
finally {
    if (Test-Path -LiteralPath $stagePath) {
        $item = Get-Item -LiteralPath $stagePath -Force
        Require ($null -eq $item.LinkType) 'Refusing linked temporary path'
        Remove-Item -LiteralPath $stagePath -Recurse -Force
    }
}
