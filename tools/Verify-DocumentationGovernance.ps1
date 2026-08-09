[CmdletBinding()]
param(
    [string]$RepositoryRoot,
    [switch]$SelfTest
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}

function Require([object]$condition, [string]$message) {
    if ($condition -is [System.Array]) {
        throw "Internal governance-check error: non-scalar condition for $message"
    }
    if (-not [bool]$condition) {
        throw $message
    }
}

function Test-Mojibake([string]$text) {
    # Stable encoding-corruption recurrence: not expressible as document schema.
    return $text -match '\u00E2|\u00C3|\uFFFD'
}

function Test-MachineLocalPath([string]$text) {
    # Stable disclosure recurrence: committed local paths are never portable evidence.
    return $text -match '(?i)[a-z]:(?:\\){2}(?:users|home)(?:\\){2}'
}

function Test-ExactNameSet([string[]]$actual, [string[]]$expected) {
    return @(
        Compare-Object -ReferenceObject @($expected | Sort-Object) `
            -DifferenceObject @($actual | Sort-Object)
    ).Count -eq 0
}

function Get-MarkdownHeadings([string]$text) {
    $inFence = $false
    $lineNumber = 0
    foreach ($line in [regex]::Split($text, "`r?`n")) {
        $lineNumber++
        if ($line -match '^\s*```') {
            $inFence = -not $inFence
            continue
        }
        if ($inFence) {
            continue
        }
        $match = [regex]::Match($line, '^(?<marks>#{1,6})[ \t]+(?<text>.+?)\s*$')
        if ($match.Success) {
            [pscustomobject]@{
                Level = $match.Groups['marks'].Value.Length
                Text = $match.Groups['text'].Value
                Line = $lineNumber
            }
        }
    }
}

function Require-HeadingSchema(
    [string]$path,
    [string]$text,
    [string]$title,
    [string[]]$allowedH2Patterns
) {
    $headings = @(Get-MarkdownHeadings $text)
    Require ($headings.Count -ge 1) "$path must contain a Markdown title."
    Require ($headings[0].Level -eq 1 -and $headings[0].Text -eq $title) `
        "$path must begin with '# $title'."
    Require ((@($headings | Where-Object { $_.Level -eq 1 })).Count -eq 1) `
        "$path must contain exactly one level-one title."
    foreach ($heading in $headings) {
        Require ($heading.Level -le 2) `
            "$path must not use heading levels below its fixed document schema."
        if ($heading.Level -eq 2) {
            $isAllowed = (@($allowedH2Patterns | Where-Object {
                $heading.Text -match $_
            }).Count -gt 0)
            Require $isAllowed `
                "$path contains an unsupported level-two section: $($heading.Text)"
        }
    }
}

function Require-NoTaskIdentifier([string]$path, [string]$text) {
    Require (-not ($text -match '\bT\d+\b')) `
        "$path must not allocate or describe a numeric implementation task."
}

function Require-NoChecklist([string]$path, [string]$text) {
    Require (-not ($text -match '(?m)^\s*[-*]\s+\[[ xX]\]\s+')) `
        "$path must not contain a checklist."
}

function Set-SelfTestFile([string]$root, [string]$relativePath, [string]$content) {
    $path = Join-Path $root $relativePath
    $parent = Split-Path -Parent $path
    New-Item -ItemType Directory -Force -Path $parent | Out-Null
    [System.IO.File]::WriteAllText(
        $path,
        $content + [Environment]::NewLine,
        [System.Text.UTF8Encoding]::new($false)
    )
}

$script:governanceScriptPath = $PSCommandPath

function Invoke-SelfTestCheck([string]$repositoryRoot, [switch]$Quiet) {
    $output = @()
    $passed = $true
    try {
        $output = @(& $script:governanceScriptPath -RepositoryRoot $repositoryRoot *>&1)
    }
    catch {
        $passed = $false
        $output += $_
    }
    if (-not $Quiet -and -not $passed) {
        $output | ForEach-Object { Write-Error $_ }
    }
    return $passed
}

