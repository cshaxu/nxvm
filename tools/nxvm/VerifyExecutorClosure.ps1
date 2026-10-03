param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path

$rules = @(
    @{ Pattern = 'core_machine_cpu_execution_refresh\s*\('; Allowed = @('src/x86/core/machine.c', 'src/x86/chips/cpu/cpu_instructions.c') },
    @{ Pattern = '\bvmachine(Refresh|Reset)\s*\('; Allowed = @() },
    @{ Pattern = '\bmachine_refresh\b'; Allowed = @() }
)
$failures = @()
$renamedHeaders = @{
    'clock_interface.h' = 'clock.h'
    'device_support_interface.h' = 'device_support.h'
    'execution_provider_interface.h' = 'execution_provider.h'
}
foreach ($coreFile in Get-ChildItem (Join-Path $root 'src/x86/core') -File) {
    $oldName = $coreFile.Name
    if ($renamedHeaders.ContainsKey($oldName)) { $oldName = $renamedHeaders[$oldName] }
    $oldPath = Join-Path $root ('src/app-nxvm/devices/' + $oldName)
    if (Test-Path -LiteralPath $oldPath) {
        $failures += ('Duplicate pre-extraction Core source: ' + $oldPath)
    }
}
foreach ($rule in $rules) {
    Get-ChildItem (Join-Path $root 'src') -Recurse -File -Filter '*.c' | ForEach-Object {
        $relative = $_.FullName.Substring($root.Length + 1).Replace('\', '/')
        if ((Get-Content -LiteralPath $_.FullName -Raw) -match $rule.Pattern -and
            $rule.Allowed -notcontains $relative) {
            $failures += "$relative matches $($rule.Pattern)"
        }
    }
}
if ($failures.Count) { $failures | ForEach-Object { Write-Error $_ }; exit 1 }
Write-Output 'M5:T86:EXECUTOR-CLOSURE:OK'
