[CmdletBinding()]
param(
    [string]$RepositoryRoot,
    [switch]$SelfTest,
    [ValidateSet('All', 'Documentation', 'GovernanceState')]
    [string]$Scope = 'All'
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
    # Stable disclosure recurrence: host/workspace paths are not portable evidence.
    # Guest DOS examples such as C:\NAME.EXT remain valid documentation.
    return $text -match '(?i)(?:[a-z]:\\(?:users|home|assets|repos(?:[._-][^\\]+)?|temp|appdata|program files)\\|/(?:home|users)/|\\\\[a-z0-9][a-z0-9.-]*\\)'
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
        Require ($heading.Level -le 3) `
            "$path must not use heading levels below its document schema."
        if ($heading.Level -eq 2) {
            $isAllowed = (@($allowedH2Patterns | Where-Object {
                $heading.Text -match $_
            }).Count -gt 0)
            Require $isAllowed `
                "$path contains an unsupported level-two section: $($heading.Text)"
        }
    }
}

function Require-RequiredH2(
    [string]$path,
    [string]$text,
    [string[]]$requiredH2Patterns
) {
    $headings = @(Get-MarkdownHeadings $text | Where-Object { $_.Level -eq 2 })
    foreach ($pattern in $requiredH2Patterns) {
        Require ((@($headings | Where-Object { $_.Text -match $pattern }).Count -gt 0)) `
            "$path must contain a required level-two section matching: $pattern"
    }
}

function Get-MarkdownLinks([string]$text) {
    $inFence = $false
    foreach ($line in [regex]::Split($text, "`r?`n")) {
        if ($line -match '^\s*```') {
            $inFence = -not $inFence
            continue
        }
        if ($inFence) {
            continue
        }
        foreach ($match in [regex]::Matches(
            $line,
            '(?<prefix>!?)\[[^\]]*\]\((?<destination><[^>]+>|[^)\s]+)(?:\s+[^)]*)?\)'
        )) {
            $destination = $match.Groups['destination'].Value.Trim('<', '>')
            if (-not [string]::IsNullOrWhiteSpace($destination)) {
                [pscustomobject]@{ Destination = $destination }
            }
        }
    }
}

function Get-MarkdownAnchor([string]$heading) {
    $anchor = $heading.ToLowerInvariant()
    $anchor = [regex]::Replace($anchor, '[^a-z0-9 _-]', '')
    $anchor = [regex]::Replace($anchor, '\s+', '-')
    return [regex]::Replace($anchor, '-+', '-')
}

function Require-RelativeMarkdownLinks([string]$repositoryRoot, [string[]]$markdownPaths) {
    foreach ($path in $markdownPaths) {
        $text = Get-Content -Raw -LiteralPath $path
        foreach ($link in @(Get-MarkdownLinks $text)) {
            $destination = $link.Destination
            if ($destination -match '^[a-z][a-z0-9+.-]*:' -or $destination.StartsWith('//')) {
                continue
            }
            $parts = $destination.Split('#', 2)
            $relativePath = $parts[0]
            $fragment = if ($parts.Count -gt 1) { $parts[1] } else { '' }
            $target = if ([string]::IsNullOrWhiteSpace($relativePath)) {
                $path
            }
            else {
                [System.IO.Path]::GetFullPath((Join-Path (Split-Path -Parent $path) $relativePath))
            }
            Require (Test-Path -LiteralPath $target) `
                "Markdown link in $path targets a missing path: $destination"
            if (-not [string]::IsNullOrWhiteSpace($fragment)) {
                $targetText = Get-Content -Raw -LiteralPath $target
                $anchors = @(Get-MarkdownHeadings $targetText | ForEach-Object {
                    Get-MarkdownAnchor $_.Text
                })
                Require ($anchors -contains $fragment.ToLowerInvariant()) `
                    "Markdown link in $path targets a missing anchor: $destination"
            }
        }
    }
}

function Get-IdentifierRecordsFromSubjects([string[]]$subjects) {
    $records = @()
    foreach ($subject in $subjects) {
        $match = [regex]::Match(
            $subject,
            '^M(?<milestone>\d+)\s+(?:(?:T(?<task>\d+)\s+S(?<subtask>\d+))|(?:Td\s+S(?<docSubtask>\d+)))\s+P\d+:'
        )
        if ($match.Success) {
            $records += [pscustomobject]@{
                Milestone = [int]$match.Groups['milestone'].Value
                Task = if ($match.Groups['task'].Success) { [int]$match.Groups['task'].Value } else { $null }
                Subtask = if ($match.Groups['subtask'].Success) { [int]$match.Groups['subtask'].Value } else { [int]$match.Groups['docSubtask'].Value }
                IsDocumentation = -not $match.Groups['task'].Success
            }
        }
    }
    return @($records)
}

function Get-ClosedIdentifierRecords([string]$repositoryRoot) {
    $subjects = @(git -C $repositoryRoot log --format=%s)
    Require ($LASTEXITCODE -eq 0) "Git commit history is required to validate active identifier allocation."
    $records = @(Get-IdentifierRecordsFromSubjects $subjects)
    $historyRoot = Join-Path $repositoryRoot "docs/history"
    $historyRecords = @(
        Get-ChildItem -LiteralPath $historyRoot -File -Filter "M*-T*-*.md" |
        ForEach-Object {
            $match = [regex]::Match(
                $_.Name,
                '^M(?<milestone>\d+)-T(?<task>\d+)-'
            )
            if ($match.Success) {
                [pscustomobject]@{
                    Milestone = [int]$match.Groups['milestone'].Value
                    Task = [int]$match.Groups['task'].Value
                    Subtask = $null
                    IsDocumentation = $false
                }
            }
        }
    )
    return @($records + $historyRecords)
}

