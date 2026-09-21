param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$allowed = @(
    (Join-Path $root 'src/type.h').Replace('\', '/'),
    (Join-Path $root 'src/lib/base/base_interface.h').Replace('\', '/')
)
$headerPattern = '^\s*#\s*include\s*[<"](?:stdio|stdlib|stdarg|stdbool|stddef|stdatomic|string|memory|time|ctype|stdint)\.h[>"]'
$failures = @()

foreach ($directory in @('src', 'test')) {
    $path = Join-Path $root $directory
    Get-ChildItem -LiteralPath $path -Recurse -File -Include '*.c', '*.h' |
        Where-Object { $_.FullName -notmatch '[\\/]nxvm-baseline[\\/]' } |
        ForEach-Object {
            $file = $_.FullName.Replace('\', '/')
            # The canonical shared Lib/Common/x86 corpora and their neutral
            # conformance tests are independently consumable. Their C-runtime
            # choices are owned and verified with the imported source, not by
            # NXVM's product facade gate.
            if ($file -like "$($root.Replace('\', '/'))/src/lib/*" -or
                $file -like "$($root.Replace('\', '/'))/src/common/*" -or
                $file -like "$($root.Replace('\', '/'))/src/x86/*" -or
                $file -like "$($root.Replace('\', '/'))/test/lib/*" -or
                $file -like "$($root.Replace('\', '/'))/test/common/*" -or
                $file -like "$($root.Replace('\', '/'))/test/x86/*") {
                return
            }
            if ($allowed -contains $file) {
                return
            }
            $line = 0
            Get-Content -LiteralPath $_.FullName | ForEach-Object {
                $line++
                if ($_ -match $headerPattern) {
                    $failures += "$($file.Substring($root.Replace('\', '/').Length + 1)):${line}: $_"
                }
            }
        }
}

if ($failures.Count -ne 0) {
    $failures | ForEach-Object { Write-Error "Direct ISO C header include: $_" }
    exit 1
}

Write-Output 'M5:T113:C-FACADE-HEADERS:OK'
