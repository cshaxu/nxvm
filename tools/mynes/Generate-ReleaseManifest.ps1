param(
    [string]$RepositoryRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)),
    [switch]$Verify
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path $RepositoryRoot).Path
$binary = Join-Path $root 'assets/binary-mynes'
$manifest = Join-Path $root 'build\mynes_0_0_0039_manifest.txt'
$artifacts = @('mynes_0_0_0039_x64.exe', 'mynes_0_0_0039_x86.exe', 'mynes.ini')

function Get-ReleaseHash([string]$Path) {
    (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-ToolchainIdentity([string]$BuildDirectory) {
    $cache = Join-Path $root "$BuildDirectory/CMakeCache.txt"
    $compilerLine = Get-Content -LiteralPath $cache | Where-Object {
        $_ -match '^CMAKE_C_COMPILER(?::[^=]+)?='
    } | Select-Object -First 1
    if ($null -eq $compilerLine) { throw "C compiler is absent from $BuildDirectory" }
    $compiler = $compilerLine.Substring($compilerLine.IndexOf('=') + 1)
    $target = (& $compiler -dumpmachine).Trim()
    $version = (& $compiler -dumpfullversion -dumpversion).Trim()
    if ($target.Length -eq 0 -or $version.Length -eq 0) {
        throw "C compiler identity is unavailable for $BuildDirectory"
    }
    return "$target gcc $version"
}

function Get-ManifestLines([string]$Revision) {
    $cmake = ((& cmake --version)[0] -replace '^cmake version ', '').Trim()
    $lines = @(
        '# MyNes release manifest',
        'format = 1',
        'product = MyNes',
        'version = 0.0.0039',
        "source_revision = $revision",
        "cmake = $cmake",
        "toolchain_x64 = $(Get-ToolchainIdentity 'build/mynes-gcc-x64-release')",
        "toolchain_x86 = $(Get-ToolchainIdentity 'build/mynes-gcc-x86-release')",
        'configuration = mynes.ini'
    )
    foreach ($artifact in $artifacts) {
        $path = Join-Path $binary $artifact
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "release artifact is missing: $artifact"
        }
        $lines += "sha256 $artifact $(Get-ReleaseHash $path)"
    }
    return $lines
}

if ($Verify) {
    if (-not (Test-Path -LiteralPath $manifest -PathType Leaf)) {
        throw 'release manifest is missing'
    }
    $actual = Get-Content -LiteralPath $manifest
    $revisionLine = $actual | Where-Object { $_ -like 'source_revision = *' }
    if ($null -eq $revisionLine -or @($revisionLine).Count -ne 1) {
        throw 'release manifest has no unique source revision'
    }
    $expected = Get-ManifestLines ($revisionLine.Substring('source_revision = '.Length))
    if (@(Compare-Object $expected $actual -SyncWindow 0).Count -ne 0) {
        throw 'release manifest does not match the current artifacts and build identity'
    }
    Write-Output 'Release manifest verified.'
    exit 0
}
$expected = Get-ManifestLines ((& git -C $root rev-parse HEAD).Trim())
[System.IO.File]::WriteAllText($manifest, (($expected -join "`n") + "`n"),
    [System.Text.UTF8Encoding]::new($false))
Write-Output "Release manifest generated: $manifest"
