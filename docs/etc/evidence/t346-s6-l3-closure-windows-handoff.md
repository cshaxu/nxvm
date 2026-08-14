# M5 T346 S6: Deterministic PC/AT L3 Closure And Windows Handoff

## Achieved Contract

T346 closes the selected PC/AT L3 foundation. `core_machine_timeline` is the
sole core-owned, allocation-free due-event queue, ordered by
`(due_tick, sequence)` and reset by cold reset. CPU retirement advances it to
the authoritative elapsed tick. At an equal due tick, the three reset-safe
callbacks provide this complete selected order:

```text
DMA -> PIT -> PIC -> FDC media observation -> ATA refresh -> RTC
    -> KBC/AUX -> VADP
```

The trace surface records copied CPU, timeline, port, and external-memory
checkpoints. VM session and platform presentation consume copied display
snapshots/mailboxes; host time, threads, polling, and render cadence do not
advance guest device state. This is deterministic L3 event-and-bus ordering.
It is not a claim of cycle-exact CPU execution, prefetch/bus phases, analog
display, generic wait-state timing, or controller service duration.

## S1 Ledger Reconciliation

| S1 family | Final disposition | Evidence or receiver |
| --- | --- | --- |
| Machine time, ratios, reset | Accepted due-event owner and reset replacement. | [S2](t346-s2-deterministic-timeline-foundation.md) |
| Physical memory, A20, ROM, ports | Accepted copied external-memory/port checkpoints; internal wait states remain outside L3. | [S2](t346-s2-deterministic-timeline-foundation.md); `TODO(High)` bus-timed PC/AT operation |
| PIC, PIT, DMA | Accepted deterministic arbitration order and one-grant pacing boundary. | [S3](t346-s3-pic-pit-dma-arbitration.md) |
| RTC/CMOS | Accepted independent RTC clock and timeline advancement; NMI producers are transferred. | [S4](t346-s4-rtc-storage-readiness.md); `TODO(Medium)` PC/AT NMI source ownership |
| FDC/ATA | Accepted media observation/readiness ordering only; command-service state timing is transferred. | [S4](t346-s4-rtc-storage-readiness.md); `TODO(High)` FDC/ATA command-service timing |
| KBC/AUX and VADP | Accepted one peripheral callback and copied host boundary. | [S5](t346-s5-input-display-timeline.md); advanced AUX/video breadth TODOs |
| Firmware/BDA | Retained consumer of the deterministic machine boundary; generated-ROM materialization remains independent. | `TODO(Medium)` default PC/AT generated-ROM materialization |
| Host input/presentation | Accepted copied boundary, intentionally outside guest time. | [S5](t346-s5-input-display-timeline.md) |
| Speaker/PPI, serial, parallel, game | Not selected by a corpus; each remains bounded debt rather than inferred parity. | `TODO(Medium/Low)` hardware-and-compatibility entries |
| Broad CGA/EGA/VGA/VBE/composite | Bounded digital modes retained; unproven breadth is transferred. | `TODO(Medium/Low)` digital-CGA and composite entries; Windows map |

No S1 family is left implicitly partial: each has either accepted evidence or
one named future admission path. Numerical x87 execution remains excluded.

## Windows Research Handoff

The Windows readiness candidate now consumes this evidence through
`docs/history/M5-T355-windows-3x-readiness-map-proposal.md`. No Windows binary, setup,
driver, or startup corpus was used by T346. Therefore the handoff identifies
prerequisites only: storage service timing, NMI sources, corpus-selected input
and display breadth, and legally handled installation media/checkpoints.

## Verification Record

The closure reruns the S2--S5 focused markers, their exact CTest registrations,
fresh configuration, documentation governance, diff check, and the complete
current gate. Source commit `fda545e18b018a1c1bfb72c86beccd434b9b8089`
builds and copies `build/output/nxvm_0_5_0346.exe`, SHA-256
`7715C8C290969A99CCC1137D2DEEC64421FF245A22BD35287A2AED0C75A8E260`.
Its startup banner identifies `Neko's x86 Virtual Machine [0.5.0346]`.
The direct parallel current-gate run passed 222/222 tests. The aggregate build
wrapper exceeded its non-streaming command timeout, so the recorded gate is
the direct CTest result, not a claim that the wrapper completed.

The retained DMA/RTC authority smoke was also reconciled with this contract:
it now retires a NOP and then HLT across two due ticks. The first RTC update
asserts IRQ8 after that tick's PIC refresh; the second due tick's PIC refresh
observes it. This replaces its stale same-batch pending-IRQ assumption without
changing RTC/PIC production behavior.
