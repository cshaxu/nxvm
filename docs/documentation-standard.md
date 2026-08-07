# Documentation Standard

Use the public [Documentation Governance skill](https://github.com/cshaxu/skills/blob/main/documentation-governance/SKILL.md)
for authority, structure, link, and history discipline. This repository keeps
its own current documents because shared skills do not define ntvdm64's product
decisions, task identifiers, source graph, or acceptance evidence.

## Principal Documents

The daily governance surface has exactly seven subjects:

1. [Design Principles](design-principles.md)
2. [Coding Standard](coding-standard.md)
3. [Documentation Standard](documentation-standard.md)
4. [System Architecture](architecture/overview.md)
5. [Roadmap And Milestones](planning/roadmap.md)
6. [Debt Tracking](debt-tracking.md)
7. [Execution Workflow](execution-workflow.md)

`planning/status.md` and `planning/m5-closure-checklist.md` are operational
companions, not additional design authorities.

## ntvdm64 Rules

- One topic has one current authority. Link to it; do not copy it into another
  plan, task note, or history document.
- Current documents state present commitments, boundaries, and acceptance
  conditions. Completed task detail belongs in a compact milestone history or
  Git, never beside current planning.
- A task record is temporary operational scaffolding. On governed completion it
  is summarized or removed; its commit remains the detailed evidence.
- History explains why a decision occurred but cannot override architecture,
  requirements, roadmap, policy, or active status.

## Current-State And Closure Discipline

`planning/status.md` is the sole authority for the current active state,
current technical baseline, selected artifact target, artifact hash, and test
count. History records closure-time facts only; it must label them as archived
and link to status rather than claim to be current. The M5 queue contains only
unstarted tasks, and `TODO.md` contains only open debt or long-horizon gaps.

An active task packet lives only in `status.md`. A closed task packet is
removed in its closure commit after a compact history summary is recorded; the
Git commit remains its detailed evidence. `docs/planning/m5-t*.md` is not a
permanent record location.

Every closure, including standalone `Td` work, runs
`tools/Verify-DocumentationGovernance.ps1`. The checker enforces the single
status baseline, idle-state packet exclusion, retired M5 task-record absence,
queue/history vocabulary boundaries, current CMake artifact-target agreement,
and mojibake rejection.

See [Historical Summaries](history/README.md) for retrieval of retired detail.
