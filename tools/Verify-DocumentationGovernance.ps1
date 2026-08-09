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

function Test-ExactNameSet([string[]]$actual, [string[]]$expected) {
    return @(
        Compare-Object -ReferenceObject @($expected | Sort-Object) `
            -DifferenceObject @($actual | Sort-Object)
    ).Count -eq 0
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
    Require (Test-ExactNameSet @("A.md", "B.md") @("B.md", "A.md")) `
        "Fixed-file checker rejected an identical control set."
    Require (-not (Test-ExactNameSet @("A.md") @("B.md"))) `
        "Fixed-file checker accepted a mismatched control set."
    Write-Output "Documentation governance self-tests passed."
    exit 0
}

$docsRoot = Join-Path $RepositoryRoot "docs"
$statusPath = Join-Path $docsRoot "STATUS.md"
$queuePath = Join-Path $docsRoot "QUEUE.md"
$presetPath = Join-Path $RepositoryRoot "CMakePresets.json"
$todoPath = Join-Path $docsRoot "TODO.md"

$expectedRootFiles = @("QUEUE.md", "README.md", "STATUS.md", "TODO.md")
$expectedRootDirectories = @("design", "etc", "history", "rules")
$expectedRuleFiles = @("ARCHITECTURE.md", "CODING.md", "DOCUMENT.md", "EXECUTION.md")
$expectedDesignFiles = @("ARCHITECTURE.md", "CODING.md", "GOAL.md", "ROADMAP.md", "UI.md")

$rootFiles = @(Get-ChildItem -LiteralPath $docsRoot -File | ForEach-Object Name)
$rootDirectories = @(Get-ChildItem -LiteralPath $docsRoot -Directory | ForEach-Object Name)
Require (Test-ExactNameSet $rootFiles $expectedRootFiles) `
    "docs/ must contain only README.md, STATUS.md, QUEUE.md, and TODO.md."
Require (Test-ExactNameSet $rootDirectories $expectedRootDirectories) `
    "docs/ must contain only rules/, design/, history/, and etc/."

$rulesPath = Join-Path $docsRoot "rules"
$designPath = Join-Path $docsRoot "design"
$historyPath = Join-Path $docsRoot "history"
$etcIndexPath = Join-Path $docsRoot "etc\\README.md"
$architectureRulesPath = Join-Path $rulesPath "ARCHITECTURE.md"
$codingRulesPath = Join-Path $rulesPath "CODING.md"
$architectureDesignPath = Join-Path $designPath "ARCHITECTURE.md"
$codingDesignPath = Join-Path $designPath "CODING.md"
$ruleFiles = @(Get-ChildItem -LiteralPath $rulesPath -File | ForEach-Object Name)
$designFiles = @(Get-ChildItem -LiteralPath $designPath -File | ForEach-Object Name)
Require (Test-ExactNameSet $ruleFiles $expectedRuleFiles) `
    "docs/rules/ must contain only its four fixed rule files."
Require (Test-ExactNameSet $designFiles $expectedDesignFiles) `
    "docs/design/ must contain only its five fixed design files."
Require (@(Get-ChildItem -LiteralPath $rulesPath -Directory).Count -eq 0) `
    "docs/rules/ must not contain subdirectories."
Require (@(Get-ChildItem -LiteralPath $designPath -Directory).Count -eq 0) `
    "docs/design/ must not contain subdirectories."
Require (@(Get-ChildItem -LiteralPath $historyPath -Directory).Count -eq 0) `
    "docs/history/ must not contain subdirectories."
Require (Test-Path -LiteralPath $etcIndexPath) `
    "docs/etc/ must contain its supporting-documentation index."
foreach ($historyFile in @(Get-ChildItem -LiteralPath $historyPath -File)) {
    Require ($historyFile.Name -match '^M\d+-T\d+-.+\.md$') `
        "docs/history/ may contain only numbered implementation-task records."
}

$architectureRules = Get-Content -Raw -LiteralPath $architectureRulesPath
$codingRules = Get-Content -Raw -LiteralPath $codingRulesPath
$architectureDesign = Get-Content -Raw -LiteralPath $architectureDesignPath
$codingDesign = Get-Content -Raw -LiteralPath $codingDesignPath
Require ($architectureRules -match 'one explicit owner and one production path') `
    "Architecture Rules must retain the one-owner/one-production-path invariant."
Require ($architectureRules -match 'raw CPU, RAM, port, device, executor, or session pointer') `
    "Architecture Rules must forbid raw mutable-internal pointers at public boundaries."
Require (-not ($architectureRules -match '(?i)\b(core|vm|mantle|dos|vdm)\b')) `
    "Architecture Rules must not duplicate the concrete ntvdm64 component map."
Require (-not ($codingRules -match '(?i)git mv|TODO\(|similar-issue')) `
    "Coding Rules must not contain task-execution discipline."
Require (-not ($architectureDesign -match 'one explicit owner and one production path|raw CPU, RAM, port, device, executor, or session pointer')) `
    "System Architecture must not duplicate architecture-rule invariants."
Require (-not ($codingDesign -match '(?m)^## Dependency Direction$')) `
    "Source Layout must not duplicate the architecture dependency map."

$status = Get-Content -Raw -LiteralPath $statusPath
$queue = Get-Content -Raw -LiteralPath $queuePath
$todo = Get-Content -Raw -LiteralPath $todoPath

Require (($status | Select-String -AllMatches -Pattern '(?m)^## Current Technical Baseline$').Matches.Count -eq 1) `
    "STATUS.md must contain exactly one Current Technical Baseline heading."

$idle = $status -match '(?m)^\*\*Idle\.'
if ($idle) {
    Require (-not ($status -match '(?m)^## (Historical )?T\d+(?: S\d+)? Packet$')) `
        "Idle STATUS.md must not retain a task packet."
    $baselineOffset = $status.IndexOf("## Current Technical Baseline")
    Require ($baselineOffset -ge 0) "Idle STATUS.md must contain a technical baseline."
    $idlePrefixLines = ([regex]::Split($status.Substring(0, $baselineOffset), "`r?`n")).Count
    Require ($idlePrefixLines -le 8) `
        "Idle STATUS.md must not retain completed narrative before its technical baseline."
}

Require (-not ($todo -match '(?m)^[-*] \[x\]')) `
    "TODO.md must contain only open debt entries."
Require (-not ($queue -match '\bT\d+\b')) `
    "QUEUE.md must not reserve numeric implementation task identifiers."

$closureSection = [regex]::Match($status, '(?ms)^## Recent M5 Closures\r?\n(?<body>.*?)(?=^## |\z)')
Require ($closureSection.Success) "STATUS.md must contain a Recent M5 Closures section."
$closureCount = ([regex]::Matches($closureSection.Groups['body'].Value, '(?m)^\| T\d+ \|')).Count
Require ($closureCount -le 8) "STATUS.md must retain at most eight recent M5 closure rows."

$governanceSection = [regex]::Match($status, '(?ms)^## Recent Governance\r?\n(?<body>.*?)(?=^## |\z)')
Require ($governanceSection.Success) "STATUS.md must contain a Recent Governance section."
$governanceCount = ([regex]::Matches($governanceSection.Groups['body'].Value, '(?m)^- \*\*M5 Td S\d+:')).Count
Require ($governanceCount -le 8) "STATUS.md must retain at most eight recent governance rows."

$presets = Get-Content -Raw -LiteralPath $presetPath | ConvertFrom-Json
$currentPreset = @($presets.buildPresets | Where-Object { $_.name -eq "current-gcc" })
Require ($currentPreset.Count -eq 1) "CMakePresets.json must define exactly one current-gcc preset."
$currentTarget = @($currentPreset[0].targets)[0]
Require ($currentTarget -match '^vm-0-5-\d{4}$') "current-gcc must select one vm-0-5-NNNN target."
$expectedArtifact = "nxvm_" + ($currentTarget.Substring(3) -replace '-', '_') + ".exe"

$statusTargets = @([regex]::Matches($status, '\bvm-0-5-\d{4}\b') | ForEach-Object Value | Select-Object -Unique)
$statusArtifacts = @([regex]::Matches($status, '\bnxvm_0_5_\d{4}\.exe\b') | ForEach-Object Value | Select-Object -Unique)
Require ($statusTargets.Count -eq 1 -and $statusTargets[0] -eq $currentTarget) `
    "STATUS.md current target must match CMakePresets.json ($currentTarget)."
Require ($statusArtifacts.Count -eq 1 -and $statusArtifacts[0] -eq $expectedArtifact) `
    "STATUS.md current artifact must match CMakePresets.json ($expectedArtifact)."

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
