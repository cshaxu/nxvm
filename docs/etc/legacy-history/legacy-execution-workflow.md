# Legacy Execution Workflow

> Superseded by [rules/EXECUTION.md](../../rules/EXECUTION.md) and the shared
> execution-governance skill. Retained only for historical retrieval.

This was the former mandatory local workflow. The public
[Execution Governance skill](https://github.com/cshaxu/skills/blob/main/execution-governance/SKILL.md)
may extend it with reusable practice, but never replaces this local contract.
It works with the
[Roadmap](../../design/ROADMAP.md), [Project Status](../../STATUS.md),
[Debt Tracking](../../TODO.md), and [Execution Policy](../../rules/EXECUTION.md).

## Mandatory Local Lifecycle

1. Answer a question, discussion, or exploration normally; do not create a
   task packet for it. For one or more requested changes, preserve the original
   request in the packet and inspect the current source, status, and applicable
   local authorities.
2. Before implementation, split the request into requirements with an owner,
   completion condition, verification method, risks, and explicit non-goals.
   Obtain owner confirmation before changing runtime behavior; an approved goal
   or task packet is that confirmation. Allocate any task identifier through
   the linear rule in the Execution Policy before creating the packet.
3. Keep exactly one subtask active in `status.md`. Execute only its approved
   scope, record commands and evidence, and use the similar-issue sweep when it
   corrects a defect or compatibility class.
4. Record discovered repository issues. Fix a clear in-scope instance as a
   tracked subtask; defer a structural or uncertain issue with priority, owner,
   admission condition, and risk in `TODO.md`.
5. Before closure, re-read the original request. Every requirement must have
   focused evidence or an owner-approved deferral; record the retrospective and
   update local standards when a reusable rule is discovered.

Between a closed task and the admission of the next task, `status.md` must say
**Idle** and contain no task packet. Idle is not an active subtask and may not
be used to conceal incomplete implementation work.

## Local Task Record

`docs/STATUS.md` contains the one active task packet and preserves the
original request. It records its requirement and risk tables, applicable
local architecture, coding, source, and execution rules, owner-approved
exceptions, exact commands, expected markers, artifact needs, and stop
conditions. This repository defines both the minimum workflow and its evidence
locations and technical gates.

For a defect or compatibility fix, the packet also contains the
similar-issue-sweep record required by the Execution Policy: defect class,
repository query, production-hit dispositions, and the planned focused or
static regression. This makes the public discovery practice locally auditable
without turning a task into unrelated cleanup.

## Local Identifiers And Evidence

[Execution Policy](../../rules/EXECUTION.md) owns the
`M<milestone> T<task> S<subtask> P<part>` MTSP form, its standalone `Td`
variant, commit subjects, artifact revision allocation, and runnable-output
rules, including the strict linear allocation rule and its sole corrective
exception. [Project Status](../../STATUS.md) is the active ledger;
[Debt Tracking](../../TODO.md) is the deferred-work ledger.

Each ntvdm64 task packet maps original requirements to evidence and records
the applicable source, test, provenance, toolchain, artifact, and release
constraints. M5 hardware work additionally uses its closure checklist and the
hardware-device verification template.

## Local Closure

Before a completion commit, re-check the active packet against its original
request. Close it only when every local requirement has concrete evidence or
an owner-approved deferral in `TODO.md`. The closure commit atomically records
the compact history summary, updates the sole status baseline, removes the
closed packet, removes completed queue rows and stale TODO implementation
items, and runs `tools/Verify-DocumentationGovernance.ps1`. A defect-fix
closure also verifies that its similar-issue sweep has no undocumented
production hit and that its focused regression or closure gate is recorded.
Standalone `Td` work has the same closure audit but never creates a runnable
artifact.
