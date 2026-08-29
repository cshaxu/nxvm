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

The admission is based on concrete prior failures, not speculative
refactoring: the XT could reach CPU wait while a device event path was not
published as progress; FDC result acknowledgement once retained IRQ6 beyond
its normal firmware-consumption boundary; DMA verify risked bypassing the DMA
mode owner; and Model-40 firmware reached a duplicate D4 ROM mapping instead
of immutable firmware during a legal protected-mode descriptor write. The
latter is not itself an IRQ route, but proves why the whole owner-to-consumer
route, rather than isolated register behavior, is the unit of audit. T501
repairs are baseline evidence; they do not pre-accept any other route.

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

S1/S2 are retained only as preliminary route inventory. They do not satisfy a
controller audit because they did not independently re-read each original
source and external implementation. Every following controller S executes the
same complete sequence for **one state owner and its downstream consumers**:
original manual and board pages, NXVM owner-to-consumer code trace, independent
86Box/MAME/PCjs/Bochs/QEMU comparison where available, a finite gap table, then
one owner-local repair batch and its tests. A passing adjacent controller or
boot checkpoint cannot close another controller's S.

1. **8259A PIC -> CPU.** Reconcile interrupt request, masking, cascade,
   INTA/vector, EOI and reset with the CPU consumer.
2. **8253/8254 PIT -> PIC/DMA.** Reconcile channel 0 IRQ0 and each selected
   channel-1 refresh consumer, including reset/gate/output and HLT wake-up.
3. **8237A DMA -> bound providers/memory.** Reconcile DREQ/DACK, request
   polarity, mode, verify, terminal count/EOP, cascade and reset for each
   selected FDC/HDC/refresh provider.
4. **8272A FDC -> DMA2/PIC/firmware.** Reconcile command, DRQ, TC, result,
   IRQ6 acknowledgement, reset, drive selection and no-media/error paths.
5. **HDC -> DMA/PIC/firmware.** Reconcile ATA, Compaq, WD1003 and Xebec
   personalities independently through their selected DRQ/DMA/IRQ/status and
   reset/error consumers; no ATA result stands in for another personality.
6. **MC146818A RTC/CMOS -> PIC/NMI/firmware.** Reconcile periodic/alarm/update
   state, IRQ8 acknowledge, port-70 NMI masking, reset and CMOS observation.
7. **8042/XT keyboard -> PIC/A20/reset.** Reconcile the selected KBC or XT PPI
   keyboard owner, FIFO/serial completion, IRQ1, A20, reset and NMI boundary.
8. **VADP -> copied display consumer.** Reconcile guest port/memory ingress,
   CGA/EGA state, output disable/text fallback and copied snapshot publication.
9. **Cross-profile closure.** Run the completed controller ledger against
   5160, 5170, Model-40 and default-AT, execute complete unit and integration
   gates, and transfer any newly proven controller-internal gap to its earliest
   unit T.

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