function Get-ActivePacket([string]$status) {
    $match = [regex]::Match(
        $status,
        '(?ms)^## M(?<milestone>\d+) (?:(?:T(?<task>\d+))|Td) S(?<subtask>\d+) Packet\r?\n(?<body>.*?)(?=^## |\z)'
    )
    if (-not $match.Success) {
        return $null
    }
    return [pscustomobject]@{
        Milestone = [int]$match.Groups['milestone'].Value
        Task = if ($match.Groups['task'].Success) { [int]$match.Groups['task'].Value } else { $null }
        Subtask = [int]$match.Groups['subtask'].Value
        IsDocumentation = -not $match.Groups['task'].Success
        Body = $match.Groups['body'].Value
    }
}

function Require-ActivePacketSchema([pscustomobject]$packet) {
    Require ($packet.Body -match '(?m)^\|\s*Field\s*\|\s*Required record\s*\|\s*$') `
        "Active task packet must contain the fixed Field table header."
    Require ($packet.Body -match '(?m)^\|\s*---\s*\|\s*---\s*\|\s*$') `
        "Active task packet must contain the fixed Field table separator."
    $requiredFields = @(
        'Identifier Mode', 'Admission And Approval', 'Objective', 'Non-goals',
        'Reference Baseline', 'Candidate Proposal', 'Files And ABI Surface', 'Applicable Rules',
        'Verification', 'Expected Markers', 'Asset Needs',
        'Reporting Requirements', 'Stop Conditions',
        'Exit Criteria', 'Original Owner Request', 'Similar-Issue Sweep'
    )
    foreach ($field in $requiredFields) {
        $pattern = '(?m)^\|\s*' + [regex]::Escape($field) + '\s*\|\s*\S.+?\s*\|\s*$'
        Require ($packet.Body -match $pattern) `
            "Active task packet must contain a non-empty '$field' table record."
    }
    if (-not $packet.IsDocumentation) {
        Require ($packet.Body -match '(?m)^\|\s*Candidate Proposal\s*\|[^|]*\]\(\.\./proposals/[^)]+\.md\)[^|]*\|\s*$') `
            "Numbered task packet must link its Candidate Proposal from docs/proposals/."
    }
}

function Get-StatusClosureRows([string]$status) {
    $closureSection = [regex]::Match($status, '(?ms)^## Recent M\d+ Closures\r?\n(?<body>.*?)(?=^## |\z)')
    Require ($closureSection.Success) "CURRENT.md must contain a recent milestone-closure section."
    return @([regex]::Matches(
            $closureSection.Groups['body'].Value,
            '(?m)^\| T(?<task>\d+)(?: S(?<subtask>\d+))? \|'
        ) | ForEach-Object {
            [pscustomobject]@{
                Task = [int]$_.Groups['task'].Value
                HasSubtask = $_.Groups['subtask'].Success
            }
        })
}

function Get-StatusGovernanceIdentifierRecords([string]$status) {
    $governanceSection = [regex]::Match(
        $status,
        '(?ms)^## Recent Governance\r?\n(?<body>.*?)(?=^## |\z)'
    )
    if (-not $governanceSection.Success) {
        return @()
    }
    return @([regex]::Matches(
        $governanceSection.Groups['body'].Value,
        '(?m)^- \*\*M(?<milestone>\d+) Td S(?<subtask>\d+)(?: P\d+)?:'
    ) | ForEach-Object {
        [pscustomobject]@{
            Milestone = [int]$_.Groups['milestone'].Value
            Subtask = [int]$_.Groups['subtask'].Value
        }
    })
}

