# M5 Core Scheduler Standard/Turbo Performance Closure

## Purpose

After the ordered controller deadline receivers have closed, consolidate their
source-qualified results into one efficient Core scheduling path.  This task
implements the event-driven performance consequences of the existing Core time
ownership: known guest events must advance through the earliest deadline rather
than cause per-source-tick polling of every controller.

This is an implementation and performance-convergence task, not a benchmark-only
audit.  It absorbs proven design lessons from 86Box, MAME, PCjs, Bochs and QEMU
only after reconciling them with original hardware sources and NXVM's ownership
rules; no external source code, compatibility shortcut, or second scheduler is
imported.

## Required Outcome

- Core remains the only guest-time writer and the only deadline composer.
- Each controller supplies only its own next observable state change through
  its established owner; it does not sleep, publish host time, or advance a
  private guest clock.
- Core selects the earliest valid deadline, advances once to it, and processes
  only the owners due at that point in the established same-tick order.
- Known sparse waits no longer execute every controller's maintenance path once
  per guest tick.  Unknown or lower-tier relations remain labelled L1/L2 and
  use the existing bounded policy rather than an invented duration.
- Standard and Turbo consume the same Core progression and event order.
  Standard may wait only when completed guest progress is ahead of its approved
  host pacing budget; Turbo removes that wall-clock wait and nothing else.

## Scope

The task consumes the completed deadline contracts for PIT, RTC, DMA, FDC, HDC,
KBC/XT keyboard, PIC publication and Model-40 D4 refresh.  It freezes the
eligible owner set and their reset, cancellation, simultaneous-deadline and
no-deadline behavior before implementation.  It then repairs the scheduler and
the affected owners as one cohesive batch where they still retain per-tick work
despite a known deadline.

The performance corpus covers the supported profile/CPU/media combinations,
HLT wait paths, DOS boot/installer checkpoints and selected hard-disk I/O.  It
records functional terminals, event order and host-cost measurements before and
after each admitted batch; a workload reaching a terminal alone is not proof of
deadline coverage.

## Non-goals

- No dynamic recompilation, CPU execution-engine replacement, or host-generated
  guest ticks.
- No claim that every controller has physical wall-clock timing.
- No device-local timer queues, VM/profile scheduler, duplicate device state,
  or broad event-framework layer.
- No copying external emulator code or adopting an external compatibility hack
  without a source-backed, owner-local NXVM contract.

## Required Sequence

1. Freeze a finite owner/deadline matrix from the completed controller ledgers;
   cross-check the event-driven architecture of available external emulators and
   record every accepted and rejected lesson.
2. Map every currently per-tick scheduler callback to a deadline, an immediate
   same-tick action, or an explicit L1/L2 receiver.  Resolve duplicated scans
   at the current state owner.
3. Implement the complete eligible batch through the sole Core scheduler and
   owner-local deadline queries.  Preserve reset, withdrawal, cancellation and
   same-deadline order.
4. Make Standard and Turbo consume that one path, then run comparative
   profile/CPU/media integration and host-cost measurements.
5. Reconcile the full frozen matrix, retain every lower-tier boundary, and
   publish a stripped Release artifact with the task revision.

## Exit Criteria

All frozen controller-owner rows have an implemented source-qualified deadline,
an explicit immediate action, or a named L1/L2 receiver.  No known eligible
deadline is hidden behind per-tick all-controller polling.  Standard and Turbo
have identical guest event ordering, distinct only in wall-clock waiting.
Complete repository-only unit and owner-managed integration suites pass, and
the performance corpus demonstrates the retained Core path rather than a
parallel fast path.
