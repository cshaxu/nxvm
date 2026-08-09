# Execution Policy

This file owns ntvdm64's mandatory request lifecycle, identifiers, evidence,
artifact, and milestone gates. The public
[Execution Governance skill](https://github.com/cshaxu/skills/blob/main/execution-governance/SKILL.md)
is supplementary reusable guidance, not a prerequisite. One subtask is active
at a time. Follow the local requirement packet and closure audit defined here.
Before implementation, create a subtask record
with objective, non-goals, reference baseline, files/ABI surface, applicable
local architecture, coding, source, and execution rules, exact test commands,
expected markers, asset needs, and stop conditions. The rules review names each
applicable invariant, its planned evidence, and any requested owner-approved
exception; a task may mark a rule not applicable only with a short reason.

## Request Lifecycle

Answer a question, discussion, or exploration normally. When an owner requests
one or more changes, preserve the request, inspect the current system and
applicable rules, and split the work into bounded tasks with explicit exit
criteria before implementation. Obtain approval before admitting the work to
`STATUS.md`; only that one active packet may execute.

During execution, record discovered repository issues. Fix a clear in-scope
instance as an added subtask and run the similar-issue sweep; otherwise record
the issue in `TODO.md` with priority, boundary, admission condition, and risk.
At closure, re-read the original owner request, map every requested outcome to
evidence, run the required verification, record the retrospective/prevention
action when applicable, and report the result. A task is not complete merely
because its implementation or tests look complete.

## Change Discipline

Structural relocation uses `git mv`: repair direct includes and build paths,
run the relevant checks, then continue with the next bounded move. A deferred
issue uses `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only with a ledger entry
and bounded admission path. Do not leave an equivalent production defect
unclassified after a local fix; apply the similar-issue sweep below.

## Similar-Issue Sweep

Every implementation task that fixes a source, behavioral, compatibility, or
architecture defect must close the defect class, not only its first observed
site. Its active task packet and completion evidence must record a
**similar-issue sweep** containing:

- the defect class and the original reproducer or observation;
- the repository scope and exact search commands or static queries used. The
  default scope is all tracked production source, tests, build descriptions,
  and task/governance records relevant to the defect class; every exclusion
  needs a short reason;
- every production-path hit and its disposition: fixed, not applicable with a
  reason, or deferred into `TODO.md` with priority, owner/boundary, and an
  admission task or gate; and
- the post-fix result, including a focused regression. When the defect class
  is mechanically detectable, add or update a closure/static gate so the same
  forbidden shape cannot return through another path.

The sweep is not a request for unrelated cleanup: a hit outside the active
scope is deferred or causes an owner-approved scope change. A task may mark the
sweep not applicable only for work that cannot correct a defect class, such as
standalone documentation; it must state that reason. A defect-fix task cannot
complete with an undocumented production hit or an untracked deferral.

## Work Identifiers

Numeric `T` identifiers are reserved for implementation tasks. A standalone
governance, roadmap, architecture, audit, or other documentation task is
identified as `M<milestone> Td`, where `d` literally means documentation. It
does not allocate a numeric task identifier, advance a task-version artifact,
or create a runnable-output requirement. It also has no task-history record;
its commit and current-status summary are its durable evidence. A multi-step
documentation task may use `M<milestone> Td S<subtask>`.

Design or contract work that is an explicit prerequisite of implementation
belongs to the admitted implementation task's subtask sequence. Before that
admission it remains an unnumbered Queue candidate; do not allocate a numeric
`T` merely to make prospective work look task-shaped.

Commit subjects are `M<milestone> Td [S<subtask>] P<part>: description` for a
standalone documentation task, or `M<milestone> T<task> S<subtask> P<part>:
description` for an implementation task and its task-specific design work.

## Linear Identifier Allocation

`QUEUE.md` holds ordered but unnumbered candidates. Allocate the next numeric
task identifier only when the owner approves one candidate and it becomes the
single active packet in `STATUS.md`. Reordering, withdrawing, or refining a
candidate consumes no identifier. Numeric tasks therefore exist only while
active and after closure in implementation history. Allocate them in strict
ascending order; never skip an identifier and never reopen a closed task or
subtask identifier.

Allocate implementation subtask (`S`) and standalone documentation (`Td S`)
identifiers strictly after the latest closed identifier in their own sequence.
Historical records do not become reusable capacity. The existing pre-policy
`M5 Td S43` record is an archival discontinuity: `S42` is permanently
unavailable. Later governance identifiers continue from the latest closed
identifier and never fill that gap.

The only corrective exception is narrow: when there is no active task, the
most recently closed **numeric** task may receive its next unused subtask for
an in-scope corrective repair. It must preserve that task's scope, run its
normal evidence, update that task's developer artifact and SHA-256 record, and
close again before a new task is admitted. It cannot reuse a closed `S`, fill a
missing identifier, add unrelated work, or bypass a queued task. Standalone
`Td` work and older numeric tasks have no such exception.

A design milestone is not complete until it has produced the bounded Task and
subtask breakdown for its immediate implementation milestone. The breakdown
maps each task to an approved decision, contract, regression set, acceptance
result, and stop condition. Do not pre-create or activate implementation tasks
from an earlier milestone: their shape is an output of the preceding design
gate, not an early roadmap prediction. Research work likewise approves one
bounded breakdown for each admitted implementation increment.

A subtask completes only when scoped behavior works, focused tests pass,
established corpus entries remain valid, applicable local rule invariants pass
or have an owner-approved exception recorded, and its commit records compact
tracking, verification, and provenance facts where needed. A later governed
history-consolidation task may replace completed per-task documents with a
milestone summary; Git history remains the detailed evidence source.
Documentation/design tasks apply only the rules relevant to their artifact and
do not manufacture runtime verification requirements. Standalone `Td` work
does not create a task artifact or change the current artifact version.

## Documentation Governance Gate

Every task closure and standalone `Td` closure runs:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .
```

The configured `current-gates-gcc` target also runs this check when PowerShell
is available. It verifies that `STATUS.md` is the sole current-baseline
authority, an Idle status has no retained packet, the queue contains no numeric
task identifier, the debt ledger contains no completed row, history makes no
current-baseline claim, status matches the configured current artifact target,
no known mojibake is present, and the status closure table remains at eight rows
or fewer. A failure blocks closure until the documents are made internally
consistent.

## Milestone Closure Evidence

Before an implementation milestone closes, its final verification record must
map the current source graph to the roadmap exit conditions, applicable rules,
open TODO deferrals, and current evidence. A historical passing task, a clean
compile, or a fixture-only smoke cannot close a changed runnable path. A
milestone-specific closure checklist may be indexed in `etc/` as supporting
evidence, but it does not supersede this policy or the local architecture,
coding, source, and execution authorities.

A configured smoke gate must state whether it builds, executes, or statically
inspects its subject. A preset presented as a runtime gate must invoke its
registered CTest cases and fail on their nonzero results; compiling smoke
executables alone is build coverage, not smoke evidence. Source-shape and
inventory checks remain explicit static gates rather than being described as
runtime tests.

Each completed implementation task that changes a runnable path must compile,
verify, and copy one usable task-level local build output to the ignored
`build/output/` directory. Every numeric implementation task uses its task
identifier as the four-digit developer-artifact revision: task `T258` produces
`0.5.0258`. This is an identity rule, not a sequence rule.
All implementation subtasks rebuild their task-level artifact under that same
revision; the source commit and recorded SHA-256 identify the exact build.
Task numbers are never reused, so a completed task artifact version cannot
collide with another task's version. The linear allocation rule above also
prohibits creating task identifiers out of queue order.

Use `nxvm_0_5_NNNN.exe` for the bootable VM product or
`nxvdm_0_5_NNNN.exe` for the DOS app-runner product. Every task record maps
its task identifier to its allocated artifact revision. Earlier historical
artifacts retain their recorded names and banners. They are evidence files, not
CMake build targets: current source must never rebuild an old task/version
name. The configured current-artifact target is the only product artifact
target admitted to the active CMake graph.
Record the artifact SHA-256, source commit, runtime identity/banner, and
whether it is a baseline/developer artifact or a product artifact in the
verification record. Smoke-test executables remain build-tree verification
tools and are never copied as developer artifacts. Design-only tasks do not
manufacture executables. Local artifacts are never release evidence, must not
bundle protected media or Microsoft binaries, and may be replaced only by a
newly verified build of the same named task.

## Build Tree Hygiene

After every build, test, smoke run, sanitizer run, or failed verification,
remove owned temporary build products as soon as they are no longer required
by the active task or its immediate next task. `build/output/` is the sole
preserved local artifact directory and retains verified task executables. All
other build configuration trees, object files, generated test executables,
logs, traces, sanitizer trees, and stale CMake/Ninja state are disposable
unless an active subtask records why they are needed. Before recursive cleanup,
verify the resolved target is below `build/`, exclude `build/output/`, and
confirm no owned process still uses it.

For a runnable artifact, the verification record also states the emitted runtime
identity/banner and version. It must follow the task-version rules above;
changing identity, version, or cutover state without an approved subtask and
regression evidence is prohibited.

For a legacy coupled system, first establish and record a runnable full-source
baseline before subtractive refactoring. A baseline import may be isolated from
the final module layout, but it cannot gain new product behavior or weaken
source, asset, licensing, or test rules.

Differential debugging is a bounded verification experiment. Its record names
both implementations, inputs, event schema, checkpoints, comparison masks,
instruction/time/no-progress budgets, and cleanup owner. It cannot become a
runtime dependency or replace focused project-owned tests.

## Recorder Trace Containment

Raw instruction recording is an ignored, potentially unbounded diagnostic
artifact. A recorder run is prohibited unless its subtask record declares a
unique ignored output path, wall-clock, no-progress, and maximum-byte budget,
the process-tree cleanup owner, and the checkpoint data to retain after the raw
trace is deleted. The byte budget is a hard limit: a cooperative recorder must
stop before it; otherwise the host harness monitors file growth and terminates
the entire launched process tree at the first exceeded limit. A timeout alone
is not sufficient.

Use a fresh run-specific name under ignored `build/` or `artifacts/`; never
reuse a trace path from an earlier run. Before launch, reserve at least twice
the declared byte budget as free workspace space. After every completion,
timeout, failure, or cancellation, the harness must wait for process exit,
verify that the trace handle is closed, record the final size/checkpoint in the
compact verification record, and delete the raw trace unless the approved
subtask explicitly retains it for immediate diagnosis. The next recorder run is
blocked while a prior owned process or trace remains.

Legacy recorders without an in-process byte limit are diagnostic-only and must
run through this monitored harness. Their raw output is never a fixture,
baseline, release artifact, or committed evidence.

Do not advance a milestone merely because individual code exists. The roadmap
owns milestone goal, scope, and exit conditions; its active subtask owns exact
commands, expected markers, budgets, and stop conditions. `breakdown.md` is an
index only and cannot close a milestone.
When a release cadence is established, an owner-designated implementation
milestone creates an immutable snapshot branch from a verified `main` commit
before its successor begins. Design and research work has no default snapshot
obligation, though the owner may require one for a specific approved
deliverable.

Escalate to the owner if a license choice, protected asset, incompatible source
license, undocumented Windows mechanism, or change to the direct-launch product
boundary becomes necessary.
