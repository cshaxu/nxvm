# T347 S4 Storage Service Reconciliation

## Retained lifecycle matrix

| Controller | Pending state | Producer boundary | Readiness-service publication | Cancellation before service |
| --- | --- | --- | --- | --- |
| FDC | `PENDING_COMMAND` | final command byte | decode, transfer/result state, then refresh/IRQ | DOR reset, controller reset, drive/media removal, and terminal command paths clear the pending state |
| FDC | `PENDING_COMPLETE` | final DMA/non-DMA transfer byte | result publication, then refresh/IRQ | DOR reset, controller reset, drive/media removal, and result acknowledgement prevent stale completion |
| ATA | `PENDING_COMMAND` | command-register write captures the complete task-file snapshot | command result/data phase, then refresh/IRQ | SRST, nIEN/control transitions, and media/reset paths discard unserved work |
| ATA | `PENDING_READ_SECTOR` / `PENDING_WRITE_SECTOR` | final data word of one sector | next DRQ sector or terminal status/IRQ | reset/control/media paths discard the pending sector before it can publish |

The service owner is exclusively `core_machine_readiness_tick`; controller code
does not directly call another controller's advance or refresh path. Service
is ordered `FDC advance -> FDC refresh -> ATA advance -> ATA refresh -> RTC
advance`. Arbitration has already run earlier in the same due tick, so a
service-side IRQ/DRQ publication is observable by later arbitration, never
retroactively within that arbitration pass.

## Executable proof

`core-machine-rtc-storage-s4-smoke` records two due ticks and requires the
five-event chain above on each one. The trace events bracket the actual FDC and
ATA calls in the sole readiness owner; they are not controller-local
surrogates. Retained S2 and S3 focused smokes prove the controller-specific
pending, cancellation, status, DRQ, IRQ, and VM/DOS PIO behavior which the
chain services. The current-gate run executes all three owners.

## Explicit transfers

This task supplies a deterministic service boundary, not a physical device
latency model. LBA48, ATAPI, bus-master IDE, host-async media, cache policy,
rotational command-duration tables, and broad bus wait-state accounting remain
outside the retained ATA/FDC command surface. DMA routing/completeness, PIC
delivery compliance, platform signal wiring, and per-device L3 mechanical
duration calibration remain the named later Queue packages. No Windows guest
or board-wide parity conclusion follows from this reconciliation.