function Require-ActiveIdentifier([pscustomobject]$packet, [string]$repositoryRoot, [string]$status) {
    $modeMatch = [regex]::Match($packet.Body, '(?m)^\|\s*Identifier Mode\s*\|\s*(?<mode>[^|]+?)\s*\|\s*$')
    Require $modeMatch.Success "Active task packet must declare Identifier Mode."
    $mode = $modeMatch.Groups['mode'].Value.Trim().Split(';')[0].Trim()
    $closed = @(Get-ClosedIdentifierRecords $repositoryRoot | Where-Object {
        $_.Milestone -ne $packet.Milestone -or
        $_.Task -ne $packet.Task -or
        ($null -ne $_.Subtask -and $_.Subtask -ne $packet.Subtask) -or
        $_.IsDocumentation -ne $packet.IsDocumentation
    })
    if ($packet.IsDocumentation) {
        Require ($mode -eq 'Governance') "A Td packet must use Identifier Mode Governance."
        $previous = @(
            @($closed | Where-Object {
                $_.IsDocumentation -and $_.Milestone -eq $packet.Milestone
            }) + @((Get-StatusGovernanceIdentifierRecords $status) | Where-Object {
                $_.Milestone -eq $packet.Milestone
            }) |
            Sort-Object Subtask -Descending |
            Select-Object -First 1
        )
        $expected = if ($previous.Count -eq 0) { 1 } else { $previous[0].Subtask + 1 }
        Require ($packet.Subtask -eq $expected) `
            "Active Td packet must use the next M$($packet.Milestone) Td S identifier ($expected)."
        return
    }

    $numeric = @($closed | Where-Object { -not $_.IsDocumentation })
    $latestTask = if ($numeric.Count -eq 0) { 0 } else { ($numeric | Measure-Object -Property Task -Maximum).Maximum }
    $closureRows = @(Get-StatusClosureRows $status)
    $progressRows = @($closureRows | Where-Object HasSubtask)
    $progressTasks = @($progressRows | ForEach-Object Task | Select-Object -Unique)
    Require ($progressTasks.Count -le 1) "CURRENT.md may retain subtask progress for only one open numeric task."
    $openTask = if ($progressTasks.Count -eq 1) { $progressTasks[0] } else { $null }
    if ($null -ne $openTask) {
        Require (-not ($closureRows | Where-Object { -not $_.HasSubtask -and $_.Task -eq $openTask })) `
            "CURRENT.md must replace closed-task subtask progress with one task-level summary."
        Require (($openTask -eq $latestTask) -or $mode -eq 'Owner-Reopen') `
            "CURRENT.md subtask progress must belong to the latest open numeric task."
    }
    if ($mode -eq 'New') {
        if ($latestTask -gt 0) {
            Require ($null -eq $openTask -and ($closureRows | Where-Object { -not $_.HasSubtask -and $_.Task -eq $latestTask })) `
                "A new task packet requires the latest numeric task T$latestTask to be task-level closed."
        }
        Require ($packet.Task -eq ($latestTask + 1) -and $packet.Subtask -eq 1) `
            "A new task packet must use T$($latestTask + 1) S1."
        return
    }
    if ($mode -eq 'Continuation') {
        Require ($null -ne $openTask -and $packet.Task -eq $openTask) `
            "A continuation packet may only use the latest open numeric task T$openTask."
        $taskRecords = @($numeric | Where-Object { $_.Task -eq $packet.Task })
        Require ($taskRecords.Count -gt 0) "A continuation packet requires prior committed evidence for T$($packet.Task)."
        $expectedSubtask = ($taskRecords | Measure-Object -Property Subtask -Maximum).Maximum + 1
        Require ($packet.Subtask -eq $expectedSubtask) `
            "A continuation packet must use T$($packet.Task) S$expectedSubtask."
        return
    }
    if ($mode -eq 'Corrective') {
        Require ($null -eq $openTask) "A corrective packet cannot run while numeric task T$openTask remains open."
        Require ($latestTask -gt 0 -and $packet.Task -eq $latestTask -and ($closureRows | Where-Object { -not $_.HasSubtask -and $_.Task -eq $latestTask })) `
            "A corrective packet may only use the most recently closed numeric task T$latestTask."
        $taskRecords = @($numeric | Where-Object { $_.Task -eq $packet.Task })
        $expectedSubtask = ($taskRecords | Measure-Object -Property Subtask -Maximum).Maximum + 1
        Require ($packet.Subtask -eq $expectedSubtask) `
            "A corrective packet must use T$($packet.Task) S$expectedSubtask."
        return
    }
    if ($mode -eq 'Owner-Reopen') {
        Require ($openTask -eq $packet.Task) "An owner-reopen packet must retain progress only for its reopened task."
        Require ($latestTask -gt 1 -and $packet.Task -eq ($latestTask - 1) -and
            ($closureRows | Where-Object { -not $_.HasSubtask -and $_.Task -eq $latestTask })) `
            "An owner-reopen packet requires the immediately prior task and a withdrawn successor."
        $taskRecords = @($numeric | Where-Object { $_.Task -eq $packet.Task })
        $expectedSubtask = ($taskRecords | Measure-Object -Property Subtask -Maximum).Maximum + 1
        Require ($packet.Subtask -eq $expectedSubtask) "An owner-reopen packet must use T$($packet.Task) S$expectedSubtask."
        return
    }
    throw "Numeric task packet Identifier Mode must be New, Continuation, Corrective, or Owner-Reopen."
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
    Set-SelfTestFile $root "README.md" "# NXVM`n`n## Start Here`n`n## Project Boundary"
    Set-SelfTestFile $root "AGENTS.md" "# Agent Instructions`n`n## Authority`n`n## Execution"
    Set-SelfTestFile $root "CONTRIBUTING.md" "# Contributing`n`n## Change Submission`n`n## Review Record`n`n## Commits And Tracking"
    Set-SelfTestFile $root "THIRD_PARTY_NOTICES.md" "# Third-Party Notices"
    Set-SelfTestFile $root "test/README.md" "# Test Directory"
    Set-SelfTestFile $root "docs/README.md" "# Documentation Guide`n`n## Task Reading Set`n`n[Current](states/CURRENT.md)`n[Execution](rules/EXECUTION.md)`n[Contributing](../CONTRIBUTING.md)`n`n## Daily Operation`n`n## Supporting Detail"
    Set-SelfTestFile $root "docs/states/QUEUE.md" "# Queue`n`n1. [Candidate work](../proposals/candidate.md)"
    Set-SelfTestFile $root "docs/states/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [ ] **Fixture debt (`TODO(High)`).** Admit only with evidence."
    Set-SelfTestFile $root "docs/proposals/candidate.md" "# Candidate Work"
    Set-SelfTestFile $root "docs/states/CURRENT.md" @'
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
    Set-SelfTestFile $root "docs/rules/ARCHITECTURE.md" "# Architecture Rules`n`n## Non-Negotiable Invariants`n`n## Source And Research Admission"
    Set-SelfTestFile $root "docs/rules/CODING.md" "# Coding Standard`n`n## Source Discipline`n`n## Test Boundaries"
    Set-SelfTestFile $root "docs/rules/DOCUMENT.md" "# Documentation Rules`n`n## Authority Boundaries"
    Set-SelfTestFile $root "docs/rules/EXECUTION.md" "# Execution Policy`n`n## Request Lifecycle`n`n## Roles And Execution Cycle`n`n## Change Discipline`n`n## Similar-Issue Sweep`n`n## Work Identifiers`n`n## Linear Identifier Allocation`n`n## Documentation Governance Gate`n`n## Milestone Closure Evidence`n`n## Build Tree Hygiene`n`n## Recorder Trace Containment"
    Set-SelfTestFile $root "docs/design/GOAL.md" "# Project Goals`n`n1. Strategic outcome"
    Set-SelfTestFile $root "docs/design/ARCHITECTURE.md" "# System Architecture`n`n## Product Shape`n`n## Modules, Ownership, And Assembly`n`n## Product And Host Boundary`n`n## Runtime Admission Boundary"
    Set-SelfTestFile $root "docs/design/CODING.md" "# Source Layout`n`n## Current And Target Trees`n`n## Files And Names`n`n## Source Organization"
    Set-SelfTestFile $root "docs/design/UI.md" "# Product UX`n`n## NXVM`n`n## NXVDM`n`n## Presentation And Debugging`n`n## Host Resources"
    Set-SelfTestFile $root "docs/design/ROADMAP.md" "# Roadmap`n`n## M0: Governance Reset"
    Set-SelfTestFile $root "docs/etc/README.md" "# Supporting Documentation Index"
    New-Item -ItemType Directory -Force -Path (Join-Path $root "docs/history") | Out-Null
    git -C $root init -q
    git -C $root config core.autocrlf false
    git -C $root config user.email "fixture@example.invalid"
    git -C $root config user.name "Documentation Fixture"
    git -C $root add .
    git -C $root commit -q -m "M5 Td S49 P1: fixture baseline"
    git -C $root commit --allow-empty -q -m "M5 T300 S4 P1: fixture baseline"
}

