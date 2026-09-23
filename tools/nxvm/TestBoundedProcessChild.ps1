[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$MarkerPath
)

$ErrorActionPreference = 'Stop'
$child = Start-Process -FilePath powershell.exe -ArgumentList @(
    '-NoProfile', '-Command', 'Start-Sleep -Seconds 30'
) -PassThru
Set-Content -LiteralPath $MarkerPath -Value $child.Id -NoNewline
while ($true) {
    Start-Sleep -Milliseconds 100
}
