[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$CTestPath,
    [Parameter(Mandatory = $true)]
    [string]$TestDirectory,
    [Parameter(Mandatory = $true)]
    [int]$ParallelJobs,
    [int]$DeadlineSeconds = 300,
    [ValidateSet('unit', 'integration')]
    [string]$Route = 'unit'
)

$ErrorActionPreference = 'Stop'
$runner = Join-Path $PSScriptRoot 'Invoke-NxvmBoundedProcess.ps1'
$arguments = @(
    '--test-dir', $TestDirectory,
    '--output-on-failure', '--no-tests=error', '--label-regex', "^$Route$"
)
$arguments += @('--parallel', $ParallelJobs)

& $runner -FilePath $CTestPath -ArgumentList $arguments `
    -DeadlineSeconds $DeadlineSeconds -WorkingDirectory $TestDirectory `
    -DiagnosticPrefix 'UNIT-AGGREGATE'
