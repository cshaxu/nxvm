# Core Event-Deadline Scheduler Convergence

## Objective

Replace Core's recurring per-tick arbitration, readiness and peripheral
maintenance route with one Core-owned ordered event-deadline progression.  The
result must migrate every current scheduler client, not merely accelerate the
DeskPro 386 Model 40 replay.  A controller either publishes its next actual
guest-state change through the shared Core mechanism, is idle and unscheduled,
or explicitly blocks safe fast advance; there is no retained implicit
`due_tick + 1` polling path.

This is a scheduler-integration task, not a claim that every controller's full
hardware behavior or physical timing is now Manual-L3. Manual formulas,
documented state relations, and verified external board inputs retain their
existing L3 provenance. When an external emulator implements the same
manual-defined relation and project probes agree, that row remains
Manual-L3 with external cross-validation; the reference implementation does
not lower it. Where a manual lacks a state-duration or transition relation, a
reproducible, read-only external-emulator model may instead provide a labelled
External-L2 reference-derived relation after project-owned probes confirm the
selected observable. A source gap with neither basis remains an L1 causal
blocker, never an invented deadline.

## Evidence And Problem

T498's bounded Model-40 BYOB replay reaches neither a CPU exception nor an FDC
command while the ROM memory diagnostic advances.  The present Core route
reschedules all three maintenance callbacks at each successive tick, so even
idle controllers are serviced for every elapsed guest tick.  The timeline
heap candidate may improve earliest-event selection, but it cannot remove the
three recurring callbacks and is not a resolution of this problem.

The authoritative controller and timing ledgers, their original manuals, and
their retained cross-checks against 86Box, MAME, PCjs, Bochs and QEMU are the
source basis for each selected deadline or causal boundary. External emulator
logic that agrees with the applicable original manual is retained as
cross-validation of the same Manual-L3 row. It never overrides a conflicting
manual or substitutes for one that is absent. In the latter case it may supply
an explicitly retained External-L2 relation only when its selected state model,
board applicability and project-owned probe agree; otherwise it remains
corroboration or a rejected research lead. No external source is copied,
transliterated, linked into the build, or made a runtime dependency.

## Reference-Design Findings

The read-only reference sweep supports the direction of this task without
authorizing any reference implementation as NXVM architecture:

- 86Box arms device-local timers against one CPU-progressed time source and
  processes the earliest armed timer. Its PIT edge callbacks also demonstrate
  that a board relation is a signal connection, not a profile-side polling
  loop.
- Bochs stores each active timer's absolute `timeToFire`, keeps one countdown
  to the earliest event, and processes every same-time callback before choosing
  the next countdown. This is the closest useful scheduling pattern for Core's
  ordered delta settlement, though its configured IPS conversion is not NXVM
  physical-time evidence.
- MAME also maintains one machine scheduler with absolute timer expiration and
  resynchronizes execution when a newly armed timer becomes earlier. Its
  general scheduler framework is deliberately not imported.
- QEMU distinguishes guest virtual time from host/realtime clocks and can
  advance a sleeping virtual CPU to the next timer. NXVM consumes only the
  guest-clock separation and Core-owned advance principle; it must not adopt a
  host-clock guest-time writer.
- PCjs computes timer state lazily from CPU cycles and caps a CPU burst at a
  device deadline. This supports delta advancement, but its source explicitly
  treats some PIT behavior as only BIOS-compatible. A PCjs path that matches
  the applicable manual can cross-validate Manual-L3; its unsupported or
  merely BIOS-compatible behavior can justify at most External-L2 after a
  project probe.

The selected NXVM mechanism is intentionally smaller: a private Core seam over
existing controller state, one ordered advancement route, and no imported
global timer framework or controller-to-host dependency.

## One-Owner Design

```
successful CPU retirement / Core-selected pause advance
                    |
                    v
       Core's sole elapsed-time publisher
                    |
                    v
    private earliest-event composition and ordered settlement
                    |
                    v
DMA/PIT/PIC -> FDC/HDC/RTC -> KBC/VADP -> copied observation
```

Core remains the only mutable owner of elapsed time, the timeline, controller
state and signal order.  Controller state remains local to its controller; a
controller does not obtain a private scheduler or receive VM/profile-written
ticks.  Profile construction may still provide frozen, validated board values
only through the existing Core plan.  VM observes copied Core progress and may
pace it, but never selects a device deadline or writes guest time.

