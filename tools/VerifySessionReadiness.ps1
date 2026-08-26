param([Parameter(Mandatory = $true)][string]$RepositoryRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$inventoryPath = Join-Path $root 'tools/session-readiness-state.tsv'
$inventory = @{}

Get-Content -LiteralPath $inventoryPath | ForEach-Object {
    if ($_ -and -not $_.StartsWith('#')) {
        $fields = $_ -split "`t", 4
        if ($fields.Count -ne 4) { throw "Invalid inventory row: $_" }
        $inventory[$fields[0]] = [pscustomobject]@{
            Classification = $fields[1]; Task = $fields[2]; Owner = $fields[3]
        }
    }
}

# Every retained mutable file-static object is initialized at its declaration.
# Match that initializer, not a `static` function return type whose parameter
# list starts on a following line.
$declaration = '^\s*static\s+(?!const\b)[^;]*='
$detected = @{}
Get-ChildItem (Join-Path $root 'src') -Recurse -File -Filter '*.c' | ForEach-Object {
    $relative = $_.FullName.Substring($root.Length + 1).Replace('\', '/')
    $line = 0
    Get-Content -LiteralPath $_.FullName | ForEach-Object {
        $line++
        if ($_ -match $declaration) {
            $detected[$relative] = $true
            Write-Output "M5:SESSION-READINESS:mutable:$relative`:$line"
        }
    }
}

$failures = @()
foreach ($path in $detected.Keys) {
    if (-not $inventory.ContainsKey($path)) {
        $failures += "Unclassified mutable file-static state: $path"
    }
}
foreach ($path in $inventory.Keys) {
    if (-not $detected.ContainsKey($path)) {
        $failures += "Stale mutable-state inventory entry: $path"
    }
}
if ($failures.Count) { $failures | ForEach-Object { Write-Error $_ }; exit 1 }
Write-Output 'M5:SESSION-READINESS:OK'
