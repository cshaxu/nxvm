param(
    [Parameter(Mandatory = $true)][string]$Compiler,
    [Parameter(Mandatory = $true)][string]$CompilerId,
    [Parameter(Mandatory = $true)][string]$Architecture
)

$ErrorActionPreference = 'Stop'
$repository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$compilerName = if ($CompilerId -eq 'GNU') { 'gcc' } else { $CompilerId.ToLowerInvariant() }
$tree = Join-Path $repository "build/.ninja/contract-$compilerName-$Architecture"
$scratchName = "build-contract-$Architecture-$PID"
$scratch = Join-Path $repository "build/$scratchName"
$source = Join-Path $PSScriptRoot 'build_fixture'
$tool = Join-Path $repository 'tools/shared/Clear-Build.ps1'
$oldDirectory = $env:CCACHE_DIR
$oldTemp = $env:CCACHE_TEMPDIR
$oldSize = $env:CCACHE_MAXSIZE
$oldStatsLog = $env:CCACHE_STATSLOG
New-Item -ItemType Directory -Path $scratch | Out-Null

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
        $commands = Run-Cmake @('--build', $tree, '--', '-t', 'commands')
        $cacheRoot = "$repository/build/.ccache".Replace('\', '/')
        $cacheEnvironment = @("CCACHE_DIR=$cacheRoot", "CCACHE_TEMPDIR=$cacheRoot/tmp", 'CCACHE_MAXSIZE=5G')
        foreach ($setting in $cacheEnvironment) {
            if (-not ($commands -join "`n").Replace('\', '/').Contains($setting)) {
                throw "Missing generated cache route: $setting"
            }
        }
        $env:CCACHE_STATSLOG = "$scratch/cache.stats"
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
        Run-Cmake @('--build', $tree, '--target', 'clean') | Out-Null
        Run-Cmake @('--build', $tree, '--parallel', '8') | Out-Null
        $stats = Run-Cmake (@('-E', 'env') + $cacheEnvironment +
            @('ccache', '--print-log-stats'))
        $hits = 0L
        foreach ($line in $stats) { if ($line -match '^(direct|preprocessed)_cache_hit\s+(\d+)') { $hits += [long]$Matches[2] } }
        if ($hits -lt 1 -or (Test-Path "$scratch/wrong-cache") -or
            (Test-Path "$scratch/wrong-temp")) { throw 'Build did not use the repository cache.' }
        Write-Output "Invocation-local repository cache hits: $hits"
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
    $env:CCACHE_STATSLOG = $oldStatsLog
    & $tool -Targets $scratchName -Apply -TasksExited | Out-Null
}