The implementation adds only private, value-based next-change queries where a
controller needs one.  It does not add live timing setters, controller
pointers, a general event framework, a second time axis, host-clock injection,
media/firmware shortcuts, or profile-specific fast paths.

## Required Complete Client Inventory

Every current client of the recurring scheduler route is in scope:

| Owner group | Required disposition |
| --- | --- |
| DMA, PIT, PIC, transaction arbitration, XT refresh/PPI and D4 refresh hold | Exact next change from the selected board/manual contract remains Manual-L3 when externally cross-validated; otherwise a labelled External-L2 relation validated by a project probe, or an explicit causal blocker while active. |
| FDC, HDC and RTC readiness | Existing command, seek, reset, byte, IRQ and RTC transitions become next-change candidates when their state proves one; a matching external model cross-validates Manual-L3, while a manual gap may use labelled External-L2; idle state is unscheduled; an otherwise unproved active HDC phase blocks rather than receives a guessed duration. |
| KBC/XT keyboard, VADP and execution provider | Existing response, serial, typematic, raster and provider transitions advance by a proven Manual-L3 relation (optionally externally cross-validated) or External-L2 delta, or expose an explicit blocker. |
| Timeline/reset/cancel/pause/debugger routes | One Core settlement route preserves equal-tick order and invalidates/cancels state exactly once. |

No owner may remain in an unconditional global every-tick callback after this
task.  A controller with no independent temporal state is not scheduled; a
controller with active but unknown timing remains visible as a blocker rather
than a hidden polling consumer.

## Subtasks

1. **S1 - complete scheduler List 1/List 2 and source reconciliation.**
   Inventory every current recurring callback caller and all of its state
   transitions; reconcile each timing relation with the controller's original
   source ledger and retained external-emulator cross-check.  Classify each as
   Manual-L3 (including external cross-validation), External-L2, explicit
   internal L2, L1 causal blocker
   or idle/no-time state.  Record the reference implementation and a
   project-owned probe for every External-L2 row; do not derive code from it.
2. **S2 - one private Core advancement seam.** Introduce the smallest
   Core-private earliest-event composition and delta-settlement seam.  Preserve
   the sole elapsed-time writer, due-time ordering, reset/cancel semantics and
   copied observation boundary; remove no controller behavior yet without its
   S1 disposition.
3. **S3 - arbitration-owner migration.** Migrate DMA, PIT, PIC, transaction
   arbitration, XT refresh/PPI and D4 refresh-hold progression.  Preserve DMA
   grant, BUSRDY, refresh and same-tick ordering; active unqualified work must
   block, not be estimated.
4. **S4 - readiness-owner migration.** Migrate FDC, HDC and RTC command,
   completion, IRQ, media and clock progression.  Reuse FDC's existing absolute
   due facts; keep an active HDC phase non-fast-forwardable unless its sole
   owner supplies a manual/board deadline or a justified External-L2 relation.
5. **S5 - peripheral-owner migration.** Migrate KBC/XT keyboard, VADP and the
   execution-provider path.  Preserve typed input/response ordering and the
   copied display boundary; no renderer or VM state becomes a time owner.
6. **S6 - remove the periodic route and prove lifecycle equivalence.** Delete
   all production `due_tick + 1` maintenance rescheduling, prove reset,
   cancellation, pause, HLT, debugger and equal-due ordering, and use bounded
   order/trace fixtures without retaining protected firmware or guest traces.
7. **S7 - whole-matrix closure.** Build the required stripped Release artifact
   and replay every supported IBM 5160, IBM 5170 Model 339, DeskPro 386 Model
   40 and `default-at` DOS-media row to its semantic terminal or its documented
   external-input boundary.  Model 40 must pass its prior ROM-diagnostic
   predecessor and enter the FDD route; a timeout or instruction count is not
   success.

## Acceptance And Stop Conditions

Closure requires a complete inventory showing no remaining unconditional
per-tick production scheduler client; focused ordering and lifecycle
regressions; documented manual, board, or External-L2 provenance for every
eligible deadline; explicit active blockers for every ineligible relation; and fresh Release
matrix evidence.  The developer artifact is the task-numbered stripped Release
`nxvm_0_5_NNNN.exe` with the runtime debugger retained.

Stop and transfer, rather than estimate, if a controller requires an unknown
physical service duration with no reproducible external-model relation, a new
board input, protected firmware behavior, or a profile/VM-owned change.  Do
not claim wall-clock physical pacing from this task; profile physical-timebase
closure remains separate.
