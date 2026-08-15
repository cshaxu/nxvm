param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$manager = Get-Content -LiteralPath (Join-Path $root 'src/core/product/session/manager.c') -Raw
$command = Get-Content -LiteralPath (Join-Path $root 'src/core/product/session/command.c') -Raw
$coreSession = Get-ChildItem (Join-Path $root 'src/core/product/session') -File -Filter '*.c'
$source = Get-ChildItem (Join-Path $root 'src') -Recurse -File -Include '*.c','*.h'
$failures = @()

if ($manager -match 'core_product_session_manager_create[\s\S]*core_product_session_manager_open\(manager, STD_NULL\)') {
    $failures += 'manager create still establishes an implicit session'
}
if ($manager -notmatch 'core_product_session_id next_id') {
    $failures += 'manager does not retain a monotonic session identifier source'
}
if ($manager -match 'manager->count <= 1u\) return TYPE_STATUS_INVALID_STATE') {
    $failures += 'manager still rejects closing the final session'
}
if ($manager -match '(?m)^\s*static\s+(?!C_INT\s+core_product_session_manager_find)') {
    $failures += 'manager declares mutable static state'
}
if ($command -notmatch 'SESSION LIST \| OPEN \| SELECT <id> \| CLOSE \[id\]') {
    $failures += 'shared command grammar is incomplete'
}
foreach ($file in $coreSession) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    if ((Get-Content -LiteralPath $file.FullName -Raw) -match '#include\s+"(vm|vdm)/') {
        $failures += "$relative includes product implementation"
    }
}
foreach ($file in $source) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    if ((Get-Content -LiteralPath $file.FullName -Raw) -match 'console_target|nxvm_product_console_target|vm_session_console_target') {
        $failures += "$relative retains old console target vocabulary"
    }
}
if ($failures.Count) { $failures | ForEach-Object { Write-Error $_ }; exit 1 }
Write-Output 'M5:T150:PRODUCT-SESSION-MANAGER:OK'
