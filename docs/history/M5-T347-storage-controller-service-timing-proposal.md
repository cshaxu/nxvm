# M5 PC/AT Storage Controller Service Timing

## Purpose

Replace the current synchronous command-side effects of the PC/AT FDC and ATA
PIO owners with explicit, deterministic service-state machines.  This is the
first post-T346 device-completeness task: it consumes the existing single core
timeline and its fixed equal-tick order; it must not create a second scheduler,
invent host-time delays, or make a Windows compatibility claim.

## Scope

The task first establishes one mechanism-level lifecycle inventory for both
controllers: command acceptance, validation, pending/busy observation,
scheduled service, media result, DRQ/DMA/IRQ publication, status/data access,
reset, media change, cancellation, and failure atomicity.  It then admits
bounded S units dynamically.  The expected order is a shared lifecycle and
timeline integration review, FDC migration, ATA migration, and a final
cross-controller reconciliation; those names do not pre-allocate S
identifiers or permit an incomplete unit to close.

The FDC work is constrained to the existing uPD765/8272A-compatible command
surface and DMA2/IRQ6 topology.  ATA work is constrained to the existing
primary PIO CHS/LBA28 `READ SECTORS`, `WRITE SECTORS`, `IDENTIFY DEVICE`,
IRQ14/nIEN/SRST surface.  Each adopted command must have a deterministic
acceptance-to-completion transition and reset/abort/media-result handling.

## Non-goals

Do not add host blocking I/O, a generic storage framework, LBA48, ATAPI,
bus-master IDE, cache policy, guest media, Windows setup execution, generic
DMA redesign, cycle-exact rotational mechanics, or a second controller clock.
Do not broaden FDC/ATA command sets merely to claim manual coverage.

## Completion Standard

T347 closes only when every command in the retained FDC and ATA surfaces is
classified once as an implemented state transition or an exact deferred
boundary; command issue cannot synchronously publish its completion-side
DRQ/DMA/IRQ.  The evidence must prove deterministic timeline ordering,
pending/busy visibility, data transfer, completion, reset/abort/media change,
and failure atomicity for both controllers.  It also records the receiver for
every excluded hardware feature.

## Dependencies And References

T346 is the baseline: its timeline owns due events and the ordering
`DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP`.  The primary hardware
contracts are the NEC uPD765/Intel 8272A-compatible command protocol and the
ATA PIO register/status contract.  The existing T346 S4 evidence identifies
the synchronous paths this task must replace.

