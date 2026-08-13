[CmdletBinding()]
param(
    [string]$RepositoryRoot,
    [string]$AllowlistPath
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}
if ([string]::IsNullOrWhiteSpace($AllowlistPath)) {
    $AllowlistPath = Join-Path $PSScriptRoot "dependency-dag-allowlist.txt"
}
$sourceRoot = Join-Path $RepositoryRoot "src"
$allowlist = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)

Get-Content $AllowlistPath | ForEach-Object {
    $entry = $_.Trim()
    if ($entry.Length -gt 0 -and -not $entry.StartsWith("#")) {
        [void]$allowlist.Add($entry)
    }
}

function Get-Owner([string]$path) {
    if ($path -match '^(core|vm|vdm)/([^/]+)/') {
        return "$($Matches[1])/$($Matches[2])"
    }
    return $null
}

$observed = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
Get-ChildItem $sourceRoot -Recurse -File | Where-Object {
    $_.Extension -in ".c", ".h"
} | ForEach-Object {
    $source = $_.FullName.Substring($sourceRoot.Length + 1).Replace('\', '/')
    $sourceOwner = Get-Owner $source
    if ($null -eq $sourceOwner) { return }

    Get-Content $_.FullName | ForEach-Object {
        if ($_ -match '^\s*#include\s+"([^"]+)"') {
            $targetOwner = Get-Owner $Matches[1]
            if ($null -eq $targetOwner) { return }

            $forbidden =
                (($sourceOwner -like "core/*") -and ($targetOwner -ne $sourceOwner) -and
                    ($targetOwner -ne "core/utils")) -or
                (($sourceOwner -match '^(vm|vdm)/') -and
                    ($sourceOwner -notmatch '^(vm|vdm)/composition$') -and
                    ($targetOwner -match '^(vm|vdm)/') -and
                    ($targetOwner -ne $sourceOwner))
            if ($forbidden) {
                [void]$observed.Add("src/$source|$targetOwner")
            }
        }
    }
}

$unexpected = $observed.Where({ -not $allowlist.Contains($_) }) | Sort-Object
$stale = $allowlist.Where({ -not $observed.Contains($_) }) | Sort-Object
if ($unexpected.Count -gt 0 -or $stale.Count -gt 0) {
    if ($unexpected.Count -gt 0) {
        Write-Error ("New forbidden dependency edge(s):`n" + ($unexpected -join "`n"))
    }
    if ($stale.Count -gt 0) {
        Write-Error ("Stale dependency allowlist edge(s):`n" + ($stale -join "`n"))
    }
    exit 1
}

Write-Output "Dependency DAG source allowlist matches $($observed.Count) known migration edges."
