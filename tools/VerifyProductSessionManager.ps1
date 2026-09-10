param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$source = Get-ChildItem (Join-Path $root 'src') -Recurse -File -Include '*.c','*.h'
$failures = @()

if (@(Get-ChildItem -LiteralPath (Join-Path $root 'src/core/product/session') -File -ErrorAction SilentlyContinue).Count -ne 0) {
    $failures += 'obsolete core product-session manager files remain'
}
foreach ($file in $source) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $contents = Get-Content -LiteralPath $file.FullName -Raw
    if ($contents -match 'core_product_session|product_session_id') {
        $failures += "$relative retains product-session manager vocabulary"
    }
    if ($contents -cmatch 'SESSION LIST|SESSION OPEN|SESSION SELECT|SESSION CLOSE') {
        $failures += "$relative retains multi-session Console grammar"
    }
    if ($contents -match 'console_target|nxvm_product_console_target|vm_session_console_target') {
        $failures += "$relative retains old console target vocabulary"
    }
}
if ($failures.Count) { $failures | ForEach-Object { Write-Error $_ }; exit 1 }
Write-Output 'M5:T526:S4:PRODUCT-SINGLE-SESSION:OK'
