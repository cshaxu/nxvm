# Execution Policy

This file owns ntvdm64's mandatory request lifecycle, identifiers, evidence,
artifact, and milestone gates. The public
[Execution Governance skill](https://github.com/cshaxu/skills/blob/main/execution-governance/SKILL.md)
is supplementary reusable guidance, not a prerequisite. One subtask is active
at a time. Follow the local requirement packet and closure audit defined here.
Before implementation, the coordinator creates the one active subtask packet
in `states/CURRENT.md` as the fixed two-column `Field | Required record` table;
the executor reads, questions, and executes that packet. It must record
identifier mode, admission and owner approval, objective, non-goals, reference
baseline, candidate proposal, files/ABI surface, applicable rules, exact
verification, expected markers, asset needs, reporting requirements, stop
conditions, exit criteria, original owner request, and the similar-issue sweep.
The gate rejects a packet missing a field, its fixed table shape, or an empty
record value. The rules review names each applicable invariant, its planned
evidence, and any requested owner-approved exception; a task may mark a rule
not applicable only with a short reason.

The exact packet fields, in the required table's first column, are: `Identifier
Mode`, `Admission And Approval`, `Objective`, `Non-goals`, `Reference
Baseline`, `Candidate Proposal`, `Files And ABI Surface`, `Applicable Rules`, `Verification`,
`Expected Markers`, `Asset Needs`, `Reporting Requirements`, `Stop Conditions`,
`Exit Criteria`, `Original Owner Request`, and `Similar-Issue Sweep`.

For this policy, the owner is the person or authority that issued or approved
the original request. `Admission And Approval` identifies that owner, the
approval source and date or baseline, the approved scope, and every allowed
exception. A non-empty table cell is not evidence of approval by itself.

## Request Lifecycle

Answer a question, discussion, or exploration normally. When an owner requests
one or more changes, preserve the request, inspect the current system and
applicable rules, and split the work into bounded tasks with explicit exit
criteria before implementation. Obtain approval before admitting the work to
`states/CURRENT.md`; only that one active packet may execute.

During execution, the executor reports discovered repository issues. For a
clear in-scope instance, the coordinator either revises the active packet/brief
or admits a later subtask, then requires the similar-issue sweep; otherwise the
coordinator records the issue in `states/TODO.md` with priority, boundary,
admission condition, and risk. The executor never adds a subtask itself.
At closure, re-read the original owner request, map every requested outcome to
evidence, run the required verification, record the retrospective/prevention
action when applicable, and report the result. A task is not complete merely
because its implementation or tests look complete.

Before accepting or closing any task or subtask, inspect its actual repository
changes, rather than relying only on a report, test summary, or diff statistic.
Review the relevant added, modified, renamed, and deleted files and the actual
code, build, test, and documentation changes against the packet scope, exit
criteria, and recorded evidence. In a one-session run, the session performs
this review after switching to the coordinator role. When separate sessions
hold the roles, the coordinator performs it independently before acceptance;
the executor's report is an evidence index, not a substitute for reviewing the
actual Git/worktree changes.

**Concise reporting.** Each conversation report states only the information
applicable to its purpose. A completion delivery records completion, pushed
commit, verification result or evidence location, and known boundary or
transfer. A blocker delivery records the blocker, current commit status if
any, relevant evidence, and the requested decision or transfer. Contract
confirmation or objection and S-required progress reports state the applicable
decision or progress node and link to their evidence. Reports link to rather
than reproduce command output, requirement matrices, implementation narratives,
or full evidence. The committed delivery and its indexed evidence retain the
complete requirement-to-proof record. Concise reporting never reduces
self-review, actual-change review, verification, or acceptance requirements.

## Roles And Execution Cycle

Every admitted S follows one coordinator/executor lifecycle with one active
`states/CURRENT.md` packet, the same identifier rules, evidence requirements,
similar-issue sweep, and closure audit. One session may perform both roles in
sequence, or two sessions may each perform one role; session assignment never
changes the lifecycle or authority.

**T, S, and P.**

`states/QUEUE.md` owns the ordered queue of T task packages. A T is a bounded,
owner-approved goal with a stated scope, non-goals, dependencies, completion
standard, and stop condition. It is not a pre-allocation of every implementation
step. The coordinator observes the evidence produced by each admitted S and
dynamically plans the next S within the active T boundary.

An S is the smallest implementation, review, and acceptance unit inside an
active T. Its active `states/CURRENT.md` packet is the required task contract. Each S
brief must state its objective, precise scope, non-goals, authority/baseline,
verification and evidence requirements, quality standard, expected changed
surface, reporting requirements (initial objection or confirmation,
S-specified progress nodes, and final delivery), stop conditions, and exit
criteria. The brief defines task-specific
technical requirements; this policy does not prescribe them. An S must not be
silently turned into a moving target: a material scope, authority, acceptance,
or risk change requires an explicit packet/brief revision before implementation
continues. The coordinator may clarify or consolidate work that remains within
the admitted objective and non-goals. A revision that expands scope, risk,
authority, or exit criteria requires renewed owner approval before work
continues.

The S brief is the task-specific delivery instruction recorded in the active
packet, not an independent contract. A coordinator message may reproduce or
clarify it, but may not add a material requirement without the packet/brief
revision required above.

A P is one sequential, pushed commit within an S. For a numbered implementation
T, the executor forms an implementation P by completing its assigned brief,
performing the required self-review, committing, and immediately pushing that
complete delivery. It is not a planning marker, internal batch, or partial
implementation milestone: it must satisfy the entire assigned brief, including
every required implementation, task documentation update, focused proof,
regression/gate result, and evidence artifact. The coordinator then reviews the
pushed implementation P independently. A standalone Td P is its complete
governance delivery: it may include truthful status closure, and requires no
later coordinator governance P.

Each pushed implementation or governance commit consumes the next `P<part>` in
that S's commit-subject sequence; the two roles share one consecutive sequence.

If coordinator review rejects an implementation P, the coordinator creates one
consolidated corrective brief containing all then-known defects and acceptance
requirements. The executor must question or confirm that brief, then complete,
commit, and push it as the next implementation P before returning it for
review. Do not create a new P for a local vector, fixture diagnosis, individual
gate, admission-artifact step, or executor-owned task-documentation follow-up
that the assigned implementation P should have included. A pushed P is
immutable: a necessary correction uses the next P rather than rewriting
history.

If post-review evidence demonstrates that an accepted and closed S did not meet
its contract, the coordinator does not reopen that S. While its containing T
remains open, the coordinator admits the next bounded corrective S in that T.
After the T has closed, the coordinator uses the narrow corrective-S exception
in **Linear Identifier Allocation** when it applies; otherwise it admits a new
T. The corrective S begins its own implementation-P lifecycle.

For a numbered implementation T, the governance P required after coordinator
acceptance is the sole exception: it is a later P without a rejected
implementation P, records only truthful closure, and adds no implementation
scope. It does not apply to standalone Td work.

When the coordinator accepts a numbered implementation P, the coordinator
updates the applicable Status, history, and governance records, commits and
pushes that purely governance P, and closes the S. The governance P records
acceptance; it does not add implementation scope. Pure documentation, status,
or state changes therefore remain in the current S when they are required for
its closure, but do not by themselves allocate a new T or S.

After pushing the numbered-T governance P, the coordinator reports to the owner
the accepted implementation P, verification conclusion, closed S or T,
transferred or deferred items, and every remaining owner decision. A Td
completion report instead names its complete Td P and governance result.

**Executor completion goal.** An executor creates one durable goal for its
entire executor session: **Complete the coordinator's assignment fully.** It
does not create, replace, reactivate, or update that goal for an S, corrective
brief, implementation P, or resume. The active packet and S brief, rather than
the goal, define the current objective, complete acceptance, stop conditions,
and no-partial-delivery rule. A local smoke, diagnosis, individual gate, or
internal batch does not alter the goal. A material objection or newly discovered
contract change pauses work pending coordinator direction; a corrective brief
then continues under the same goal. The goal is retired only when the executor
session ends or the coordinator transfers or cancels that executor role.

When a separate executor session is assigned, an unreported stop occurs when it
stops active work without reporting progress, a result, or a blocker to the
coordinator. The coordinator checks every five minutes whether that executor is
stopped; when it is and no report was made, the coordinator records one stop
for that S. Reported waiting for coordinator direction is not an unreported
stop.

Only the coordinator may create, admit, re-plan, suspend, close, cancel, or
reorder T packages and their S tasks. The executor may question and execute an
admitted S, and may commit and push only its complete assigned P. It may not
allocate identifiers, admit or expand S scope, create the next S, or close a T.
A finding outside an admitted S is
reported to the coordinator; the coordinator decides whether it is an in-scope
S revision, a later S in the active T, a future T, or a `states/TODO.md` deferral.

A T closes only when its stated completion standard is proven, every in-scope
remaining gap is closed or explicitly transferred through the normal queue/debt
process, and the T-level closure audit completes. Merely completing some S
tasks does not close a T. A T-level audit reviews documentation, code quality,
open debt, task evidence, and applicable rules; it cannot be bypassed by a
passing local implementation.

**Per-S reading index.** Before first work, and again on its first resume, the
executor first follows the [Task Reading Set](../README.md)
and then reads the active packet plus the S-contract, P-lifecycle,
completion-goal, stop, and role-authority paragraphs above. It also reads each
project rule triggered by the packet or change surface. This index is
navigation only; the referenced authorities remain the sole requirements
sources.

**Role cycle.**

1. The coordinator admits a bounded S packet under an approved T or standalone
   Td work, and provides the executor the complete S brief.
2. The executor scrutinizes the brief, relevant routes, existing evidence, and
   task-specific risks. It reports either a material objection or confirmation
   before execution. An objection pauses affected work for coordinator decision
   and, if needed, packet/brief revision; it cannot be overridden by a request
   to continue.
3. After confirmation, the executor works under its durable completion goal,
   sends the S-required progress reports, self-reviews every acceptance
   requirement against actual evidence, then commits and pushes only its
   complete P. A partial implementation, local smoke, diagnosis, internal
   batch, packet preparation, registration lookup, or status update is not a
   completion point.
4. The coordinator reviews the original request, S brief, packet, evidence,
   applicable rules, and the pushed P's actual Git/worktree changes. The
   executor report is only an evidence index. The coordinator either issues one
   consolidated corrective brief or accepts the P and applies the applicable
   P-lifecycle closure above.

A one-session run explicitly switches between these roles and repeats the
coordinator-side actual-change review after the executor-side P; it must not
claim independent review. A two-session run assigns the roles to separate
sessions, so the coordinator review is independent. If a role moves between
sessions mid-S, its handoff records the accepted brief, current evidence,
unresolved objections, and worktree state before the receiving session resumes.
## Change Discipline

Structural relocation uses `git mv`: repair direct includes and build paths,
run the relevant checks, then continue with the next bounded move. A deferred
issue uses `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only with a ledger entry
and bounded admission path. Do not leave an equivalent production defect
unclassified after a local fix; apply the similar-issue sweep below. Only an
admitted standalone `Td` task may modify `docs/rules/`. An implementation T or
S must report a needed rule change for separate Td admission; it may not fold
that change into its implementation or governance closure.

Every successful task or subtask commit must be pushed to its configured remote
immediately. Do not accumulate local completed commits or defer the push until
the end of a task package. A push failure blocks acceptance, task closure, and
completed-result reporting until it is resolved, or until the owner explicitly
approves and the task evidence records a bounded deferral and recovery
condition. It does not block a report that specifically identifies the push
failure and awaits further instruction.

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
  reason, or deferred into `states/TODO.md` with priority, owner/boundary, and an
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

`states/QUEUE.md` holds ordered but unnumbered candidates. A candidate enters
that Queue only after its proposal is stored in `docs/proposals/`. Allocate the
next numeric task identifier only when the owner approves one candidate and it
becomes the single active packet in `states/CURRENT.md`; at that transition,
create its `docs/history/M<milestone>-T<task>-<name>.md` main record. At task
closure, retain the proposal by moving it to
`docs/history/M<milestone>-T<task>-<name>-proposal.md`. When a candidate or
task is withdrawn and will not be implemented, clean up or retain its proposal
and main record according to the documented retention decision. Reordering,
withdrawing, or refining an unapproved candidate consumes no identifier.
Numeric tasks therefore exist only while active and after closure in
implementation history. Allocate them in strict ascending order; never skip an
identifier and never reopen a closed task or subtask identifier.

Allocate implementation subtask (`S`) and standalone documentation (`Td S`)
identifiers strictly after the latest used identifier in their own sequence.
Historical records and any historical discontinuity do not become reusable
capacity. Later identifiers continue from the latest used identifier and never
fill a historical gap.

Git commit subjects prove the immutable `P` history and the highest used `S`.
`states/CURRENT.md` task-level rows determine whether the latest numeric task is
closed: a `| T<n> |` row is closed; a `| T<n> S<m> |` row is retained progress
for the one latest open numeric task. The governance gate combines only those
machine-readable forms. It never infers task state from free prose or from the
mere existence of an implementation commit.

The packet's `Identifier Mode` explicitly declares `New`, `Continuation`,
`Corrective`, or `Governance` so allocation is inspectable before code changes
begin:

- `New` requires the latest numeric task to have a task-level closure row (or
  no numeric history), allocates the next global `T`, and starts at `S1`.
- `Continuation` requires retained progress for the latest open numeric task,
  uses that same `T`, and allocates exactly its next unused `S`.
- `Corrective` requires no open numeric task and may use only the most recently
  closed numeric `T` with that task's next unused `S`.
- `Governance` allocates the next milestone-local `Td S` and never allocates a
  numeric implementation task.

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

The default command composes two diagnostic scopes, either of which may be run
independently while investigating a failure:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Scope Documentation
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot . -Scope GovernanceState
```

`Documentation` verifies topology, principal-document schemas, the Task
Reading Set navigation, `etc/` index coverage, relative Markdown links,
mojibake, and machine-local paths. `GovernanceState` verifies active-packet
fields and identifier continuity, the sole `states/CURRENT.md` technical baseline,
queue/debt boundaries, artifact identity, and capped Status summaries. The
configured `current-gates-gcc` target runs the default combined check when
PowerShell is available. A failure blocks closure until the documents are
internally consistent. The gate verifies structural schemas, not semantic
document ownership; the closure audit must still compare each changed document
against the authority matrix in `docs/rules/DOCUMENT.md`.

Status retention and closure-summary limits follow the
[Documentation Rules](DOCUMENT.md).

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
