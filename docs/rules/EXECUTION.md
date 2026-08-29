# Execution Policy

This file owns the NXVM repository's mandatory request lifecycle, identifiers, evidence,
artifact, and milestone gates. The public
[Execution Governance skill](https://github.com/cshaxu/skills/blob/main/execution-governance/SKILL.md)
is supplementary reusable guidance, not a prerequisite. One subtask is active
at a time. Between accepted subtasks, the latest open numeric task may retain
only its compact progress and has no active packet. Follow the local
requirement packet and closure audit defined here. Before implementation, the
coordinator creates the one active subtask packet
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

Use this entry table to select an existing path; it creates no additional
authority or exception:

| Situation | Permitted action | Required before execution or acceptance |
| --- | --- | --- |
| Discussion or read-only investigation | Explore, inspect, and ask questions. | No active packet or repository change; an owner request is required before change work is admitted. |
| Approved new implementation | Admit `New` `T<n> S1`. | Owner approval, the next numeric identifier, an active packet, and its named proposal. |
| Next step of the latest open implementation task | Admit `Continuation` `T<n> S<m>`. | Retained progress for that latest T, its next S identifier, and an active packet. |
| Defect in the latest closed implementation task | Admit its narrow `Corrective` S, or a new T when outside that scope. | The corrective boundary, current evidence, active packet, and normal closure proof. |
| Approved standalone governance or documentation | Admit `Td S<n>`. | Owner approval, the next Td identifier, an active packet, applicable gates, and actual-change review; it creates neither numeric T nor artifact baseline. |
| Completed admitted S | Deliver one or more reviewable P commits. | Coordinator actual-change review, applicable verification, evidence/transfer update, and acceptance before closure. |

During execution, the executor reports discovered repository issues. For a
clear in-scope instance, the coordinator either revises the active packet/brief
or admits a later subtask, then requires the similar-issue sweep; otherwise the
coordinator records the issue in `states/TODO.md` with priority, boundary,
admission condition, and risk. The executor never adds a subtask itself.
At closure, re-read the original owner request, map every requested outcome to
evidence, run the required verification, record the retrospective/prevention
action when applicable, and report the result. A task is not complete merely
because its implementation or tests look complete.

**Mechanism defects.** For a repeated construction, lifecycle, state-publication,
validation, or failure-atomicity defect, the packet and evidence identify the
shared owner, relevant variants and callers, and the validation-to-commit or
rollback boundary. The scope repairs that mechanism or explicitly transfers an
excluded variant through Queue or TODO; the architecture and coding rules own
the resulting production-path constraint.

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

Every executing S follows one coordinator/executor lifecycle with one active
`states/CURRENT.md` packet, the same identifier rules, evidence requirements,
similar-issue sweep, and closure audit. Between an accepted S and the next
admission, that packet is removed and the latest open numeric T retains its
compact progress row; no implementation may run in that interval. One session
may perform both roles in sequence, or two sessions may each perform one role;
session assignment never changes the lifecycle or authority.

**T, S, and P.**

`states/QUEUE.md` owns the ordered queue of T task packages. A T is a bounded,
owner-approved goal with a stated scope, non-goals, dependencies, completion
standard, and stop condition. It is not a pre-allocation of every implementation
step. The coordinator observes the evidence produced by each admitted S and
dynamically plans the next S within the active T boundary.
### Task-Scale Convergence And Coverage

Dynamic S planning is evidence-led adaptation, not permission to redefine a T as the sequence of its first observed failures. A **coverage-bearing T** is a task whose stated outcome uses complete, all, qualification, closure, audit, L3, or an equivalent whole-domain claim. Before its first implementation S, its proposal or an admitted planning S must define one durable task convergence ledger. The ledger is detailed evidence or task history, not a second current authority; the active packet links to the applicable ledger and names the batch it consumes.

The ledger records, at the level appropriate to the T, all of:

- the frozen coverage universe: named capabilities, forms, state transitions, devices, routes, or finite corpus/checkpoints to which the claim applies;
- the unit of coverage and required context, including any source, ownership, timing, lifecycle, reset, failure, or interoperability facts needed to make a disposition;
- each permitted disposition: accepted with direct proof, explicitly non-applicable with reason, retained non-eligible/deferred with its named receiver, or blocked by a stated authority/implementation prerequisite;
- the evidence and regression owner for an accepted disposition, and the admission path for every non-accepted one; and
- the T-level completion predicate: which ledger states must be exhausted or transferred before its claimed gate can open.

The universe may be refined only by evidence that changes the admitted domain; the coordinator records the change, its reason, and its effect on every affected disposition before more implementation proceeds. Refinement must not silently turn an unbounded runtime path into a moving target. Where execution or firmware is the consumer, the corpus uses named finite semantic checkpoints; a retirement, wall-clock, byte, or retry budget is containment only and never coverage success.

Every S inside a coverage-bearing T states which ledger batch it consumes and, at closure, records the before/after disposition of that complete batch. A successful replay that reaches farther, a new first terminal, or one repaired form is diagnostic evidence only. It cannot by itself select the next repair, advance a qualification gate, or support a complete/L3 claim. The coordinator first collects or reconciles the full affected batch, then either admits one batch repair with exact authority and regressions or transfers the whole underdetermined class to its earliest owner. A narrowly scoped safety fix may stop unsafe publication immediately, but it does not count toward coverage until it enters the ledger and receives the same batch disposition.

Before accepting an S or closing a coverage-bearing T, coordinator actual-diff review verifies that the implemented and observed surface is mapped to the ledger, that no unresolved member is hidden by a later checkpoint, and that every claimed gate has its stated completion predicate. The documentation gate checks structure only; it cannot prove convergence. A T that cannot establish this ledger is not eligible for a whole-domain conclusion and must remain an inventory, diagnostic, or bounded implementation task with its residual work transferred explicitly.

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

**Result simplicity, code-size and cleanup.** Prefer the simplest resulting
mechanism, not merely the fewest edited lines. A local patch is insufficient
when it leaves duplicate production paths, mirrored state, an obsolete owner,
or a reversed dependency that the admitted work can correctly retire. A
code-changing S records its counted tracked source/test paths, excluding
documentation and generated/artifact paths. At closure, report added, removed
and net lines with the method, identify the retained production path and state
owner, and explain any material positive increase. Non-positive net code is a
design goal, not an admission or closure gate: a broader cohesive replacement
is required when it produces the simpler correct result. Remove
obsolete/duplicate/unreachable code in the changed mechanism, or name each
live retained path, its distinct semantic reason, and its planned receiver in
the evidence.

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

Apply the preceding rules in order: coordinator admission under **Request
Lifecycle** and the S contract; executor confirmation or objection, completion
goal, reporting, self-review, and complete-P push; then coordinator
actual-change review and the applicable P-lifecycle outcome.
Coordinator review compares the original request, S brief, packet, applicable
rules, evidence, and actual Git/worktree changes against the exit criteria; an
executor report indexes evidence and cannot substitute for that review.
**Role authority**
and the one-session/two-session review boundary above govern who performs those
steps and whether the review is independent. One-session runs switch to the
coordinator role for that review; a separate-session coordinator review is
independent. A material objection pauses
affected work pending coordinator direction and cannot be overridden by a
request to continue. A mid-S handoff records the
accepted brief, current evidence, unresolved objections, and worktree state
before the receiving session resumes.
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

**Test closure.** A focused selection is transient: it is named only by the
active S packet, changes with that S, and is never a fixed manifest, source
list or retained task evidence. Every numbered S closes only after the complete
repository-only unit suite passes. Every numbered T closes only after that unit
suite and its owner-managed external-ROM/disk integration suite pass; an S may
include either relevant suite in its transient focused selection. Standalone
documentation work runs only its applicable governance checks.
Documentation/design tasks apply only the rules relevant to their artifact and
do not manufacture runtime verification requirements. Standalone `Td` work
does not create a task artifact or change the current artifact version.

## Documentation Governance Gate

Every task and standalone `Td` closure runs:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .
```

For diagnosis, run `-Scope Documentation` or `-Scope GovernanceState`
separately. The first checks topology, document schemas, Reading-Set navigation,
`etc/` indexing, links, mojibake, and machine-local paths; the second checks
packet fields and identifiers, the sole `CURRENT.md` technical baseline,
queue/debt boundaries, artifact identity, and capped Status summaries.
`current-gates-gcc` runs the combined check when PowerShell is available.
Failure blocks closure. Passing this gate proves structural consistency only;
it does not prove that Queue, history, baseline, evidence, and current runnable
source truthfully agree. Coordinator closure review must inspect the actual
changes and apply the [Documentation Rules](DOCUMENT.md) authority matrix and
its status-retention limits to every changed document.

## Milestone Closure Evidence

Before an implementation milestone closes, final evidence maps the current
source graph to roadmap exits, applicable rules, open TODO deferrals, and
current evidence. An earlier pass, clean compile, or fixture-only smoke cannot
close a changed runnable path. Supporting `etc/` checklists do not supersede
the applicable architecture, coding, source, or execution authority.

Every configured gate identifies build, execution, or static inspection.
A claimed runtime gate invokes its registered CTest cases and fails on nonzero
results; compiling a smoke executable is build coverage only. Source-shape and
inventory checks remain named static gates.

Each completed implementation task that changes a runnable path compiles,
verifies, and copies one usable local developer artifact to ignored
`build/output/`. Its four-digit revision is the numeric task identifier
(`T258` is `0.5.0258`): it is an identity, not sequencing, rule. All its
subtasks rebuild that revision; source commit plus SHA-256 identify the build.
Identifiers are never reused or allocated out of queue order.

The executable is `nxvm_0_5_NNNN.exe` for the bootable VM or
`nxvdm_0_5_NNNN.exe` for the DOS runner. Task records map identifier to
revision; historical artifacts retain their names and banners but are evidence,
never active CMake targets. The current-artifact target is the only admitted
product artifact target. Record SHA-256, source commit, identity/banner, and
baseline/developer/product kind. Smoke executables stay in the build tree;
design-only work creates none. Local artifacts are not release evidence, carry
no protected media or Microsoft binaries, and are replaced only by a newly
verified build of the same named task.

A current product artifact is a stripped Release build with no compiler debug information; this never removes an admitted runtime debugger, and Debug builds are for development and gates only.

## Build Tree Hygiene

After every build, test, smoke, sanitizer, or failed verification, remove owned
temporary products once the active or immediately next subtask no longer needs
them. Only `build/output/` preserves verified task executables; configuration
trees, objects, generated tests, logs, traces, sanitizer trees, and stale
CMake/Ninja state are disposable unless the active subtask records a need.
Before recursive cleanup, verify the resolved target is below `build/`, exclude
`build/output/`, and stop every owned process using it.

Runnable evidence records the emitted identity/banner and version. Changing
identity, version, or cutover state requires an approved subtask and regression
evidence. For a legacy coupled system, record a runnable full-source baseline
before subtractive refactoring; an isolated import gains neither behavior nor
weaker source, asset, licensing, or test rules. Differential debugging is a
bounded experiment: record both implementations, inputs, event schema,
checkpoints, masks, instruction/time/no-progress budgets, and cleanup owner; it
is neither a runtime dependency nor a substitute for focused owned tests.

## Recorder Trace Containment

Raw instruction recording is ignored and potentially unbounded. A subtask must
declare a unique ignored output path, wall-clock/no-progress/byte budgets,
process-tree cleanup owner, and retained checkpoints before launch. Byte budget
is hard: recorder stops before it, or the harness monitors growth and kills the
whole launched process tree on first excess; timeout alone is insufficient.

Use a fresh name beneath ignored `build/` or `artifacts/`, reserve twice the
byte budget before launch, and never reuse an earlier trace path. After any
outcome, wait for exit, verify its handle is closed, record final
size/checkpoint, and delete raw trace unless the approved subtask retains it
for immediate diagnosis. Block the next run while an owned process or trace
remains. A legacy recorder without an in-process cap is diagnostic-only through
this harness; its raw output is never fixture, baseline, release artifact, or
committed evidence.

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