function New-SelfTestRepository([string]$root) {
    New-Item -ItemType Directory -Force -Path $root | Out-Null
    New-Item -ItemType Directory -Force -Path (Join-Path $root "tools") | Out-Null
    Set-SelfTestFile $root "CMakePresets.json" @'
{
  "version": 4,
  "buildPresets": [
    { "name": "current-gcc", "targets": ["vm-0-5-0300"] }
  ]
}
'@
    Set-SelfTestFile $root "README.md" "# ntvdm64`n`n## Start Here`n`n## Project Boundary"
    Set-SelfTestFile $root "AGENTS.md" "# Agent Instructions`n`n## Authority`n`n## Execution"
    Set-SelfTestFile $root "CONTRIBUTING.md" "# Contributing`n`n## Change Submission`n`n## Review Record`n`n## Commits And Tracking"
    Set-SelfTestFile $root "docs/README.md" "# Documentation Guide"
    Set-SelfTestFile $root "docs/QUEUE.md" "# Queue`n`n1. Candidate work"
    Set-SelfTestFile $root "docs/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [ ] **Fixture debt (`TODO(High)`).** Admit only with evidence."
    Set-SelfTestFile $root "docs/STATUS.md" @'
# Project Status

## Current Work

**Idle.**

## Current Technical Baseline

- `vm-0-5-0300` / `nxvm_0_5_0300.exe`

## Recent M5 Closures

| Task | Compact result |
| --- | --- |

## Recent Governance
'@
    Set-SelfTestFile $root "docs/rules/ARCHITECTURE.md" "# Architecture Rules"
    Set-SelfTestFile $root "docs/rules/CODING.md" "# Coding Standard"
    Set-SelfTestFile $root "docs/rules/DOCUMENT.md" "# Documentation Rules`n`n## Authority Boundaries"
    Set-SelfTestFile $root "docs/rules/EXECUTION.md" "# Execution Policy"
    Set-SelfTestFile $root "docs/design/GOAL.md" "# Project Goals`n`n1. Strategic outcome"
    Set-SelfTestFile $root "docs/design/ARCHITECTURE.md" "# System Architecture"
    Set-SelfTestFile $root "docs/design/CODING.md" "# Source Layout"
    Set-SelfTestFile $root "docs/design/UI.md" "# Product UX"
    Set-SelfTestFile $root "docs/design/ROADMAP.md" "# Roadmap`n`n## M0: Governance Reset"
    Set-SelfTestFile $root "docs/etc/README.md" "# Supporting Documentation Index"
    New-Item -ItemType Directory -Force -Path (Join-Path $root "docs/history") | Out-Null
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
    $fixtureRoot = Join-Path ([System.IO.Path]::GetTempPath()) `
        ("ntvdm64-doc-governance-" + [guid]::NewGuid().ToString("N"))
    try {
        New-SelfTestRepository $fixtureRoot
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema rejected the controlled passing fixture."
        Set-SelfTestFile $fixtureRoot "docs/QUEUE.md" "# Queue`n`n1. T301 is not allowed here"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a Queue task identifier."
        Set-SelfTestFile $fixtureRoot "docs/QUEUE.md" "# Queue`n`n1. Candidate work"
        Set-SelfTestFile $fixtureRoot "docs/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [x] **Closed (`TODO(High)`).**"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a completed debt entry."
        Set-SelfTestFile $fixtureRoot "docs/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [ ] **Fixture debt (`TODO(High)`).** Admit only with evidence."
        Set-SelfTestFile $fixtureRoot "docs/STATUS.md" @'
# Project Status

## Current Work

**Idle.**

## Milestone State

## Current Technical Baseline

- `vm-0-5-0300` / `nxvm_0_5_0300.exe`

## Recent M5 Closures

| Task | Compact result |
| --- | --- |

## Recent Governance
'@
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted an unsupported STATUS section."
    }
    finally {
        if (Test-Path -LiteralPath $fixtureRoot) {
            Remove-Item -LiteralPath $fixtureRoot -Recurse -Force
        }
    }
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
$goalDesignPath = Join-Path $designPath "GOAL.md"
$uiDesignPath = Join-Path $designPath "UI.md"
$roadmapDesignPath = Join-Path $designPath "ROADMAP.md"
$rootReadmePath = Join-Path $RepositoryRoot "README.md"
$agentsPath = Join-Path $RepositoryRoot "AGENTS.md"
$contributingPath = Join-Path $RepositoryRoot "CONTRIBUTING.md"
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
$goalDesign = Get-Content -Raw -LiteralPath $goalDesignPath
$uiDesign = Get-Content -Raw -LiteralPath $uiDesignPath
$roadmapDesign = Get-Content -Raw -LiteralPath $roadmapDesignPath
$rootReadme = Get-Content -Raw -LiteralPath $rootReadmePath
$agents = Get-Content -Raw -LiteralPath $agentsPath
$contributing = Get-Content -Raw -LiteralPath $contributingPath

Require-HeadingSchema "README.md" $rootReadme "ntvdm64" @(
    '^Start Here$',
    '^Project Boundary$'
)
Require-HeadingSchema "AGENTS.md" $agents "Agent Instructions" @(
    '^Authority$',
    '^Execution$'
)
Require-HeadingSchema "CONTRIBUTING.md" $contributing "Contributing" @(
    '^Change Submission$',
    '^Review Record$',
    '^Commits And Tracking$'
)
Require-HeadingSchema "docs/README.md" (Get-Content -Raw -LiteralPath (Join-Path $docsRoot "README.md")) `
    "Documentation Guide" @()
Require-HeadingSchema "docs/rules/ARCHITECTURE.md" $architectureRules "Architecture Rules" @()
Require-HeadingSchema "docs/rules/CODING.md" $codingRules "Coding Standard" @()
Require-HeadingSchema "docs/rules/DOCUMENT.md" (Get-Content -Raw -LiteralPath (Join-Path $rulesPath "DOCUMENT.md")) `
    "Documentation Rules" @('^Authority Boundaries$')
Require-HeadingSchema "docs/rules/EXECUTION.md" (Get-Content -Raw -LiteralPath (Join-Path $rulesPath "EXECUTION.md")) `
    "Execution Policy" @(
        '^Request Lifecycle$',
        '^Change Discipline$',
        '^Similar-Issue Sweep$',
        '^Work Identifiers$',
        '^Linear Identifier Allocation$',
        '^Documentation Governance Gate$',
        '^Milestone Closure Evidence$',
        '^Build Tree Hygiene$',
        '^Recorder Trace Containment$'
    )
Require-HeadingSchema "docs/design/GOAL.md" $goalDesign "Project Goals" @()
Require-HeadingSchema "docs/design/ARCHITECTURE.md" $architectureDesign "System Architecture" @(
    '^Product Shape$',
    '^Modules, Ownership, And Assembly$',
    '^Product And Host Boundary$',
    '^Runtime Admission Boundary$'
)
Require-HeadingSchema "docs/design/CODING.md" $codingDesign "Source Layout" @(
    '^Current And Target Trees$',
    '^Files And Names$',
    '^Source Organization$'
)
Require-HeadingSchema "docs/design/UI.md" $uiDesign "Product UX" @(
    '^NXVM$',
    '^NXVDM$',
    '^Presentation And Debugging$',
    '^Host Resources$'
)
Require-HeadingSchema "docs/design/ROADMAP.md" $roadmapDesign "Roadmap" @(
    '^M\d+(?: And Later)?: .+$'
)

Require-NoTaskIdentifier "docs/QUEUE.md" (Get-Content -Raw -LiteralPath $queuePath)
Require-NoTaskIdentifier "docs/design/GOAL.md" $goalDesign
Require-NoTaskIdentifier "docs/design/UI.md" $uiDesign
Require-NoTaskIdentifier "docs/design/ROADMAP.md" $roadmapDesign
Require-NoChecklist "docs/QUEUE.md" (Get-Content -Raw -LiteralPath $queuePath)
Require-NoChecklist "docs/design/GOAL.md" $goalDesign
Require-NoChecklist "docs/design/ARCHITECTURE.md" $architectureDesign
Require-NoChecklist "docs/design/CODING.md" $codingDesign
Require-NoChecklist "docs/design/UI.md" $uiDesign
Require-NoChecklist "docs/design/ROADMAP.md" $roadmapDesign

$status = Get-Content -Raw -LiteralPath $statusPath
$queue = Get-Content -Raw -LiteralPath $queuePath
$todo = Get-Content -Raw -LiteralPath $todoPath

Require-HeadingSchema "docs/STATUS.md" $status "Project Status" @(
    '^Current Work$',
    '^Current Technical Baseline$',
    '^Recent M\d+ Closures$',
    '^Recent Governance$',
    '^M\d+ (?:T\d+|Td) S\d+ Packet$'
)
Require-HeadingSchema "docs/QUEUE.md" $queue "Queue" @()
Require-HeadingSchema "docs/TODO.md" $todo "Long-Term Review Ledger" @('^.+ Debt$')

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
else {
    Require (($status | Select-String -AllMatches -Pattern '(?m)^## M\d+ (?:T\d+|Td) S\d+ Packet$').Matches.Count -eq 1) `
        "Active STATUS.md must contain exactly one task packet."
}

Require (-not ($todo -match '(?m)^[-*] \[x\]')) `
    "TODO.md must contain only open debt entries."
$todoEntries = @([regex]::Matches($todo, '(?m)^- \[ \] .+$') | ForEach-Object { $_.Value })
foreach ($entry in $todoEntries) {
    Require ($entry -match 'TODO\((?:High|Medium|Low)\)') `
        "TODO.md debt entries must declare TODO(High), TODO(Medium), or TODO(Low)."
}
Require (-not ($queue -match '(?m)^\s*[-*]\s+')) `
    "QUEUE.md must use ordered candidates rather than an unordered list."
Require (-not ($queue -match '(?m)^\s*\d+\)\s+')) `
    "QUEUE.md must use Markdown ordered-list syntax for candidates."
Require (([regex]::Matches($goalDesign, '(?m)^\d+\.\s+')).Count -gt 0) `
    "Project Goals must contain an ordered strategic-outcome list."
Require ((@((Get-MarkdownHeadings $roadmapDesign) | Where-Object { $_.Level -eq 2 })).Count -gt 0) `
    "Roadmap must contain at least one milestone section."

$closureSection = [regex]::Match($status, '(?ms)^## Recent M\d+ Closures\r?\n(?<body>.*?)(?=^## |\z)')
Require ($closureSection.Success) "STATUS.md must contain a recent milestone-closure section."
$closureCount = ([regex]::Matches($closureSection.Groups['body'].Value, '(?m)^\| T\d+ \|')).Count
Require ($closureCount -le 8) "STATUS.md must retain at most eight recent milestone-closure rows."

$governanceSection = [regex]::Match($status, '(?ms)^## Recent Governance\r?\n(?<body>.*?)(?=^## |\z)')
Require ($governanceSection.Success) "STATUS.md must contain a Recent Governance section."
$governanceCount = ([regex]::Matches($governanceSection.Groups['body'].Value, '(?m)^- \*\*M\d+ Td S\d+:')).Count
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
