# Documentation Rules

Use the public [Documentation Governance skill](https://github.com/cshaxu/skills/blob/main/documentation-governance/SKILL.md)
for reusable method. This file is the ntvdm64-specific authority.

`docs/README.md` is the sole entry point. `rules/` contains local governance
rules; `design/` contains current project decisions; `STATUS.md`, `QUEUE.md`,
and `TODO.md` have distinct operational roles; `history/` contains numbered
implementation-task records; and `etc/` contains only owner-approved supporting
material indexed by [etc/README.md](../etc/README.md).

`rules/ARCHITECTURE.md` and [design/ARCHITECTURE.md](../design/ARCHITECTURE.md)
are linked rule-versus-project-design authorities. `rules/CODING.md` and
[design/CODING.md](../design/CODING.md) have the same distinction.

- One topic has one current authority. Link instead of copying.
- [STATUS.md](../STATUS.md) alone defines active work, technical baseline,
  artifact identity, and recent closure summaries.
- [QUEUE.md](../QUEUE.md) contains only unnumbered, ordered candidate work.
  [TODO.md](../TODO.md) contains only unplanned debt.
- Each numeric implementation task has one `history/M<milestone>-T<task>-*.md`
  record. Standalone `Td` tasks have no history record; their compact outcome
  is retained in Status and Git.
- `etc/legacy-history/` is read-only aggregation retained during migration. It
  is evidence, never a current authority.

Every closure runs `tools/Verify-DocumentationGovernance.ps1`, link validation,
and `git diff --check`. A failure blocks closure until paths, authorities,
status, queue, debt, and retained-detail indexes agree.
