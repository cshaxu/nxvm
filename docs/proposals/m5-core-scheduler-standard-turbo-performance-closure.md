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

## Audit Prerequisite

Earlier controller closures are inputs to this task, not proof that the current
scheduler integration is complete. Every selected controller is re-audited
against its source, current code and available external implementations before
the system-level path may consume it. A discovered cross-owner defect is fixed
in that controller's S batch; it is not deferred merely because an older
controller T had closed.

The resulting finite convergence matrix includes every deadline producer and
immediate-action owner. PIC is re-audited as an immediate-action owner: its
source and command mutations must publish cascade state immediately, and the
task either removes an obsolete scheduler cascade fallback or traces one real
remaining mutation source to that publication path.

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

The task re-audits PIT, RTC, DMA, FDC, HDC, KBC/XT keyboard, PIC publication,
VADP CGA/EGA and Model-40 D4 refresh. It freezes their reset, cancellation,
simultaneous-deadline and no-deadline behavior before scheduler composition.
It then repairs every discovered owner-local defect before it makes a global
scheduler change.

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

Every controller S first renders/quality-checks its applicable original manual
or board source, reviews its actual Core and consumer code, and cross-checks
86Box, MAME, PCjs, Bochs and QEMU where locally available. It records a finite
List 1/List 2 for its deadline or immediate-action surface. It then fixes the
complete discovered owner-local batch, including affected downstream devices,
or retains each unsupported relation with a truthful L1/L2 receiver.

1. **S3 — PIC 8259A.** Re-audit source/command/EOI/cascade publication and
   every scheduler refresh/blocker. Repair every real late or duplicate
   publication path; retain no fictional PIC delay.
2. **S4 — DMA 8237A.** Re-audit request, arbitration, phase, DMA clock,
   refresh, FDC DMA2 and Xebec DMA3 publication through the present scheduler.
3. **S5 — PIT 8253/8254.** Re-audit counter/output deadlines and IRQ0,
   refresh, speaker and board-consumer ordering.
4. **S6 — RTC/CMOS.** Re-audit periodic/update/alarm deadlines, IRQ8 and
   NMI/firmware-facing board effects.
5. **S7 — KBC 8042 and XT PPI keyboard.** Re-audit command, serial, typematic,
   A20/reset and IRQ1 deadlines or immediate transitions for both topologies.
6. **S8 — Intel 8272A FDC and media/DMA2 consumer.** Re-audit command, seek,
   byte gate, DRQ, TC, IRQ6, cancellation and reset without inventing rotation
   or mechanics timing.
7. **S9 — VADP CGA/EGA.** Re-audit that video advancement is either a sourced
   owner-local deadline or an explicitly non-wakeup/presentation relation;
   preserve VADP as the sole port/VRAM/snapshot owner.
8. **S10 — HDC personalities.** Re-check ATA, WD1003, Compaq/WD and Xebec
   phase, DRQ/IRQ/DMA or PIO consumers against the current scheduler.
9. **S11 — Model-40 D4 refresh-hold.** Re-audit the board-specific immediate,
   hold and reset boundary separately from generic DMA.
10. **S12 — Core scheduler composition.** Only after S3--S11 have accepted
    dispositions, freeze the complete owner matrix; remove eligible global
    per-tick scans and compose one earliest-deadline/same-tick order path.
11. **S13 — Standard/Turbo product and performance closure.** Verify the one
    Core path across the profile/CPU/media corpus, compare functional event
    observations and host cost, run full unit/integration, and publish the
    stripped Release artifact.

## Exit Criteria

All S3--S11 controller-owner rows have an implemented source-qualified deadline,
an explicit immediate action, or a named L1/L2 receiver, with no unresolved
cross-owner defect deferred to S10. No known eligible deadline is hidden behind
per-tick all-controller polling. Standard and Turbo have identical guest event
ordering, distinct only in wall-clock waiting. Complete repository-only unit
and owner-managed integration suites pass, and the performance corpus
demonstrates the retained Core path rather than a parallel fast path.
