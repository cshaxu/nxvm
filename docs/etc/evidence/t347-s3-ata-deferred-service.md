# M5 T347 S3: ATA PIO Deferred Command and Sector Service

## Local State Contract

The HDC retains its existing primary PIO surface: `READ SECTORS` (`20h`),
`WRITE SECTORS` (`30h`), `IDENTIFY DEVICE` (`ECh`), CHS/LBA28 selection,
status and alternate-status reads, `nIEN`, and `SRST`. Command execution,
media access, data-buffer ownership, task-file progression, and IRQ14 remain
in `hdc.c`; no provider, DMA, host, or generic scheduling API changed.

| Guest transaction | Local state | Service action | Guest-visible result |
| --- | --- | --- | --- |
| Command write | `PENDING_COMMAND`, `BSY`, no DRQ/new IRQ | Restore the captured task-file image and execute the retained command | data phase with DRQ/IRQ, or retained error/IRQ |
| Final read word | `PENDING_READ_SECTOR`, `BSY` | Complete final transfer or load/advance one next sector | next DRQ/IRQ or idle completion/IRQ |
| Final write word | `PENDING_WRITE_SECTOR`, `BSY` | Store and complete/advance one sector | next DRQ/IRQ or idle completion/IRQ |

Command capture copies the command, features, sector count/number, cylinder,
and drive/head at the command port. Subsequent task-file writes cannot alter
the pending command. A service transition that prepares a later sector first
commits `DATA_READ` or `DATA_WRITE`, `DRDY|DSC|DRQ`, and its new data index;
this prevents a later readiness tick from consuming a second completion before
the guest has transferred that sector.

## Timeline and Cancellation

`core_machine_readiness_tick` advances FDC service/observation first, then
ATA service/observation, preserving T346's equal-tick order
`DMA -> PIT -> PIC -> FDC -> ATA -> RTC -> KBC -> VADP`. ATA work made ready
there is visible to PIC arbitration only at the following due tick. The
existing status read clears IRQ while alternate status does not; `nIEN`
suppresses line publication; SRST/reset clears all pending state, data index,
sector count, and IRQ before a later service tick. Failure through absent,
read-only, invalid-range, or unsupported media is evaluated only by service.

## Proof and Sweep

The HDC owner smoke proves command BSY/no-IRQ, task-file capture, READ and
WRITE final-sector pending states, deferred `IDENTIFY` completion, nIEN,
status acknowledgement, SRST cancellation, CHS media errors, and retained
write/read-only behavior. The ATA DOS probe now polls BSY/DRQ before the first
and later data sectors and before issuing its next command; it proves two-sector
PIO write/read through the real VM path. The HDD boot probe remains green.

The command/data/task-file/status/control/nIEN/SRST/media/reset/finalization/
refresh paths and all `execute`, next-sector, and completion callers were
reviewed. Only `core_machine_hdc_advance`, called by the existing readiness
owner, invokes command execution or final-sector progression. FDC is not
changed by S3. Fresh configuration, focused ATA/HDC/VM probes, documentation
governance, whitespace validation, and the current gate are recorded with the
implementation commit.
