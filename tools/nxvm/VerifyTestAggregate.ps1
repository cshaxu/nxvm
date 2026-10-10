[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'
$runner = Join-Path $RepositoryRoot 'tools\nxvm\Invoke-NxvmBoundedProcess.ps1'
$childScript = Join-Path $RepositoryRoot 'tools\nxvm\TestBoundedProcessChild.ps1'
$marker = Join-Path ([System.IO.Path]::GetTempPath()) ("nxvm-unit-aggregate-child-$PID.txt")
Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue

try {
    $deadlineObserved = $false
    try {
        & $runner -FilePath 'powershell.exe' -ArgumentList @(
            '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $childScript,
            '-MarkerPath', $marker
        ) -DeadlineSeconds 5 -DiagnosticPrefix 'UNIT-AGGREGATE-SELFTEST'
    }
    catch {
        if ($_.Exception.Message -match 'UNIT-AGGREGATE-SELFTEST:DEADLINE') {
            $deadlineObserved = $true
        }
        else {
            throw
        }
    }
    if (-not $deadlineObserved) {
        throw 'Unit aggregate deadline self-test did not report its deadline.'
    }
    if (-not (Test-Path -LiteralPath $marker)) {
        throw 'Unit aggregate deadline self-test did not record its child PID.'
    }
    $childPid = [int](Get-Content -Raw -LiteralPath $marker)
    Start-Sleep -Milliseconds 250
    if (Get-Process -Id $childPid -ErrorAction SilentlyContinue) {
        throw "Unit aggregate deadline self-test left child process $childPid alive."
    }
    Write-Output 'UNIT-AGGREGATE:OK'
}
finally {
    Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue
}
