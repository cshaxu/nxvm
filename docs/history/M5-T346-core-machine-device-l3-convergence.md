# M5 T346: Core-Machine Device Parity And L3 Timing Convergence

## Task Record

T346 turns the completed CPU-profile program into a hardware composition
program.  It inventories and then closes, or precisely transfers, the
core-machine device, bus, port, memory-map, interrupt, DMA, and deterministic
timing gaps relevant to a PC/AT-class Windows 3.x research machine.  It
excludes numerical x87 execution.

## Closure

T346 closes its selected deterministic PC/AT device/L3 program. Its detailed
ledger reconciliation, Windows handoff, artifact provenance, and retained
timing limits are indexed at
`docs/etc/evidence/t346-s6-l3-closure-windows-handoff.md`. No later device or
timing task may bypass that ledger or make a Bochs/PCjs implementation a
project dependency.

## Completed Subtasks

### S1 - Whole-Machine Capability, Reference, And L3 Audit

Accepted the indexed whole-machine ledger at
`docs/etc/evidence/t346-s1-core-machine-device-l3-audit.md`. It measured the
current deterministic, host-clock-independent L2 scheduler and identified the
required L3 foundation: one due-event/arbitration owner plus bounded memory,
I/O, and DMA transaction visibility. It allocated S2 before all controller
expansion, S3/S4 as dependent device migrations, and S5 as the reconciliation
handoff to the Windows readiness map. It refreshed stale capability wording,
preserved unselected device debt in TODO, and excluded x87 execution and source
import.

### S2 - Deterministic Time, Event, And Bus Foundation

Accepted the implementation and evidence at
`docs/etc/evidence/t346-s2-deterministic-timeline-foundation.md`. The core
machine now owns one allocation-free deterministic due-event timeline ordered
by `(due_tick, sequence)`, with cancellation and cold-reset removal. Copied
trace checkpoints cover CPU retirement, retained DMA advancement, checked
external memory transactions, and existing port transactions at machine time.
S2 deliberately retains the L2 controller scheduler and assigns every
controller migration to S3--S5; it does not claim device-specific L3 service
timing or host-time behavior.

### S3 - PIC, PIT, And DMA Arbitration

Accepted the implementation and evidence at
`docs/etc/evidence/t346-s3-pic-pit-dma-arbitration.md`. The machine now
advances the PIC/PIT/DMA chain only through a one-tick timeline callback in
the stable order DMA, PIT, PIC. Copied trace records distinguish the
post-retirement elapsed observation from the full 64-bit due-event time. Cold
reset cancels the prior callback and schedules exactly one new tick-one
callback. RTC/NMI, storage, input, display, and host presentation retain their
separate S4/S5 assignments; no service duration or sub-instruction claim is
made.

### S4 - RTC And Storage-Observation Readiness

Accepted the implementation and evidence at
`docs/etc/evidence/t346-s4-rtc-storage-readiness.md`. The core machine now
uses a distinct RTC clock domain and a reset-safe timeline readiness callback
after the S3 arbitration chain. At each due tick it deterministically performs
FDC media observation, retained ATA refresh, and RTC advance; copied trace
events expose this sequence. The controller sweep establishes that FDC final
commands and ATA PIO service are still synchronous, so their pending/busy,
media-result, DRQ/DMA/IRQ, abort/reset, and hardware-duration contract is a
single explicit TODO rather than a fabricated L3 duration. PC/AT parity and
I/O-channel NMI production likewise remains a separately admitted boundary;
the retained CMOS index-port bit is only the NMI mask.

### S5 - Input, Display, And Peripheral Timing

Accepted the implementation and evidence at
`docs/etc/evidence/t346-s5-input-display-timeline.md`. KBC/AUX guest response
and typematic state, followed by VADP guest raster state, now advance only from
the reset-safe timeline peripheral callback. The full equal-tick machine order
is `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP`. The sweep confirms
that the VM-session/platform display crossing is a copied snapshot and mailbox
boundary, and that no host presentation or input cadence supplies guest time.
Speaker/PPI, advanced AUX behavior, broader video modes, and host policy retain
their separately recorded boundaries.

### S6 - PC/AT L3 Closure And Windows Handoff

The closure reconciliation is retained at
`docs/etc/evidence/t346-s6-l3-closure-windows-handoff.md`. It maps every S1
device/bus/timing family either to accepted S2--S5 evidence or to one exact
later Queue/TODO admission. T346 therefore closes the selected deterministic
PC/AT L3 event-and-bus contract: the timeline is ordered by `(due_tick,
sequence)` and equal ticks process DMA, PIT, PIC, FDC observation, ATA refresh,
RTC, KBC/AUX, and VADP in that order. This is not a cycle-exact, controller
service-duration, or Windows-compatibility claim. The Windows readiness
candidate receives only the evidenced blockers, and no Windows corpus or media
was used.

The task-level developer artifact is `nxvm_0_5_0346.exe`, built from
`fda545e18b018a1c1bfb72c86beccd434b9b8089` with SHA-256
`7715C8C290969A99CCC1137D2DEEC64421FF245A22BD35287A2AED0C75A8E260`.
Its banner is `Neko's x86 Virtual Machine [0.5.0346]`. Fresh configuration,
the retained S2--S5 focused proofs, documentation governance, diff check, and
the direct parallel 222-test current gate passed. The aggregate build wrapper
timed out without streamed failure output; the successful direct CTest result
is the recorded runtime gate.
