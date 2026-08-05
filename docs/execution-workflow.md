# Execution Workflow

Use the public [Execution Governance skill](https://github.com/cshaxu/skills/blob/main/execution-governance/SKILL.md)
for request intake, confirmation, task decomposition, verification, discovery,
retrospective, and closure against the original request. This document records
the ntvdm64-specific execution overlay; it works with the
[Roadmap](planning/roadmap.md), [Project Status](planning/status.md),
[Debt Tracking](debt-tracking.md), and [Execution Policy](planning/execution-policy.md).

## Local Task Record

`docs/planning/status.md` contains the one active task packet and preserves the
original request. It records its requirement and risk tables, applicable
`RULES.md` sections, owner-approved exceptions, exact commands, expected
markers, artifact needs, and stop conditions. The public skill defines the
workflow; this repository defines the evidence locations and technical gates.

## Local Identifiers And Evidence

[Execution Policy](planning/execution-policy.md) owns the
`M<milestone> T<task> S<subtask> P<part>` MTSP form, its standalone `Td`
variant, commit subjects, artifact revision allocation, and runnable-output
rules. [Project Status](planning/status.md) is the active ledger;
[Debt Tracking](debt-tracking.md) is the deferred-work ledger.

Each ntvdm64 task packet maps original requirements to evidence and records
the applicable source, test, provenance, toolchain, artifact, and release
constraints. M5 hardware work additionally uses its closure checklist and the
hardware-device verification template.

## Local Closure

Before a completion commit, re-check the active packet against its original
request. Close it only when every local requirement has concrete evidence or
an owner-approved deferral in `TODO.md`; then update its compact history when
appropriate. Standalone `Td` work has the same closure audit but never creates
a runnable artifact.
