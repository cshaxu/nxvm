[CmdletBinding()]
param(
    [string]$RepositoryRoot
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}
$RepositoryRoot = (Resolve-Path $RepositoryRoot).Path
$sourceRoot = Join-Path $RepositoryRoot "src"
if (-not (Test-Path $sourceRoot -PathType Container)) {
    throw "Source root is missing: $sourceRoot"
}

function Get-Owner([string]$Path) {
    if ($Path -match '^lib/') { return "lib" }
    if ($Path -match '^emulator/') { return "emulator" }
    if ($Path -match '^(x86-debug|x86-ibmpc)/') { return $Matches[1] }
    if ($Path -match '^x86-core/(cpu|fpu|machine)/') { return "x86-core/$($Matches[1])" }
    if ($Path -match '^x86-nxvm/chips/[^/]+/') { return "x86-nxvm/chips" }
    if ($Path -match '^x86-nxvm/(board-base|board-xt|board-at|machine|product)/') {
        return "x86-nxvm/$($Matches[1])"
    }
    if ($Path -match '^app-([^/]+)/') { return "app/$($Matches[1])" }
    return $null
}

# This is the current source-level component DAG. It follows the public
# component layout rather than preserving the retired Core/VM/VDM migration
# graph. Same-owner includes are local implementation details.
$allowedTargets = @{
    "lib" = @()
    "emulator" = @("lib")
    "x86-debug" = @("lib", "emulator")
    "x86-ibmpc" = @("lib", "emulator", "x86-debug")
    "x86-core/fpu" = @("lib")
    "x86-core/cpu" = @("lib", "x86-core/fpu")
    "x86-core/machine" = @("lib", "x86-core/cpu", "x86-core/fpu")
    "x86-nxvm/chips" = @("lib")
    "x86-nxvm/board-at" = @("lib", "x86-nxvm/chips", "x86-core/machine")
    "x86-nxvm/board-xt" = @("lib", "x86-nxvm/chips", "x86-core/machine")
    "x86-nxvm/board-base" = @("lib", "x86-nxvm/chips", "x86-core/machine", "x86-nxvm/board-at", "x86-nxvm/board-xt")
    "x86-nxvm/machine" = @("lib", "emulator", "x86-debug", "x86-nxvm/chips", "x86-core/machine", "x86-nxvm/board-base")
    "x86-nxvm/product" = @("lib", "emulator", "x86-ibmpc", "x86-nxvm/machine")
}

$observed = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
Get-ChildItem $sourceRoot -Recurse -File | Where-Object {
    $_.Extension -in ".c", ".h"
} | ForEach-Object {
    $source = $_.FullName.Substring($sourceRoot.Length + 1).Replace('\', '/')
    $sourceOwner = Get-Owner $source
    if ($null -eq $sourceOwner) { return }

    Get-Content $_.FullName | ForEach-Object {
        if ($_ -notmatch '^\s*#include\s+"([^"]+)"') { return }
        $targetOwner = Get-Owner $Matches[1]
        if ($null -eq $targetOwner -or $targetOwner -eq $sourceOwner) { return }

        $allowed = $false
        if ($sourceOwner.StartsWith("app/")) {
            $allowed = -not $targetOwner.StartsWith("app/")
        } elseif ($allowedTargets.ContainsKey($sourceOwner)) {
            $allowed = $allowedTargets[$sourceOwner] -contains $targetOwner
        }
        if (-not $allowed) {
            [void]$observed.Add("src/$source|$targetOwner")
        }
    }
}

$unexpected = $observed | Sort-Object
if ($unexpected.Count -gt 0) {
    Write-Error ("Forbidden source dependency edge(s):`n" + ($unexpected -join "`n"))
    exit 1
}

Write-Output "Dependency DAG source graph verified against current component owners."
