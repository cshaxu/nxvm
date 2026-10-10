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
    if ($text -match '\bprofile(?:_[A-Za-z0-9]+)*_borrow\b') {
        $failures += "$relative exports or uses a profile raw binding"
    }
    if ($text -match '#\s*include\s*[<\"](?:\.\./)*test/support/') {
        $failures += "$relative includes test/support"
    }
}

foreach ($file in $publicHeaders) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = Get-Content -LiteralPath $file.FullName -Raw
    if ($text -match '#\s*include\s*[<"](?:app-nxvm/devices|core/x86|core/(?:board-base|board-at|board-xt))/(?:cpu|cpu_instructions|dma|fdc|hdc|kbc|machine|machine_board_state|memory|pic|pit|port|rtc|vadp)\.h[>"]') {
        $failures += "$relative includes a private core-machine header"
    }
    # Opaque declarations and pointer parameters do not publish the layout.
    # Reject definitions and by-value use after removing only those forms.
    $opaqueTypes = '(?:t_vadp|core_machine_fdc|core_machine_hdc|core_machine_rtc)'
    $layoutText = $text -replace "\btypedef\s+struct\s+($opaqueTypes)\s+\1\s*;", ''
    $layoutText = $layoutText -replace "\b$opaqueTypes\s*\*", ''
    if ($layoutText -match '\b(?:t_cpu|t_cpuins|t_ram|t_port|t_pic|t_pit|t_dma|t_vadp|core_machine_fdc|core_machine_hdc|core_machine_rtc)\b') {
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
Write-Output 'PUBLIC-RAW-BORROW-CLOSURE:OK'
Write-Output 'PUBLIC-INTERFACE-BOUNDARY:OK'
