[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'
$runner = Join-Path $RepositoryRoot 'tools\Invoke-NxvmBoundedProcess.ps1'
$childScript = Join-Path $RepositoryRoot 'tools\TestBoundedProcessChild.ps1'
$marker = Join-Path ([System.IO.Path]::GetTempPath()) ("nxvm-t382-child-$PID.txt")
Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue

try {
    $deadlineObserved = $false
    try {
        & $runner -FilePath 'powershell.exe' -ArgumentList @(
            '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $childScript,
            '-MarkerPath', $marker
        ) -DeadlineSeconds 5 -DiagnosticPrefix 'T382:CURRENT-GATE-SELFTEST'
    }
    catch {
        if ($_.Exception.Message -match 'T382:CURRENT-GATE-SELFTEST:DEADLINE') {
            $deadlineObserved = $true
        }
        else {
            throw
        }
    }
    if (-not $deadlineObserved) {
        throw 'T382 aggregate deadline self-test did not report its deadline.'
    }
    if (-not (Test-Path -LiteralPath $marker)) {
        throw 'T382 aggregate deadline self-test did not record its child PID.'
    }
    $childPid = [int](Get-Content -Raw -LiteralPath $marker)
    Start-Sleep -Milliseconds 250
    if (Get-Process -Id $childPid -ErrorAction SilentlyContinue) {
        throw "T382 aggregate deadline self-test left child process $childPid alive."
    }
    Write-Output 'M5:T382:S1:CURRENT-GATE-AGGREGATE:OK'
}
finally {
    Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue
}
