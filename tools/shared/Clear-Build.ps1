param(
    [Parameter(Mandatory = $true)]
    [string[]]$Targets,
    [switch]$Apply,
    [switch]$TasksExited,
    [switch]$RequireClean,
    [hashtable]$ReviewedFiles = @{}
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSHOME 'Modules/Microsoft.PowerShell.Utility')
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $repository 'build'
$prefix = $build + [IO.Path]::DirectorySeparatorChar
$comparison = [StringComparison]::OrdinalIgnoreCase

function Assert-NotLink([IO.FileSystemInfo]$Item)
{
    if ($Item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw "Refusing link/junction: $($Item.FullName)"
    }
}

if (-not (Test-Path -LiteralPath $build -PathType Container)) {
    throw 'Repository build directory does not exist.'
}
Assert-NotLink (Get-Item -LiteralPath $repository -Force)
Assert-NotLink (Get-Item -LiteralPath $build -Force)
foreach ($cache in @('.ccache', '.ninja')) {
    if (Test-Path -LiteralPath (Join-Path $build $cache)) {
        Assert-NotLink (Get-Item -LiteralPath (Join-Path $build $cache) -Force)
    }
}
if ($Apply -and -not $TasksExited) {
    throw 'Apply requires actual caller verification of task exit and -TasksExited acknowledgement.'
}

$plans = [Collections.Generic.List[object]]::new()
$paths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($target in $Targets) {
    if ([IO.Path]::IsPathRooted($target) -or $target -match '(^|[\\/])\.\.([\\/]|$)' -or
        $target -match '[:*?~]' -or $target -match '[. ]([\\/]|$)' -or
        [string]::IsNullOrWhiteSpace($target)) {
        throw "Refusing nonliteral build-relative target: $target"
    }
    $path = [IO.Path]::GetFullPath((Join-Path $build $target)).TrimEnd([IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
    if (-not $path.StartsWith($prefix, $comparison)) { throw "Target escapes build: $target" }
    $relative = $path.Substring($prefix.Length).Replace('\', '/')
    if ($relative -match '^(\.ccache|\.ninja)(/|$)') { throw "Refusing persistent cache: $target" }
    $ancestor = [IO.Path]::GetDirectoryName($path)
    while ($ancestor -ne $build) {
        if (Test-Path -LiteralPath $ancestor) { Assert-NotLink (Get-Item -LiteralPath $ancestor -Force) }
        $ancestor = [IO.Path]::GetDirectoryName($ancestor)
    }
    if (-not $paths.Add($path)) { throw "Overlapping targets: $target" }
    $pending = [Collections.Generic.Stack[IO.FileSystemInfo]]::new()
    $pending.Push((Get-Item -LiteralPath $path -Force))
    $bytes = 0L
    $files = 0
    while ($pending.Count) {
        $item = $pending.Pop()
        Assert-NotLink $item
        if ($item.Name -in @('.git', '.gitmodules', '.gitconfig') -or
            ($item -is [IO.DirectoryInfo] -and
             (Test-Path -LiteralPath (Join-Path $item.FullName 'HEAD')) -and
             (Test-Path -LiteralPath (Join-Path $item.FullName 'objects')))) {
            throw "Refusing Git data; audit provenance and local commits first: $($item.FullName)"
        }
        if ($item -is [IO.DirectoryInfo]) {
            foreach ($child in $item.EnumerateFileSystemInfos()) { $pending.Push($child) }
        }
        else {
            $key = $item.FullName.Substring($prefix.Length).Replace('\', '/')
            if ($item.Extension -match '^\.(ini|cfg|conf|toml|yaml|yml|rom|bin|nes|mns|sav|img|ima|iso|dsk|vhdx?|vdi|vmdk|qcow2|raw|flp|pdf|zip|7z|tar)$') {
                if (-not $ReviewedFiles.ContainsKey($key) -or
                    $ReviewedFiles[$key] -notmatch '^[0-9a-fA-F]{64}$' -or
                    (Get-FileHash -LiteralPath $item.FullName -Algorithm SHA256).Hash -ne $ReviewedFiles[$key]) {
                    throw "Unreviewed config/media/research/archive: $key"
                }
            }
            $bytes += $item.Length
            ++$files
        }
    }
    $plans.Add([pscustomobject]@{ Path = $path; Relative = $relative; Bytes = $bytes; Files = $files })
}
foreach ($plan in $plans) {
    $ancestor = [IO.Path]::GetDirectoryName($plan.Path)
    while ($ancestor -ne $build) {
        if ($paths.Contains($ancestor)) { throw "Overlapping targets: $($plan.Relative)" }
        $ancestor = [IO.Path]::GetDirectoryName($ancestor)
    }
}

# Every candidate has been checked before the first removal. The caller owns
# exclusion of concurrent writers; TasksExited is not a process detector.
if ($RequireClean) {
    $remaining = @(Get-ChildItem -LiteralPath $build -Force | Where-Object {
        $_.Name -notin @('.ccache', '.ninja') -and -not $paths.Contains($_.FullName)
    })
    if ($remaining.Count) { throw 'Targets do not cover all disposable build children.' }
}
foreach ($plan in $plans) {
    if ($Apply) {
        Remove-Item -LiteralPath $plan.Path -Recurse -Force
        if (Test-Path -LiteralPath $plan.Path) { throw "Target remains: $($plan.Relative)" }
    }
    [pscustomobject]@{ Target = $plan.Relative; Bytes = $plan.Bytes; Files = $plan.Files; Deleted = [bool]$Apply }
}
if ($RequireClean -and $Apply -and @(Get-ChildItem -LiteralPath $build -Force |
        Where-Object { $_.Name -notin @('.ccache', '.ninja') }).Count) {
    throw 'Disposable build content remains after cleanup.'
}
