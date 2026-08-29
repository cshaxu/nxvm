# M5 Controller Signal-Chain Convergence

## Purpose

Audit and repair every selected controller path that is locally implemented
but can fail to reach its owning consumer.  The task covers the complete
causal route, rather than isolated port-register smoke tests: device state,
board wiring, DMA/PIC delivery, Core progression/deadline publication, CPU
wait or firmware observation, reset, and failure completion.

It follows the active Model-40 repair because that work supplies a concrete
external-ROM consumer.  It precedes the Turbo L1 compatibility escape: a
missing signal chain must be repaired at its owner, never hidden by generic
time progression.

## Frozen coverage universe

The ledger records one disposition for each of these routes and their normal,
masked, reset, absent-media/device, terminal/error, and HLT/wait states:

- PIT0 -> PIC IRQ0 -> CPU; PIT1 -> DMA0 refresh where selected by board;
- DMA service (including verify, terminal count, EOP, mask and cascade) ->
  FDC or HDC owner;
- FDC command/result/IRQ6 -> firmware, including drive-select and media
  change; HDC DRQ/IRQ/media completion for each selected personality;
- KBC command/FIFO/IRQ1/A20/reset -> Core CPU/memory state;
- RTC/CMOS/NMI/IRQ8 -> firmware-visible configuration and interrupt state;
- VADP port/memory writes -> its sole state -> copied display snapshot; and
- every selected controller's earliest observable deadline -> the one Core
  progression route while the CPU waits.

The ledger distinguishes a proven no-route condition from an unsupported
controller capability.  A controller-internal function already owned by a
separate hardware T is transferred to that earliest owner; this task repairs
only the connection, duplicated connection, or missing observation boundary.

## Evidence and design constraints

For every route, first reconcile the relevant original manual/board source,
existing List 1/List 2 row, and at least one independently inspected emulator
when primary material leaves the connection ambiguous.  External emulator code
is behavioral corroboration only; it is never imported or made a dependency.

Core remains the only owner of controller state, IRQ/DMA signaling, guest time
and event ordering.  Profiles contribute only immutable board wiring and
source-qualified values at construction.  Firmware/VM observe normal bounded
Core interfaces; they receive neither controller pointers nor a workaround
mailbox.  A repair must delete or consolidate a duplicate/implicit path where
one exists, not add a profile-side shim, polling loop, synthetic ready result,
or parallel scheduler.

## S decomposition

1. **Ledger and source reconciliation.** Freeze the route/state matrix;
   classify every row as direct proof, explicit unsupported/deferred receiver,
   or connection defect with its source basis.
2. **Clock, interrupt and refresh routes.** Repair any PIT/PIC/CPU and
   PIT/DMA refresh connection defects through the existing Core owners.
3. **DMA/controller service routes.** Repair FDC/HDC DMA, DRQ, terminal/error
   and IRQ lifecycle defects as one owner-local batch; include verify and
   no-media paths.
4. **Board/firmware observation routes.** Repair KBC, RTC/CMOS/NMI and VADP
   owner-to-consumer defects, retaining one state owner and copied display
   output.
5. **Cross-profile closure.** Run the frozen controller-route matrix against
   the selected 5160, 5170, Model-40 and default-AT profiles; execute the
   complete unit and integration gates and transfer any newly proven
   controller-internal gap to its earliest unit T.

## Exit criteria

- Every frozen route/state row has direct evidence, a repaired single owner
  path with regression coverage, or a named earliest receiver.
- No selected CPU HLT/wait state is falsely terminal while a connected
  controller has an observable pending state change.
- DMA verify, terminal/error and IRQ acknowledgement cannot bypass the device
  owner.
- No profile/firmware/VM compatibility shim, second controller state, or
  second time path is introduced.
- Complete unit and integration gates pass, with external-ROM evidence used
  only through the approved BYOB boundary.
