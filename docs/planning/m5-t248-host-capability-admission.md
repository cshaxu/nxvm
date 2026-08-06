# M5 T248: Host-Capability Admission And Migration Queue

**Status:** S1 active.

## Goal

Turn the core/platform versus VM/platform boundary into an executable,
evidence-backed migration queue. T248 does not make a provider a default
dependency merely because an external report mentions it. It determines the
present owner, thread/lifetime contract, NXVM need, and trusted-research
requirement for each candidate, then reserves a bounded follow-on task for each
approved migration.

## Scope And Non-Goals

Candidates are copied normalized input, copied presentation sinks, monotonic
host-clock observation, and cancellable wait. Filesystem, drive/path policy,
window/Console policy, media attachment, host pacing policy, and guest mutation
remain outside core/platform.

T248 does not move PC/AT controllers, BIOS/firmware, CMOS/RTC, or storage
controllers. It creates no second session, event loop, host queue, or product
runtime path.

## Subtasks

### S1: Evidence Inventory

Map each candidate's current implementation, mutable state owner, creator,
destroyer, callback thread, copy rule, shutdown boundary, concrete NXVM need,
and trusted-research requirement. Record one of `ADMIT`, `KEEP_VM`, or `DEFER`
for each candidate. Stop if a candidate needs path policy, direct guest
mutation, or product exit/display policy.

### S2: Freeze The Migration Queue

For every `ADMIT` decision, define the smallest public contract and assign its
follow-on task below. For every `KEEP_VM` or `DEFER` decision, record why no
source move is authorized. Update the task queue before any migration begins.

### S3: Close The Admission Boundary

Verify that the resulting queue has one owner and regression strategy per
candidate, that no core contract exposes external ABI or product policy, and
that no later task starts before its S1 contract. T248 produces no artifact
unless a separately approved runtime change is made within T248.

## Reserved Follow-On Tasks

| Task | Scope if T248 admits it | Required regression and stop condition |
| --- | --- | --- |
| T249 | Copied normalized input source and composition ingress boundary. | Source-stop/thread/ownership probes, retained keyboard/mouse/Console regression. Stop if an event requires direct guest mutation or VM UX policy. |
| T250 | Copied presentation sink/frame boundary. | Copy/lifetime/teardown probes and retained text/CGA/EGA presentation regression. Stop if the contract needs guest VRAM, renderer ownership, or display policy. |
| T251 | Host monotonic-clock observation and cancellable-wait boundary. | Thread/teardown/no-guest-time-mutation probes and run-handle regression. Stop if the candidate changes product pacing, watchdog, or guest elapsed ticks. |
| T252 | VM composition lifecycle closure after admitted platform moves. | One create/configure/freeze/run/stop/destroy sequence; provider order, tick-delta, run-handle, Console/debugger, FDD/HDD regressions. Stop on a second scheduler, session, or machine path. |

Reserved tasks are conditional: a `KEEP_VM` or `DEFER` result closes its slot
without source movement or artifact and does not repurpose that identifier.

## Renumbered Hardware Queue

The former unstarted T249--T256 tasks follow the reserved boundary work:
T253 ATA/IDE, T254 digital CGA, T255 profile admission, T256 timing fidelity,
T257 protected mode, T258 paging, T259 task switching, and T260 present FPU.
Completed history and developer-artifact revisions are immutable.

## Applicable Rules And Evidence

Apply module layout, contracts, coding standard, source policy, execution
policy, and the M5 closure checklist. Any behavior-changing follow-on task
must use its own S1 contract, focused regression, current GCC/CTest gate, and
task artifact. T248 closure records the inventory, decision table, task queue,
static dependency review, and `git diff --check`.