if ($SelfTest) {
    Require (Test-Mojibake (([char]0x00E2).ToString() + "quoted")) `
        "Mojibake detector did not reject the controlled negative sample."
    Require (-not (Test-Mojibake "ASCII only")) `
        "Mojibake detector rejected an ASCII control sample."
    foreach ($machineLocalPath in @(
            'C:\Users\alice\file.txt',
            'D:\home\example',
            'O:\assets\fdd.img',
            '/home/alice/file.txt',
            '\\server\share\file.txt'
        )) {
        Require (Test-MachineLocalPath $machineLocalPath) `
            "Machine-local path detector did not reject: $machineLocalPath"
    }
    Require (-not (Test-MachineLocalPath 'C:\NAME.EXT')) `
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
        Set-SelfTestFile $fixtureRoot "test/README.md" "# Test Directory`n`nC:\Users\alice\private"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a machine-local path in a test README."
        Set-SelfTestFile $fixtureRoot "test/README.md" "# Test Directory"
        $validStatus = Get-Content -Raw -LiteralPath (Join-Path $fixtureRoot "docs/states/CURRENT.md")
        Set-SelfTestFile $fixtureRoot "docs/design/ARCHITECTURE.md" "# System Architecture"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a principal document without required sections."
        Set-SelfTestFile $fixtureRoot "docs/design/ARCHITECTURE.md" "# System Architecture`n`n## Product Shape`n`n## Modules, Ownership, And Assembly`n`n## Product And Host Boundary`n`n## Runtime Admission Boundary"
        Set-SelfTestFile $fixtureRoot "docs/etc/unindexed.md" "# Unindexed Support"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted an unindexed supporting file."
        Remove-Item -LiteralPath (Join-Path $fixtureRoot "docs/etc/unindexed.md") -Force
        Set-SelfTestFile $fixtureRoot "docs/README.md" "# Documentation Guide`n`n## Task Reading Set`n`n[Current](states/CURRENT.md)`n[Execution](rules/EXECUTION.md)`n[Contributing](../CONTRIBUTING.md)`n[missing](missing.md)`n`n## Daily Operation`n`n## Supporting Detail"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a broken relative Markdown link."
        Set-SelfTestFile $fixtureRoot "docs/README.md" "# Documentation Guide`n`n## Task Reading Set`n`n[Current](states/CURRENT.md)`n[Execution](rules/EXECUTION.md)`n[Contributing](../CONTRIBUTING.md)`n`n## Daily Operation`n`n## Supporting Detail"
        Set-SelfTestFile $fixtureRoot "docs/states/QUEUE.md" "# Queue`n`n1. T301 is not allowed here"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a Queue task identifier."
        Set-SelfTestFile $fixtureRoot "docs/states/QUEUE.md" "# Queue`n`n1. [Candidate work](../proposals/candidate.md)"
        Set-SelfTestFile $fixtureRoot "docs/proposals/orphan.md" "# Orphaned Candidate"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a proposal not linked from Queue."
        Remove-Item -LiteralPath (Join-Path $fixtureRoot "docs/proposals/orphan.md") -Force
        Set-SelfTestFile $fixtureRoot "docs/states/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [x] **Closed (`TODO(High)`).**"
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a completed debt entry."
        Set-SelfTestFile $fixtureRoot "docs/states/TODO.md" "# Long-Term Review Ledger`n`n## Compatibility Debt`n`n- [ ] **Fixture debt (`TODO(High)`).** Admit only with evidence."
        $incompletePacket = $validStatus.Replace(
            "**Idle.**",
            "**Active: M5 Td S50.**"
        ).Replace(
            "## Current Technical Baseline",
            "## M5 Td S50 Packet`n`n| Field | Required record |`n| --- | --- |`n| Identifier Mode | Governance |`n`n## Current Technical Baseline"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $incompletePacket
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted an incomplete active task packet."
        $packetRows = @'
| Field | Required record |
| --- | --- |
| Identifier Mode | Governance |
| Admission And Approval | Fixture approval |
| Objective | Fixture objective |
| Non-goals | Fixture non-goals |
| Reference Baseline | T300 |
| Candidate Proposal | Not applicable |
| Files And ABI Surface | Documentation only |
| Applicable Rules | Documentation rules |
| Verification | Fixture gate |
| Expected Markers | Fixture marker |
| Asset Needs | None |
| Reporting Requirements | Fixture reports |
| Stop Conditions | Fixture stop |
| Exit Criteria | Fixture exit |
| Original Owner Request | Fixture request |
| Similar-Issue Sweep | Fixture sweep |
'@
        $validPacket = $validStatus.Replace(
            "**Idle.**",
            "**Active: M5 Td S50.**"
        ).Replace(
            "## Current Technical Baseline",
            "## M5 Td S50 Packet`n`n$packetRows`n## Current Technical Baseline"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $validPacket
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema rejected a complete next-identifier packet."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($validPacket.Replace("| Field | Required record |", "Field | Required record"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted an active packet without the fixed table header."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($validPacket.Replace("M5 Td S50", "M5 Td S51"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a skipped Td identifier."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($validPacket.Replace("M5 Td S50", "M5 T302 S1").Replace("Governance", "New"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a skipped numeric task identifier."
        $activeNumericPacket = $validPacket.Replace("M5 Td S50", "M5 T301 S1").Replace(
            "| Identifier Mode | Governance |",
            "| Identifier Mode | New |"
        ).Replace(
            "| Candidate Proposal | Not applicable |",
            "| Candidate Proposal | [Fixture proposal](../proposals/candidate.md) |"
        ).Replace(
            "| --- | --- |",
            "| --- | --- |`n| T300 | Closed fixture |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $activeNumericPacket
        git -C $fixtureRoot add docs/states/CURRENT.md
        git -C $fixtureRoot commit -q -m "M5 T301 S1 P0: admit fixture"
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema treated a committed active-packet admission as closed."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($activeNumericPacket.Replace(
            "| Candidate Proposal | [Fixture proposal](../proposals/candidate.md) |",
            "| Candidate Proposal | Not applicable |"
        ))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a numbered task packet without a Candidate Proposal link."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $activeNumericPacket
        $latestTaskProgress = $validPacket.Replace(
            "| --- | --- |",
            "| --- | --- |`n| T301 S1 | Fixture progress |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $latestTaskProgress
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema rejected compact progress for the latest open numeric task."
        $betweenSubtasks = [regex]::Replace(
            $activeNumericPacket,
            '(?ms)^## M5 T301 S1 Packet\r?\n.*?(?=^## Current Technical Baseline)',
            ''
        ).Replace(
            "**Active.**",
            "**Active.** T301 remains open between accepted subtasks."
        ).Replace(
            "| --- | --- |",
            "| --- | --- |`n| T301 S1 | Fixture progress |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $betweenSubtasks
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema rejected an open task between accepted subtasks."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($betweenSubtasks.Replace(
            "| T301 S1 | Fixture progress |`n",
            ""
        ))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted an active packetless status without task progress."
        $continuationPacket = $activeNumericPacket.Replace("M5 T301 S1", "M5 T301 S2").Replace(
            "| Identifier Mode | New |",
            "| Identifier Mode | Continuation |"
        ).Replace(
            "| --- | --- |",
            "| --- | --- |`n| T301 S1 | Fixture progress |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $continuationPacket
        Require (Invoke-SelfTestCheck $fixtureRoot) `
            "Documentation schema rejected the next subtask of the latest open numeric task."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($continuationPacket.Replace("M5 T301 S2", "M5 T301 S3"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a skipped continuation subtask identifier."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($continuationPacket.Replace("M5 T301 S2", "M5 T302 S1"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a foreign continuation task identifier."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($activeNumericPacket.Replace(
            "| Identifier Mode | New |",
            "| Identifier Mode | Continuation |"
        ).Replace("M5 T301 S1", "M5 T301 S2"))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted continuation without retained task progress."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" ($continuationPacket.Replace(
            "M5 T301 S2",
            "M5 T302 S1"
        ).Replace(
            "| Identifier Mode | Continuation |",
            "| Identifier Mode | New |"
        ))
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted a new task while the latest numeric task remains open."
        $mixedTaskClosure = $latestTaskProgress.Replace(
            "| T301 S1 | Fixture progress |",
            "| T301 S1 | Fixture progress |`n| T301 | Closed fixture |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $mixedTaskClosure
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted mixed task-progress and task-closure rows."
        $staleTaskProgress = $latestTaskProgress.Replace("T301 S1", "T300 S4")
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $staleTaskProgress
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted progress for a non-latest numeric task."
        $multipleTaskProgress = $latestTaskProgress.Replace(
            "| T301 S1 | Fixture progress |",
            "| T301 S1 | Fixture progress |`n| T300 S4 | Fixture progress |"
        )
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $multipleTaskProgress
        Require (-not (Invoke-SelfTestCheck $fixtureRoot -Quiet)) `
            "Documentation schema accepted progress for multiple numeric tasks."
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" $validStatus
        Set-SelfTestFile $fixtureRoot "docs/states/CURRENT.md" @'
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
$statesPath = Join-Path $docsRoot "states"
$proposalsPath = Join-Path $docsRoot "proposals"
$statusPath = Join-Path $statesPath "CURRENT.md"
$queuePath = Join-Path $statesPath "QUEUE.md"
$presetPath = Join-Path $RepositoryRoot "CMakePresets.json"
$todoPath = Join-Path $statesPath "TODO.md"

$expectedRootFiles = @("README.md")
$expectedRootDirectories = @("design", "etc", "history", "proposals", "rules", "states")
$expectedStateFiles = @("CURRENT.md", "QUEUE.md", "TODO.md")
$expectedRuleFiles = @("ARCHITECTURE.md", "CODING.md", "DOCUMENT.md", "EXECUTION.md")
$expectedDesignFiles = @("ARCHITECTURE.md", "CODING.md", "GOAL.md", "ROADMAP.md", "UI.md")
$RunDocumentation = $Scope -in @('All', 'Documentation')
$RunGovernanceState = $Scope -in @('All', 'GovernanceState')

if ($RunDocumentation) {
$rootFiles = @(Get-ChildItem -LiteralPath $docsRoot -File | ForEach-Object Name)
$rootDirectories = @(Get-ChildItem -LiteralPath $docsRoot -Directory | ForEach-Object Name)
Require (Test-ExactNameSet $rootFiles $expectedRootFiles) `
    "docs/ must contain only README.md."
Require (Test-ExactNameSet $rootDirectories $expectedRootDirectories) `
    "docs/ must contain only rules/, design/, history/, states/, proposals/, and etc/."

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
Require (Test-ExactNameSet @(Get-ChildItem -LiteralPath $statesPath -File | ForEach-Object Name) $expectedStateFiles) `
    "docs/states/ must contain only CURRENT.md, QUEUE.md, and TODO.md."
Require (@(Get-ChildItem -LiteralPath $statesPath -Directory).Count -eq 0) `
    "docs/states/ must not contain subdirectories."
Require (Test-Path -LiteralPath $etcIndexPath) `
    "docs/etc/ must contain its supporting-documentation index."
$etcIndex = Get-Content -Raw -LiteralPath $etcIndexPath
$etcIndexEntries = @([regex]::Matches($etcIndex, '`(?<entry>[^`]+)`') | ForEach-Object {
    $_.Groups['entry'].Value.Replace('\', '/')
})
foreach ($supportingFile in @(Get-ChildItem -LiteralPath (Split-Path -Parent $etcIndexPath) -Recurse -File -Filter "*.md")) {
    if ($supportingFile.FullName -eq (Resolve-Path $etcIndexPath).Path) {
        continue
    }
    $supportingRoot = (Resolve-Path (Split-Path -Parent $etcIndexPath)).Path.TrimEnd('\')
    $relativePath = $supportingFile.FullName.Substring($supportingRoot.Length).TrimStart('\').Replace('\', '/')
    $covered = (@($etcIndexEntries | Where-Object {
        if ($_.EndsWith('/')) {
            return $relativePath.StartsWith($_, [System.StringComparison]::OrdinalIgnoreCase)
        }
        return [System.Management.Automation.WildcardPattern]::new(
            $_,
            [System.Management.Automation.WildcardOptions]::IgnoreCase
        ).IsMatch($relativePath)
    }).Count -gt 0)
    Require $covered `
        "Supporting Markdown file is not covered by docs/etc/README.md: $relativePath"
}
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
$docsReadme = Get-Content -Raw -LiteralPath (Join-Path $docsRoot "README.md")
$documentRules = Get-Content -Raw -LiteralPath (Join-Path $rulesPath "DOCUMENT.md")
$executionRules = Get-Content -Raw -LiteralPath (Join-Path $rulesPath "EXECUTION.md")

Require ($docsReadme -match '(?m)^## Task Reading Set$') `
    "docs/README.md must contain the Task Reading Set section."
foreach ($requiredReadingLink in @('states/CURRENT.md', 'rules/EXECUTION.md', '../CONTRIBUTING.md')) {
    Require ($docsReadme -match [regex]::Escape("($requiredReadingLink)")) `
        "docs/README.md Task Reading Set must link to $requiredReadingLink."
}

Require-HeadingSchema "README.md" $rootReadme "NXVM" @(
    '^Introduction$',
    '^Platform$',
    '^Build$',
    '^Quick Start$',
    '^Components$',
    '^Historical Application Captures$',
    '^Start Here$',
    '^Project Boundary$',
    '^References$'
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
Require-HeadingSchema "docs/README.md" $docsReadme "Documentation Guide" @(
    '^Task Reading Set$',
    '^Orientation Map$',
    '^Daily Operation$',
    '^Supporting Detail$'
)
Require-HeadingSchema "docs/rules/ARCHITECTURE.md" $architectureRules "Architecture Rules" @(
    '^Non-Negotiable Invariants$',
    '^Source And Research Admission$'
)
Require-HeadingSchema "docs/rules/CODING.md" $codingRules "Coding Standard" @(
    '^Source Discipline$',
    '^Test Boundaries$'
)
Require-HeadingSchema "docs/rules/DOCUMENT.md" $documentRules `
    "Documentation Rules" @('^Authority Boundaries$')
Require-HeadingSchema "docs/rules/EXECUTION.md" $executionRules `
    "Execution Policy" @(
        '^Request Lifecycle$',
        '^Roles And Execution Cycle$',
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

Require-RequiredH2 "README.md" $rootReadme @('^Start Here$', '^Project Boundary$')
Require-RequiredH2 "AGENTS.md" $agents @('^Authority$', '^Execution$')
Require-RequiredH2 "CONTRIBUTING.md" $contributing @(
    '^Change Submission$', '^Review Record$', '^Commits And Tracking$'
)
Require-RequiredH2 "docs/README.md" $docsReadme @(
    '^Task Reading Set$', '^Daily Operation$', '^Supporting Detail$'
)
Require-RequiredH2 "docs/rules/ARCHITECTURE.md" $architectureRules @(
    '^Non-Negotiable Invariants$', '^Source And Research Admission$'
)
Require-RequiredH2 "docs/rules/CODING.md" $codingRules @(
    '^Source Discipline$', '^Test Boundaries$'
)
Require-RequiredH2 "docs/rules/DOCUMENT.md" $documentRules @('^Authority Boundaries$')
Require-RequiredH2 "docs/rules/EXECUTION.md" $executionRules @(
    '^Request Lifecycle$', '^Roles And Execution Cycle$', '^Change Discipline$', '^Similar-Issue Sweep$',
    '^Work Identifiers$', '^Linear Identifier Allocation$',
    '^Documentation Governance Gate$', '^Milestone Closure Evidence$',
    '^Build Tree Hygiene$', '^Recorder Trace Containment$'
)
Require-RequiredH2 "docs/design/ARCHITECTURE.md" $architectureDesign @(
    '^Product Shape$', '^Modules, Ownership, And Assembly$',
    '^Product And Host Boundary$', '^Runtime Admission Boundary$'
)
Require-RequiredH2 "docs/design/CODING.md" $codingDesign @(
    '^Current And Target Trees$', '^Files And Names$', '^Source Organization$'
)
Require-RequiredH2 "docs/design/UI.md" $uiDesign @(
    '^NXVM$', '^NXVDM$', '^Presentation And Debugging$', '^Host Resources$'
)

Require-NoTaskIdentifier "docs/states/QUEUE.md" (Get-Content -Raw -LiteralPath $queuePath)
Require-NoTaskIdentifier "docs/design/GOAL.md" $goalDesign
Require-NoTaskIdentifier "docs/design/UI.md" $uiDesign
Require-NoTaskIdentifier "docs/design/ROADMAP.md" $roadmapDesign
Require-NoChecklist "docs/states/QUEUE.md" (Get-Content -Raw -LiteralPath $queuePath)
Require-NoChecklist "docs/design/GOAL.md" $goalDesign
Require-NoChecklist "docs/design/ARCHITECTURE.md" $architectureDesign
Require-NoChecklist "docs/design/CODING.md" $codingDesign
Require-NoChecklist "docs/design/UI.md" $uiDesign
Require-NoChecklist "docs/design/ROADMAP.md" $roadmapDesign
Require (([regex]::Matches($goalDesign, '(?m)^\d+\.\s+')).Count -gt 0) `
    "Project Goals must contain an ordered strategic-outcome list."
Require ((@((Get-MarkdownHeadings $roadmapDesign) | Where-Object { $_.Level -eq 2 })).Count -gt 0) `
    "Roadmap must contain at least one milestone section."
}

if ($RunGovernanceState) {
$status = Get-Content -Raw -LiteralPath $statusPath
$queue = Get-Content -Raw -LiteralPath $queuePath
$todo = Get-Content -Raw -LiteralPath $todoPath

Require-HeadingSchema "docs/states/CURRENT.md" $status "Project Status" @(
    '^Current Work$',
    '^Current Technical Baseline$',
    '^Recent M\d+ Closures$',
    '^Recent Governance$',
    '^M\d+ (?:T\d+|Td) S\d+ Packet$'
)
Require-HeadingSchema "docs/states/QUEUE.md" $queue "Queue" @('^M\d+ Candidates$')
Require-HeadingSchema "docs/states/TODO.md" $todo "Long-Term Review Ledger" @('^.+ Debt$')

Require (($status | Select-String -AllMatches -Pattern '(?m)^## Current Technical Baseline$').Matches.Count -eq 1) `
    "CURRENT.md must contain exactly one Current Technical Baseline heading."

$activePacket = $null
$idle = $status -match '(?m)^\*\*Idle\.'
if ($idle) {
    Require (-not ($status -match '(?m)^## (Historical )?T\d+(?: S\d+)? Packet$')) `
        "Idle CURRENT.md must not retain a task packet."
    $baselineOffset = $status.IndexOf("## Current Technical Baseline")
    Require ($baselineOffset -ge 0) "Idle CURRENT.md must contain a technical baseline."
    $idlePrefixLines = ([regex]::Split($status.Substring(0, $baselineOffset), "`r?`n")).Count
    Require ($idlePrefixLines -le 8) `
        "Idle CURRENT.md must not retain completed narrative before its technical baseline."
}
else {
    $packetCount = ($status | Select-String -AllMatches -Pattern '(?m)^## M\d+ (?:T\d+|Td) S\d+ Packet$').Matches.Count
    Require ($packetCount -le 1) "Active CURRENT.md must not contain multiple task packets."
    if ($packetCount -eq 1) {
        $activePacket = Get-ActivePacket $status
        Require ($null -ne $activePacket) "Active CURRENT.md must expose a parseable task packet."
        Require-ActivePacketSchema $activePacket
        Require-ActiveIdentifier $activePacket $RepositoryRoot $status
    }
    else {
        $closureRows = @(Get-StatusClosureRows $status)
        $betweenSubtasks = @($closureRows | Where-Object HasSubtask)
        $taskClosures = @($closureRows | Where-Object { -not $_.HasSubtask })
        Require (($betweenSubtasks.Count -eq 1) -or
            ($betweenSubtasks.Count -eq 0 -and $taskClosures.Count -gt 0)) `
            "CURRENT.md without a task packet must retain either one open-task progress row or a task-level closure."
    }
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
$queueEntries = @([regex]::Matches($queue, '(?m)^\d+\.\s+(?<entry>.+)$'))
Require ($queueEntries.Count -gt 0) "QUEUE.md must contain at least one ordered candidate."
foreach ($queueEntry in $queueEntries) {
    Require ($queueEntry.Groups['entry'].Value -match '\]\(\.\./proposals/[^)]+\.md\)') `
        "Every QUEUE.md candidate must link to one proposal."
}
foreach ($proposalFile in @(Get-ChildItem -LiteralPath $proposalsPath -File -Filter '*.md')) {
    Require ($queue -match [regex]::Escape("../proposals/$($proposalFile.Name)")) `
        "Every docs/proposals/ file must be linked from QUEUE.md: $($proposalFile.Name)"
}
$closureSection = [regex]::Match($status, '(?ms)^## Recent M\d+ Closures\r?\n(?<body>.*?)(?=^## |\z)')
Require ($closureSection.Success) "CURRENT.md must contain a recent milestone-closure section."
$closureRows = @(Get-StatusClosureRows $status)
$subtaskProgressRows = @($closureRows | Where-Object HasSubtask)
if ($subtaskProgressRows.Count -gt 0) {
    $progressTaskNumbers = @($subtaskProgressRows | ForEach-Object Task | Select-Object -Unique)
    Require ($progressTaskNumbers.Count -eq 1) `
        "CURRENT.md may retain subtask progress for only one open numeric task."
    $progressTask = $progressTaskNumbers[0]
    $closedNumericRecords = @(Get-ClosedIdentifierRecords $RepositoryRoot | Where-Object { -not $_.IsDocumentation })
    $latestNumericTask = ($closedNumericRecords | Measure-Object -Property Task -Maximum).Maximum
    Require (($progressTask -eq $latestNumericTask) -or
        $status -match '(?m)^\|\s*Identifier Mode\s*\|\s*Owner-Reopen\s*\|\s*$') `
        "CURRENT.md subtask progress must belong to the latest open numeric task."
    Require (-not ($closureRows | Where-Object { -not $_.HasSubtask -and $_.Task -eq $progressTask })) `
        "CURRENT.md must replace closed-task subtask progress with one task-level summary."
}
$taskClosureCount = @($closureRows | Where-Object { -not $_.HasSubtask }).Count
Require ($taskClosureCount -le 8) "CURRENT.md must retain at most eight recent task-level closure rows."

$governanceSection = [regex]::Match($status, '(?ms)^## Recent Governance\r?\n(?<body>.*?)(?=^## |\z)')
Require ($governanceSection.Success) "CURRENT.md must contain a Recent Governance section."
$governanceCount = ([regex]::Matches($governanceSection.Groups['body'].Value, '(?m)^- \*\*M\d+ Td S\d+:')).Count
Require ($governanceCount -le 8) "CURRENT.md must retain at most eight recent governance rows."

$presets = Get-Content -Raw -LiteralPath $presetPath | ConvertFrom-Json
$currentPreset = @($presets.buildPresets | Where-Object { $_.name -eq "current-gcc" })
Require ($currentPreset.Count -eq 1) "CMakePresets.json must define exactly one current-gcc preset."
$currentTarget = @($currentPreset[0].targets)[0]
Require ($currentTarget -match '^vm-0-5-\d{4}$') "current-gcc must select one vm-0-5-NNNN target."
$expectedArtifact = "nxvm_" + ($currentTarget.Substring(3) -replace '-', '_') + ".exe"

$statusTargets = @([regex]::Matches($status, '\bvm-0-5-\d{4}\b') | ForEach-Object Value | Select-Object -Unique)
$statusArtifacts = @([regex]::Matches($status, '\bnxvm_0_5_\d{4}\.exe\b') | ForEach-Object Value | Select-Object -Unique)
Require ($statusTargets.Count -eq 1 -and $statusTargets[0] -eq $currentTarget) `
    "CURRENT.md current target must match CMakePresets.json ($currentTarget)."
Require ($statusArtifacts.Count -eq 1 -and $statusArtifacts[0] -eq $expectedArtifact) `
    "CURRENT.md current artifact must match CMakePresets.json ($expectedArtifact)."
}

if ($RunDocumentation) {
$markdownFiles = @(
    Get-Item -LiteralPath $rootReadmePath
    Get-Item -LiteralPath $agentsPath
    Get-Item -LiteralPath $contributingPath
    Get-Item -LiteralPath (Join-Path $RepositoryRoot "THIRD_PARTY_NOTICES.md") -ErrorAction SilentlyContinue
    Get-ChildItem -LiteralPath $docsRoot -Recurse -File -Filter "*.md"
    Get-ChildItem -LiteralPath (Join-Path $RepositoryRoot "test") -Recurse -File -Filter "*.md" -ErrorAction SilentlyContinue
    Get-ChildItem -LiteralPath (Join-Path $RepositoryRoot "tools") -Recurse -File -Filter "*.md"
) | Sort-Object -Property FullName -Unique
$mojibake = $markdownFiles |
    Where-Object { Test-Mojibake (Get-Content -Raw -LiteralPath $_.FullName) }
Require ($null -eq $mojibake) "Documentation contains mojibake: $($mojibake.FullName -join ', ')"
$localPaths = $markdownFiles |
    Where-Object { Test-MachineLocalPath (Get-Content -Raw -LiteralPath $_.FullName) }
Require ($null -eq $localPaths) "Documentation contains machine-local paths: $($localPaths.FullName -join ', ')"
Require-RelativeMarkdownLinks $RepositoryRoot @($markdownFiles | ForEach-Object FullName)
}

$scopeTarget = if ($RunGovernanceState) { $currentTarget } else { 'documentation scope' }
Write-Output "Documentation governance checks passed for $scopeTarget."
