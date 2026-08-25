# T468 S1 ATA-3 Function And Timing Checklist

## Sources And Cross-Check Method

The primary source is X3T13, *AT Attachment-3 Interface*, 2008D Rev. 7b,
January 1997, 182 PDF pages, SHA-256
`FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237`.
It is born-digital with searchable text, tables and figures. Rendered review
of PDF page 30 (printed page 16) confirms that Command is write-only, captures
the command-block parameters, starts processing and clears a pending
interrupt; rendered review is authoritative and OCR remains locator-only.

Manual clauses 4--5 and 7--9 define host/device facts. They do not select an
ISA IDE adapter, host port decode, IRQ wiring, device identity, disk image,
media persistence, cable or controller timing. Those inputs need a source
qualified profile or remain L2. The previous T450 S17/S18 15-row inventory is
the frozen base; this S1 verifies its primary source, expands its external
comparison, and does not promote current code by resemblance.

Read-only corroborators are 86Box `4fef696` (`src/disk/hdc_ide.c`), Bochs 2.6
compat (`iodev/harddrv.cc`), QEMU main (`hw/ide/core.c`) and MAME main
(`ide_controller_32` integration). 86Box and Bochs implement configurable
ATA/IDE task files, device identities and their own timing; QEMU is a modern
synthetic IDE/ATAPI model with host asynchronous backing; MAME integrates
selected IDE controllers with board-specific IRQs. They corroborate logical
register/PIO/IRQ behavior but cannot override ATA-3 or select NXVM's board,
device or media. PCjs contains no ATA/IDE controller implementation in the
checked revision, so it is inapplicable rather than negative evidence.

## Checklist 1

| ID | Primary requirement | Cross-model result | Final source disposition |
| --- | --- | --- | --- |
| ATA-R1 | ATA-3 5.1--5.2, printed pp. 19--27: command/control block members, directions and aliases. | 86Box, Bochs and QEMU retain task/control blocks; MAME supplies selected-controller wrappers. | Manual L3 for the interface grammar. |
| ATA-R2 | ATA-3 4.2.10, 5.2.1 and 5.2.13, printed pp. 15, 27: Status read clears pending INTRQ; Alternate Status does not. | 86Box has an explicit alternate-status non-clear assertion; Bochs exposes both status addresses; QEMU preserves the distinction. | Manual L3. |
| ATA-R3 | ATA-3 5.2.6 and 8.3--8.5, printed pp. 24--25, 109--116: Data is 16-bit PIO and DRQ controls a data block. | 86Box, Bochs and QEMU use word PIO plus data-phase gating. | Manual L3 protocol; board-cycle conversion remains L2. |
| ATA-R4 | ATA-3 clause 7, printed pp. 40--105: finite command forms and their stated prerequisite/result rules. | All three implement broader, device-specific command sets. | Manual L3 command forms; selected supported subset is device L2. |
| ATA-R5 | ATA-3 7.7--7.8, printed pp. 48--64: IDENTIFY returns 256 words through defined PIO/DMA lifecycles. | Every corroborator synthesizes its own device identity and geometry. | Manual L3 transfer shape; exact word image is device L2. |
| ATA-F1 | ATA-3 4.1--4.2, printed pp. 11--18: ATA signal directions and electrical requirements. | Models provide adapter-specific signal bridges. | Manual L3 interface facts; chosen bridge/cable remains L2. |
| ATA-F2 | ATA-3 4.2.8--4.2.13 and 8.6, printed pp. 15--17, 117--121: DMA handshake, INTRQ/nIEN and reset relations. | 86Box, Bochs and QEMU each choose different DMA/controller integration. | Manual L3 logical relation; board DMA/arbitration is L2. |
| ATA-F3 | ATA-3 8.1--8.2, printed pp. 106--108: power/hardware/SRST reset state, diagnostics and sequence. | Corroborators use different delay and device-default policies. | Manual L3 reset sequence; host-time conversion is L2. |
| ATA-F4 | ATA-3 8.3--8.5, printed pp. 109--116: PIO handshake, DRQ, IORDY and command termination. | Corroborators agree on phase gating but schedule it differently. | Manual L3 logical phase; Core tick/board waits are L2. |
| ATA-F5 | ATA-3 clause 9, printed pp. 122--137: register/PIO/DMA/reset electrical timing tables. | Models select controller/device-specific values independently. | Manual L3 timing facts; controller/cable/device values are L2. |
| ATA-T1 | ATA-3 does not choose PC decode, IRQ14, DMA channel or compatibility personality. | Each model binds ports and IRQs in a board/controller implementation. | L2 until a source-qualified board profile supplies the values to a sole HDC owner. |
| ATA-T2 | ATA-3 allows device selection and diagnostics but does not choose a disk/device identity. | Corroborators select master/slave presence and identity in configuration. | L2 until a source-qualified device policy supplies it to the HDC owner. |
| ATA-T3 | ATA-3 does not define image grammar, persistence, host failure or media lifecycle. | Corroborators use their own backing abstractions. | L2 until a project backing-media contract is selected. |
| ATA-T4 | ATA-3 plus T449 requires one Core HDC/media/PIC/scheduler path for any accepted term. | Models have their own timers and host mechanisms. | L2: no source-qualified timing input and sole consumer have yet been verified. |
| ATA-T5 | ATA-3 is not IBM MFM/ST-506 and supplies no Model-339 fixed-disk claim. | 86Box and MAME retain MFM and IDE as distinct families. | Architecture boundary; ATA must not be used as a fallback label. |

## S1 Result

The ledger retains the frozen fifteen identifiers: `ATA-R1`--`ATA-R5`,
`ATA-F1`--`ATA-F5` and `ATA-T1`--`ATA-T5`. ATA-3 establishes the explicitly
identified interface facts at Manual L3; it does not make current HDC an ATA
device. T1--T4 remain L2 until S2 proves a source-qualified board/device/media
input and its one existing HDC consumer. If that path already accepts the
source-qualified input, the affected row is Board L3 rather than L2; no second
state or timing path may be added merely to promote it. T5 forbids an ATA-to-MFM
substitution. S2 must audit every row against HDC/media/PIC/scheduler/profile
code and repair only eligible Manual/Other/Board-L3 behavior through existing
owners.
