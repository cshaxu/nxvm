param(
    [Parameter(Mandatory = $true)][string]$Compiler,
    [Parameter(Mandatory = $true)][string]$CompilerId,
    [Parameter(Mandatory = $true)][string]$Architecture
)

$ErrorActionPreference = 'Stop'
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$compilerName = if ($CompilerId -eq 'GNU') { 'gcc' } else { $CompilerId.ToLowerInvariant() }
$tree = Join-Path $repository "build/.ninja/contract-$compilerName-$Architecture"
$scratch = Join-Path $repository "build/build-contract-$Architecture"
$source = Join-Path $PSScriptRoot 'build_fixture'
$tool = Join-Path $repository 'tools/shared/Clear-Build.ps1'
$oldDirectory = $env:CCACHE_DIR
$oldTemp = $env:CCACHE_TEMPDIR
$oldSize = $env:CCACHE_MAXSIZE
New-Item -ItemType Directory -Force -Path $scratch | Out-Null

function Run-Cmake([string[]]$Arguments)
{
    $output = & cmake @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) { throw ($output -join "`n") }
    return $output
}

try {
    $ErrorActionPreference = 'Continue'
    $outside = & cmake -S $source -B "$scratch/rejected" -G Ninja 2>&1
    $outsideCode = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    if ($outsideCode -eq 0 -or ($outside -join "`n") -notlike '*must be below*') {
        throw 'Outside Ninja directory was not refused by real configuration.'
    }
    $common = @('-S', $source, '-B', $tree, '-G', 'Ninja',
        "-DCMAKE_C_COMPILER=$Compiler", '-DCMAKE_BUILD_TYPE=Release')
    Run-Cmake ($common + @('-DREPOSITORY_USE_CCACHE=OFF', '-DCMAKE_C_COMPILER_LAUNCHER=stale-launcher')) | Out-Null
    if ((Get-Content "$tree/CMakeCache.txt" -Raw) -notmatch '(?m)^CMAKE_C_COMPILER_LAUNCHER:STRING=\r?$') {
        throw 'Disabled cache retained a launcher.'
    }
    Run-Cmake @('--build', $tree, '--parallel', '8') | Out-Null
    if (Get-Command ccache -ErrorAction SilentlyContinue) {
        Run-Cmake ($common + '-DREPOSITORY_USE_CCACHE=ON') | Out-Null
        $commands = & ninja -C $tree -t commands
        foreach ($setting in @('CCACHE_DIR=', 'CCACHE_TEMPDIR=', 'CCACHE_MAXSIZE=5G')) {
            if (($commands -join "`n") -notlike "*$setting*") { throw "Missing generated cache route: $setting" }
        }
        $env:CCACHE_DIR = "$scratch/wrong-cache"
        $env:CCACHE_TEMPDIR = "$scratch/wrong-temp"
        $env:CCACHE_MAXSIZE = '1M'
        Run-Cmake @('--build', $tree, '--target', 'clean') | Out-Null
        Run-Cmake @('--build', $tree, '--parallel', '8') | Out-Null
        $object = @(Get-ChildItem "$tree/CMakeFiles/build_probe.dir" -File |
            Where-Object { $_.Extension -in @('.obj', '.o') })
        if ($object.Count -ne 1) { throw 'Missing actual compiler object.' }
        $stamp = $object.LastWriteTimeUtc
        $incremental = Run-Cmake @('--build', $tree, '--parallel', '8')
        if (($incremental -join "`n") -notlike '*no work to do*' -or
            (Get-Item $object.FullName).LastWriteTimeUtc -ne $stamp) { throw 'Unchanged build recompiled.' }
        $statsEnvironment = @('-E', 'env', "CCACHE_DIR=$repository/build/.ccache",
            "CCACHE_TEMPDIR=$repository/build/.ccache/tmp", 'CCACHE_MAXSIZE=5G', 'ccache', '--print-stats')
        $before = & cmake @statsEnvironment
        Run-Cmake @('--build', $tree, '--target', 'clean') | Out-Null
        Run-Cmake @('--build', $tree, '--parallel', '8') | Out-Null
        $after = & cmake @statsEnvironment
        $hitsBefore = 0L
        $hitsAfter = 0L
        foreach ($line in $before) { if ($line -match '^(direct|preprocessed)_cache_hit\s+(\d+)') { $hitsBefore += [long]$Matches[2] } }
        foreach ($line in $after) { if ($line -match '^(direct|preprocessed)_cache_hit\s+(\d+)') { $hitsAfter += [long]$Matches[2] } }
        if ($hitsAfter -le $hitsBefore -or (Test-Path "$scratch/wrong-cache") -or
            (Test-Path "$scratch/wrong-temp")) { throw 'Build did not use the repository cache.' }
        Write-Output "Repository cache hits added: $($hitsAfter - $hitsBefore)"
        Run-Cmake ($common + '-DREPOSITORY_USE_CCACHE=OFF') | Out-Null
        if ((Get-Content "$tree/CMakeCache.txt" -Raw) -notmatch '(?m)^CMAKE_C_COMPILER_LAUNCHER:STRING=\r?$') {
            throw 'Enabled-to-disabled configuration retained launcher.'
        }
    }
    else { Write-Output 'Optional ccache unavailable; cache-on checks not executed.' }
    Write-Output "Build environment contract ($Architecture): OK"
}
finally {
    $env:CCACHE_DIR = $oldDirectory
    $env:CCACHE_TEMPDIR = $oldTemp
    $env:CCACHE_MAXSIZE = $oldSize
    & $tool -Targets "build-contract-$Architecture" -Apply -TasksExited | Out-Null
}
