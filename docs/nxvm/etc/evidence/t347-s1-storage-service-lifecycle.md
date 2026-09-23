# M5 T347 S1: Storage Service Lifecycle Foundation

## Decision

T346's readiness callback remains the single core owner of storage-service
visibility.  It already follows the equal-tick DMA/PIT/PIC arbitration
callback, so storage sources made ready at a readiness tick become eligible to
DMA/PIC only at the following arbitration tick.  T347 must retain that rule.
No controller receives a host clock, a private scheduler, or an arbitrary
hardware-duration constant.

The common mechanism is deliberately small: a command/data port transaction
may capture guest input and set controller-local pending state, but it may not
perform completion-side media access or publish DRQ/DMA/IRQ.  The existing
machine readiness callback advances each controller-local service state once
per due tick in stable FDC-then-ATA order.  A controller's local owner alone
interprets its command set, media geometry, result bytes, and data phase.
Reset, media-loss cancellation, and controller finalization clear pending work
before the next service tick.  This is a deterministic service-visibility
contract, not a claim that one tick models a physical seek or rotation time.

## Existing Timeline And Ownership

`core_machine_readiness_tick` is scheduled after the arbitration callback and
currently executes FDC media observation, ATA refresh, then RTC advancement.
The FDC refresh is only a media-generation/DIR observation; ATA refresh is a
no-op.  The task therefore changes the existing owner rather than introducing
a second scheduler.  The resulting intended per-tick order is:

```text
DMA -> PIT -> PIC -> FDC service/observation -> ATA service/observation -> RTC
```

If a later primary contract requires a distinct FDC versus ATA clock ratio or
a bus occupancy interval, it is a later bounded timing admission.  The
current task establishes the state and visibility boundary first.

## FDC Path Inventory

| Input/path | Current behavior | Required T347 disposition |
| --- | --- | --- |
| `SPECIFY`, `SENSE DRIVE`, `VERSION` | Final command byte directly changes command/result state. | Capture then service on next readiness tick; preserve their controller-local result semantics. |
| `RECALIBRATE`, `SEEK`, `READ ID` | Final byte directly changes position/result and can assert IRQ. | Pending command; service step performs validation/position/result/IRQ. |
| `READ DATA`, `WRITE DATA`, `READ TRACK`, `FORMAT TRACK` | Final byte directly creates execution phase and can assert DMA request. | Pending command; only service step validates and publishes execution/DRQ. DMA or non-DMA byte movement remains controller-local after publication. |
| DMA callbacks and non-DMA data port | Transfer one byte and can complete result/IRQ. | Retain byte owner, but final transfer completion must enter a service-completion boundary if it would otherwise publish completion in the data transaction. |
| DOR reset, drive loss, finalization | Clears active transfer/DRQ and/or controller state. | Also cancels any pending command/completion before its service event. |
| `refresh` | Observes media generation and DIR disk-change state. | Remains observation only; does not duplicate service completion. |

The FDC's existing command, execution, and result phases are insufficient to
represent the required pre-service pending and post-data completion boundaries.
T347 S2 owns those local states and their MSR/RQM/CB evidence.

## ATA PIO Path Inventory

| Input/path | Current behavior | Required T347 disposition |
| --- | --- | --- |
| Command port `20h`, `30h`, `ECh` | Validates, reads/builds data, sets DRQ and raises IRQ synchronously. | Capture task-file/command, set guest-visible pending/busy state, and validate/load/identify only from the next service tick. |
| Data-port read/write final word | Advances sector, loads/stores next sector, and can raise completion IRQ synchronously. | Capture completed data sector; service step owns next-sector load/store, next DRQ, final completion, and media failure result. |
| Status vs alternate-status read | Status read clears IRQ; alternate status does not. | Preserve this published interface through pending and completed states. |
| nIEN/SRST | Clears IRQ or resets controller synchronously. | Preserve control-register semantics and cancel all pending service transitions atomically. |
| `refresh` | No operation. | Becomes controller-local observation only if a future media-generation contract requires it; it cannot remain a claimed service path. |

ATA needs explicit `COMMAND_PENDING` and `DATA_COMPLETION_PENDING` states in
addition to idle/data-read/data-write.  Their implementation remains local to
the ATA owner; no media-provider ABI change is implied.

## Caller And Write Sweep

The only current periodic storage caller is `core_machine_readiness_tick`.
FDC writes to command phase/result phase/DMA request/IRQ through its data-port,
DOR, DMA callback, reset, and finalization paths.  ATA writes to status/phase,
data buffer, task-file progression, and IRQ through command/data/control/reset
paths.  Timeline scheduling is owned by `machine.c`; controller files have no
timeline pointer and must not gain one.  This preserves one construction and
one cancellation owner.

## Migration Plan And Transfers

1. **T347 S2:** add the controller-local pending/completion state vocabulary,
   route service through the existing readiness owner, and migrate the FDC
   command and transfer-completion paths with focused MSR/DRQ/DMA/IRQ/reset
   probes.
2. **T347 S3:** migrate ATA command and sector-completion paths with task-file,
   BSY/DRQ/status, IRQ/nIEN/SRST, media-failure, and cancellation probes.
3. **T347 S4:** perform a cross-controller service-order, trace, reset, and
   current-gate closure audit.  It transfers real mechanical durations,
   LBA48/ATAPI, broad command sets, generic bus waits, and storage backends
   once to Queue/TODO.

No T347 S may claim disk rotational timing, generic asynchronous host media, or
Windows installation readiness from these transitions alone.

