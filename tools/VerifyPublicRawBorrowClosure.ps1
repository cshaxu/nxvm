param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$sourceFiles = Get-ChildItem (Join-Path $root 'src') -Recurse -File |
    Where-Object { $_.Extension -in @('.c', '.h') }
$publicHeaders = $sourceFiles | Where-Object { $_.Name -like '*_interface.h' }
$failures = @()

foreach ($file in $sourceFiles) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = Get-Content -LiteralPath $file.FullName -Raw
    if ($text -match 'core_machine_(configuration|debug)_[A-Za-z0-9_]*borrow') {
        $failures += "$relative exports or uses a raw core borrow"
    }
    if ($text -match '\bprofile(?:_[A-Za-z0-9]+)*_(?:borrow|binding)\b') {
        $failures += "$relative exports or uses a profile raw binding"
    }
    if ($text -match '#\s*include\s*[<\"](?:\.\./)*test/support/') {
        $failures += "$relative includes test/support"
    }
}

foreach ($file in $publicHeaders) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = Get-Content -LiteralPath $file.FullName -Raw
    if ($text -match '#\s*include\s*[<"]core/machine/(?:cpu|cpu_instructions|dma|fdc|hdc|kbc|machine|memory|pic|pit|port|rtc|vadp)\.h[>"]') {
        $failures += "$relative includes a private core-machine header"
    }
    if ($text -match '\b(?:t_cpu|t_cpuins|t_ram|t_port|t_pic|t_pit|t_dma|t_vadp|core_machine_fdc|core_machine_hdc|core_machine_rtc)\b') {
        $failures += "$relative exposes a complete private core-machine layout"
    }
}

foreach ($file in $sourceFiles) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = Get-Content -LiteralPath $file.FullName -Raw
    if ($text -match 'core_token\s*=\s*\([^\r\n]*(?:type_unsigned_pointer|t_dma\s*\*)' -or
        $text -match '\(t_dma\s*\*\)\s*[^\r\n]*core_token') {
        $failures += "$relative converts a public core token to or from a DMA pointer"
    }
}

if ($failures.Count) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}
Write-Output 'M5:T299:S3:PUBLIC-RAW-BORROW-CLOSURE:OK'
Write-Output 'M5:T300:S4:PUBLIC-INTERFACE-BOUNDARY:OK'
