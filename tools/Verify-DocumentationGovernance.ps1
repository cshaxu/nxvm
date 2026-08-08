[CmdletBinding()]
param(
    [string]$RepositoryRoot,
    [switch]$SelfTest
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}

function Require([bool]$condition, [string]$message) {
    if (-not $condition) {
        throw $message
    }
}

function Test-Mojibake([string]$text) {
    return $text -match '\u00E2|\u00C3|\uFFFD'
}

function Test-MachineLocalPath([string]$text) {
    return $text -match '(?i)[a-z]:(?:\\){2}(?:users|home)(?:\\){2}'
}

if ($SelfTest) {
    Require (Test-Mojibake (([char]0x00E2).ToString() + "quoted")) `
        "Mojibake detector did not reject the controlled negative sample."
    Require (-not (Test-Mojibake "ASCII only")) `
        "Mojibake detector rejected an ASCII control sample."
    Require (Test-MachineLocalPath 'D:\\home\\example') `
        "Machine-local path detector did not reject the controlled negative sample."
    Require (-not (Test-MachineLocalPath 'C:\\NAME.EXT')) `
        "Machine-local path detector rejected a guest DOS path control sample."
    Write-Output "Documentation governance mojibake self-test passed."
    exit 0
}

$docsRoot = Join-Path $RepositoryRoot "docs"
$planningRoot = Join-Path $docsRoot "planning"
$statusPath = Join-Path $planningRoot "status.md"
$historyPath = Join-Path $docsRoot "history/m5.md"
$queuePath = Join-Path $planningRoot "m5-pcat-hardware-convergence.md"
$presetPath = Join-Path $RepositoryRoot "CMakePresets.json"
$todoPath = Join-Path $RepositoryRoot "TODO.md"

$status = Get-Content -Raw -LiteralPath $statusPath
$history = Get-Content -Raw -LiteralPath $historyPath
$queue = Get-Content -Raw -LiteralPath $queuePath
$todo = Get-Content -Raw -LiteralPath $todoPath

Require (($status | Select-String -AllMatches -Pattern '(?m)^## Current Technical Baseline$').Matches.Count -eq 1) `
    "status.md must contain exactly one Current Technical Baseline heading."

$idle = $status -match '(?m)^\*\*Idle\.'
if ($idle) {
    Require (-not ($status -match '(?m)^## (Historical )?T\d+(?: S\d+)? Packet$')) `
        "Idle status.md must not retain a task packet."
    $baselineOffset = $status.IndexOf("## Current Technical Baseline")
    Require ($baselineOffset -ge 0) "Idle status.md must contain a technical baseline."
    $idlePrefixLines = ([regex]::Split($status.Substring(0, $baselineOffset), "`r?`n")).Count
    Require ($idlePrefixLines -le 8) `
        "Idle status.md must not retain completed narrative before its technical baseline."
}

$taskRecords = @(Get-ChildItem -LiteralPath $planningRoot -File -Filter "m5-t*.md")
Require ($taskRecords.Count -eq 0) `
    "Completed M5 task records must be retired from docs/planning: $($taskRecords.Name -join ', ')"

Require (-not ($history -match '(?im)^The latest completed M5 technical baseline')) `
    "M5 history must not claim a current or latest technical baseline."
Require (-not ($history -match '(?im)^## Remaining M5 Work$')) `
    "M5 history must not carry current remaining-work state."
Require (-not ($history -match '(?i)\bcurrent (?:static gates|CTest)')) `
    "M5 history must use closure-time, not current, verification wording."
Require (-not ($queue -match '(?im)^The retained baseline is')) `
    "The M5 queue must link to status instead of copying the current baseline."
Require (-not ($queue -match '(?m)^\| T\d+ \| \*\*Completed\.')) `
    "The M5 queue must not retain completed task rows."
Require (-not ($queue -match '(?im)^The completed migration order')) `
    "The M5 queue must not retain completed migration narrative."
Require (-not ($todo -match '(?m)^[-*] \[x\]')) `
    "TODO.md must contain only open debt entries."

$closureSection = [regex]::Match($status, '(?ms)^## Recent M5 Closures\r?\n(?<body>.*?)(?=^## |\z)')
Require ($closureSection.Success) "status.md must contain a Recent M5 Closures section."
$closureCount = ([regex]::Matches($closureSection.Groups['body'].Value, '(?m)^\| T\d+ \|')).Count
Require ($closureCount -le 8) "status.md must retain at most eight recent M5 closure rows."

$governanceSection = [regex]::Match($status, '(?ms)^## Recent Governance\r?\n(?<body>.*?)(?=^## |\z)')
Require ($governanceSection.Success) "status.md must contain a Recent Governance section."
$governanceCount = ([regex]::Matches($governanceSection.Groups['body'].Value, '(?m)^- \*\*M5 Td S\d+:')).Count
Require ($governanceCount -le 8) "status.md must retain at most eight recent governance rows."

$presets = Get-Content -Raw -LiteralPath $presetPath | ConvertFrom-Json
$currentPreset = @($presets.buildPresets | Where-Object { $_.name -eq "current-gcc" })
Require ($currentPreset.Count -eq 1) "CMakePresets.json must define exactly one current-gcc preset."
$currentTarget = @($currentPreset[0].targets)[0]
Require ($currentTarget -match '^vm-0-5-\d{4}$') "current-gcc must select one vm-0-5-NNNN target."
$expectedArtifact = "nxvm_" + ($currentTarget.Substring(3) -replace '-', '_') + ".exe"

$statusTargets = @([regex]::Matches($status, '\bvm-0-5-\d{4}\b') | ForEach-Object Value | Select-Object -Unique)
$statusArtifacts = @([regex]::Matches($status, '\bnxvm_0_5_\d{4}\.exe\b') | ForEach-Object Value | Select-Object -Unique)
Require ($statusTargets.Count -eq 1 -and $statusTargets[0] -eq $currentTarget) `
    "status.md current target must match CMakePresets.json ($currentTarget)."
Require ($statusArtifacts.Count -eq 1 -and $statusArtifacts[0] -eq $expectedArtifact) `
    "status.md current artifact must match CMakePresets.json ($expectedArtifact)."

$markdownFiles = @(
    Get-ChildItem -LiteralPath $docsRoot -Recurse -File -Filter "*.md"
    Get-ChildItem -LiteralPath (Join-Path $RepositoryRoot "tools") -Recurse -File -Filter "*.md"
)
$mojibake = $markdownFiles |
    Where-Object { Test-Mojibake (Get-Content -Raw -LiteralPath $_.FullName) }
Require ($null -eq $mojibake) "Documentation contains mojibake: $($mojibake.FullName -join ', ')"
$localPaths = $markdownFiles |
    Where-Object { Test-MachineLocalPath (Get-Content -Raw -LiteralPath $_.FullName) }
Require ($null -eq $localPaths) "Documentation contains machine-local paths: $($localPaths.FullName -join ', ')"

Write-Output "Documentation governance checks passed for $currentTarget."
