[CmdletBinding()]
param([string]$RepositoryRoot)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}

$legacyNames = @(
    "vcpu", "vcpuins", "vram", "vport", "vpic1", "vpic2", "vpit",
    "vlatch", "vdma1", "vdma2", "vkbc", "vvadp", "vcmos", "vfdd",
    "vfdc", "vhdd", "vdebug", "vbios", "qdxTable"
)
$definition = "^\s*(?:extern\s+)?t_[A-Za-z0-9_\s\*]+\s+({0})\s*(?:\[.*\])?\s*;\s*$" -f
    (($legacyNames | ForEach-Object { [regex]::Escape($_) }) -join "|")
$violations = @()

Get-ChildItem (Join-Path $RepositoryRoot "src") -Recurse -Filter *.c | ForEach-Object {
    $lineNumber = 0
    Get-Content $_.FullName | ForEach-Object {
        $lineNumber++
        if ($_ -match $definition) {
            $violations += "$($_.FullName):$lineNumber $_"
        }
    }
}

if ($violations.Count -ne 0) {
    Write-Error ("Legacy full-PC storage definition(s) remain:`n" +
        ($violations -join "`n"))
    exit 1
}

Write-Output "M5 live-machine authority scan found no legacy full-PC storage definitions."
