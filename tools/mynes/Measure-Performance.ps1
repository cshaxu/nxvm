[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$BuildDirectory,
    [Parameter(Mandatory = $true)][string]$AssetDirectory,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [string]$ProbeExecutable = '',
    [ValidateRange(0, 62)][int]$ProcessorIndex = 0,
    [int]$DeadlineSeconds = 60
)

$ErrorActionPreference = 'Stop'
$repository = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$build = (Resolve-Path -LiteralPath $BuildDirectory).Path
$assets = (Resolve-Path -LiteralPath $AssetDirectory).Path
$output = [IO.Path]::GetFullPath($OutputDirectory)
$buildRoot = Join-Path $repository 'build'
if (-not $output.StartsWith($buildRoot + '\') -or $DeadlineSeconds -le 0) {
    throw 'Performance results require an owned build directory and positive deadline.'
}
$cache = Get-Content -Raw -LiteralPath (Join-Path $build 'CMakeCache.txt')
if ($cache -notmatch '(?m)^CMAKE_BUILD_TYPE:[^=]+=Release\r?$' -or
    $cache -notmatch '(?m)^REPOSITORY_BUILD_NXVM:[^=]+=OFF\r?$') {
    throw 'Use an optimized MyNES-only cache, not a mixed or Debug baseline.'
}
$executable = Join-Path $build 'test-app-mynes-integration/mynes-performance-probe.exe'
if ($ProbeExecutable) {
    $executable = (Resolve-Path -LiteralPath $ProbeExecutable).Path
    if (-not $executable.StartsWith($buildRoot + '\')) {
        throw 'A reference probe must be retained under the owned build directory.'
    }
}
if (-not (Test-Path -LiteralPath $executable)) { throw 'Build mynes-performance-probe first.' }
New-Item -ItemType Directory -Force -Path $output | Out-Null
$records = @()
foreach ($name in @('smb1','drmario','jackal','smb2','smb3','tmnt3')) {
    $rom = Join-Path $assets ($name + '.nes')
    $identity = (Get-FileHash -LiteralPath $rom -Algorithm SHA256).Hash
    foreach ($mode in @('graphics','text')) {
        $stdout = Join-Path $output "$name-$mode.csv"
        $stderr = Join-Path $output "$name-$mode.error.txt"
        # A hybrid host can otherwise schedule the two variants on different
        # core classes. The child inherits affinity before any timed work.
        $parentProcess = [Diagnostics.Process]::GetCurrentProcess()
        $parentAffinity = $parentProcess.ProcessorAffinity
        $affinity = [long]1 -shl $ProcessorIndex
        try {
            if (($parentAffinity.ToInt64() -band $affinity) -eq 0) {
                throw 'Selected processor is not available to this process.'
            }
            $parentProcess.ProcessorAffinity = [IntPtr]$affinity
            $process = Start-Process -FilePath $executable -ArgumentList @(
                ('"' + $rom + '"'), $mode) -PassThru -WindowStyle Hidden `
                -RedirectStandardOutput $stdout -RedirectStandardError $stderr
        }
        finally {
            $parentProcess.ProcessorAffinity = $parentAffinity
            $parentProcess.Dispose()
        }
        try {
            $null = $process.Handle
            # This inspected leaf invokes no worker, presenter or child process.
            if (-not $process.WaitForExit($DeadlineSeconds * 1000)) {
                $process.Kill()
                $process.WaitForExit()
                throw "$name/$mode benchmark exceeded its deadline."
            }
            $process.Refresh()
            if ($process.ExitCode -ne 0) {
                throw "$name/$mode benchmark failed: $(Get-Content -Raw $stderr)"
            }
            $rows = @(Import-Csv -LiteralPath $stdout)
            if ($rows.Count -ne 5) { throw "$name/$mode did not complete its five fixed rounds." }
            foreach ($row in $rows) {
                if ($row.scope -ne 'core+conversion' -or $row.frames -ne '120') {
                    throw 'Unexpected measurement scope or guest frame count.'
                }
                $row | Add-Member -NotePropertyName workload -NotePropertyValue $name
                $row | Add-Member -NotePropertyName rom_sha256 -NotePropertyValue $identity
                $row | Add-Member -NotePropertyName processor_index -NotePropertyValue $ProcessorIndex
                if ($mode -eq 'text') {
                    $paired = $records | Where-Object {
                        $_.workload -eq $name -and $_.mode -eq 'graphics' -and
                        $_.round -eq $row.round
                    } | Select-Object -First 1
                    foreach ($field in @('cycles','instructions','pcm_hash','state_hash')) {
                        if ($null -eq $paired -or $row.$field -ne $paired.$field) {
                            throw "$name guest state changed between output representations."
                        }
                    }
                }
                $records += $row
            }
            if ((Get-FileHash -LiteralPath $rom -Algorithm SHA256).Hash -ne $identity) {
                throw "$name input changed during measurement."
            }
            Write-Output "$name/${mode}: five deterministic rounds complete"
        }
        finally { $process.Dispose() }
    }
}
$records | Export-Csv -NoTypeInformation -LiteralPath (Join-Path $output 'baseline.csv')
