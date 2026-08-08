param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$sourceFiles = Get-ChildItem (Join-Path $root 'src') -Recurse -File |
    Where-Object { $_.Extension -in @('.c', '.h') }
$failures = @()

foreach ($file in $sourceFiles) {
    $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = Get-Content -LiteralPath $file.FullName -Raw
    if ($text -match 'core_machine_(configuration|debug)_[A-Za-z0-9_]*borrow') {
        $failures += "$relative exports or uses a raw core borrow"
    }
    if ($text -match 'profile[^\r\n]*(borrow|binding)') {
        $failures += "$relative exports or uses a profile raw binding"
    }
    if ($text -match '#\s*include\s*[<\"](?:\.\./)*tests/support/') {
        $failures += "$relative includes tests/support"
    }
}

if ($failures.Count) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}
Write-Output 'M5:T299:S3:PUBLIC-RAW-BORROW-CLOSURE:OK'
