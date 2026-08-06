# M5 T248: Host-Capability Admission And Migration Queue

**Status:** Complete.

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

#### S1 Evidence And Decisions

| Candidate | Current implementation and real owner | Thread, copy, and shutdown evidence | NXVM and trusted-research need | Decision |
| --- | --- | --- | --- | --- |
| Normalized host input | `vm/platform/input.[ch]` defines keyboard/mouse callback transports; `vm/composition/session/lifecycle.c` binds them to session callbacks, which create copied `vm_platform_request` values and enqueue them in the session-owned ingress transport. | Win32/Linux adapters call the transport; the callback must not mutate the guest and currently only enqueues a scalar copy. `vm_platform_request_transport_close()` closes ingress before session storage is finalized. | NXVM needs one ordered host-to-session boundary for Console and window input. The trusted VDM research requires the same copied, non-guest-mutating boundary. | `ADMIT`: extract only neutral copied event value/sink vocabulary to `core/platform`. The queue, event ordering, mapper, and KBC/AUX delivery remain composition/VM policy. |
| Copied presentation frame | `core/platform/display_frame.h` already owns the copied frame value. `vm/platform/presentation_mailbox.[ch]` owns a VM-local lock and one copied latest-frame slot; `vm/composition/session/display.c` copies the core snapshot into it. | VM backends capture a copy from the mailbox; they do not borrow guest VRAM. The mailbox lives in `vm_session` and is finalized only after the run handle stops and joins. | NXVM needs identical Console/window consumption without renderer access to guest state. Trusted research requires a copied frame sink usable by a future mantle consumer. | `ADMIT`: promote only the policy-free snapshot/mailbox contract to `core/platform`; keep frame capture, graphics promotion, renderers, and display choice in VM/composition. |
| Host wait and cancellation | `core/platform/sleep.[ch]` supplies platform sleep. `core/utils/wait*` supplies a callback scope. VM uses it in execution-flip polling and debugger waits; runner/control currently call sleep directly. | The wait scope has no guest state, but it is not cancellable and has no explicit teardown/result contract. Guest `elapsed_ticks` are owned exclusively by `core_machine`; host waiting only paces host threads. | NXVM needs bounded start/stop waiting and debugger polling. Trusted research requires cancellable wait, while explicitly forbidding host-clock mutation of guest time. | `ADMIT` for a bounded cancellable wait contract. `DEFER` raw monotonic-clock observation: no current NXVM consumer needs it, and admitting it now risks conflating wall time with guest time. |

The inventory confirms that no candidate authorizes a `core -> vm` dependency,
host-thread guest mutation, raw guest pointer, filesystem/path policy, or a
second execution path. `vm_platform_run_context`, `vm_platform_run_handle`,
renderers, display-mode selection, and PC/AT input mapping remain VM-owned
because they contain NXVM product policy and platform-thread lifecycle.

### S2: Freeze The Migration Queue

For every `ADMIT` decision, define the smallest public contract and assign its
follow-on task below. For every `KEEP_VM` or `DEFER` decision, record why no
source move is authorized. Update the task queue before any migration begins.

#### S2 Frozen Migration Contracts

| Follow-on task | Frozen smallest contract | Owner after task | Explicitly remains outside the contract |
| --- | --- | --- | --- |
| T249 | `core_platform_input_event` value plus a copied source/sink call boundary. A composition-owned ingress queue accepts the value and consumes it only at the existing execution boundary. | `core/platform` owns the value vocabulary; mantle/VM composition owns queue, ordering, and consumer. | Keyboard layout, scan-code mapping, KBC/AUX, BIOS/BDA, host capture mode, Console/window policy. |
| T250 | `core_platform_presentation_mailbox` owns one synchronized copied `core_platform_display_frame` snapshot with initialize/publish/capture/finalize lifecycle. | `core/platform` owns the copy container; composition owns snapshot production and VM platform owns renderer consumption. | Guest VRAM, VADP capture, frame cadence, auto promotion, renderer/window/Console ownership. |
| T251 | `core_platform_wait` accepts a bounded interval and explicit cancellation predicate/result. It is host-thread only and cannot observe or advance guest time. | `core/platform` owns the neutral wait primitive; VM composition owns stop policy and calls it. | `elapsed_ticks`, CPU/PIT scheduling, pacing policy, watchdogs, host monotonic-clock API, or session exit policy. |
| T252 | One VM composition lifecycle sequence binds the admitted contracts: create -> configure -> freeze -> start -> request stop -> join -> finalize -> destroy. | VM composition owns this sequence and its run handle. | A second scheduler/session/machine, new product loop, or changes to NXVM Console/debugger/boot behavior. |

`T249`--`T252` are therefore implementation tasks, not optional labels. A task
may close without a source move only if its S1 proves the frozen contract has
no real consumer or would violate the stated boundary; it must record that
proof. The former unstarted hardware tasks start at `T253` and retain the
renumbering below.

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
| T251 | Cancellable host-wait boundary; raw monotonic-clock observation remains deferred. | Thread/teardown/no-guest-time-mutation probes and run-handle regression. Stop if the candidate changes product pacing, watchdog, or guest elapsed ticks. |
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

#### S3 Closure Evidence

- S1 records the current owner, copy rule, lifecycle boundary, NXVM consumer,
  and trusted-research requirement for every candidate.
- S2 gives each admitted capability exactly one bounded implementation task
  (`T249`--`T252`) and moves the unstarted hardware queue to `T253`--`T260`.
- The task records one deliberate deferral: a raw host monotonic-clock API has
  no current NXVM consumer and must not affect `core_machine.elapsed_ticks`.
- The frozen contracts prohibit a core-to-VM dependency, guest-memory borrow,
  host-thread guest mutation, product policy, and a second session/run loop.
- Static search of `src/core`, `src/vm`, `src/vdm`, and architecture/planning
  records confirms the admitted state is copied at every cross-thread edge;
  `git diff --check` passes. T248 changes no runtime source, build target, or
  artifact.

**Closure marker:** `M5:T248:S3:BOUNDARY-CLOSURE:OK`.

Task scheduling is intentionally not recorded here. See
[Project Status](status.md) for the current active state and the next
approval-required work; this completed T248 record does not authorize source
work on its own.
