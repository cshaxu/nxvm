# T503 S7 HDC To Consumer Direct Audit

`M5:T503:S7:HDC-FIRMWARE-ROUTE:OK`

## Sources and method

The selected controller families are distinct immutable HDC personalities in
one Core-owned `core_machine_hdc`; the VM media provider owns only image bytes
and persistence. This audit re-read the existing primary ledgers for ATA,
DeskPro/Compaq WD and IBM WD1003, then directly rendered IBM publication
6139790 (printed pp. 1, 4, 14 and 15). Those pages establish the XT adapter's
DMA3, IRQ5, completion-byte, result-read and mask-register relations. The
manual calls `323h` a DMA/interrupt mask register but does not assign bit
positions. Read-only 86Box `hdc_st506_xt.c` assigns DMA enable bit 0 and IRQ
enable bit 1, gates both routes, and clears IRQ when result data is read.
PCjs `hdc.js` independently raises IRQ5 at result phase and clears it on data
read, while explicitly marking its undocumented hardware-status bits as
guesses. NXVM adopts no status-bit value or timer from either project.

ATA-3 remains primary for ATA PIO status, alternate-status, `nIEN`, `SRST` and
PIO completion. The prior IBM WD1003 source ledger remains primary for the AT
task-file, IRQ14, `3F6h` high-head input and PIO result sequence. The prior
DeskPro source ledger remains primary for its WD-40MB/`3F7h` shared-read
contract. No local MAME or QEMU controller source was available; the local
Bochs tree has no selected Xebec/WD1003 implementation. These absences make no
negative claim.

## Complete selected route matrix

| Personality | Core state and board route | Consumer/acknowledgement | Disposition |
| --- | --- | --- | --- |
| ATA PIO (`default-at`) | Task file -> pending command -> Core readiness advance -> PIO DRQ/data -> Core media owner -> IRQ14. `nIEN` only gates publication. | Command/status read clears pending IRQ; alternate-status does not; SRST resets the one state owner. | Retained Manual L3 ATA-3 path; existing `vm-hdc-port-smoke` and `core-machine-hdc-smoke` cover PIO, status versus alternate status, reset, no-media and error. No selected ATA DMA route exists. |
| Compaq WD 40MB (Model 40) | Distinct task-file personality -> one Core PIO/CHS state -> media -> IRQ14; fixed low `3F7h` bits are wired into the existing FDC-owned bit 7 read. | Status acknowledges IRQ14; alternate-status does not. | Retained Manual L3 route; `core-machine-compaq-hdc-s5-smoke`, machine-wiring smoke and Model-40 VM smoke cover the shared read and reset/error path. No second HDC or FDC state exists. |
| IBM WD1003/ST-506 (Model 339) | Separate task-file personality -> one Core PIO/CHS/step-selector state -> media -> IRQ14. IBM `3F6h` supplies only high head bit and never enters ATA control state. | Status read acknowledges IRQ14; firmware observes the selected profile through its ordinary fixed-disk port/INT13 composition. | Retained Manual L3 logical PIO route; physical MFM/ECC/rotation/ready remains an explicit unsupported boundary, with no invented deadline. |
| IBM/Xebec XT (5160) | Six-byte DCB -> selected Core Xebec state -> DMA3 only when `323h` DMA enable is set -> one media owner -> completion/status stack; completion raises IRQ5 only when `323h` IRQ enable is set. | First result-byte read clears the enabled IRQ5; reset clears IRQ and DMA request. The BYOB option-ROM remains an external firmware consumer of this normal port contract. | **Repaired.** Manual L3 for the causal DMA3/IRQ5/mask/result relationship; the bit positions are bounded 86Box corroboration. Physical mechanics, status-register bit values and service delay remain unsupported/L1 rather than guessed. |

## Repair and ownership result

The old Xebec branch saved the `323h` mask pattern but did not use it: DMA was
unconditionally requested and completion never reached IRQ5. The repair
centralizes both effects in the existing Xebec owner: one helper synchronizes
the existing DMA3 request only while a transfer phase and its mask enable are
present; the existing response transition raises the existing PIC source only
when IRQ enable is present; the existing result read clears that source. It
adds no controller, media cache, profile switch, BIOS condition or scheduler.

The owner-local Xebec smoke proves disabled DMA stays unrequested, enabling it
during a transfer asserts the same existing request, enabled completion raises
IRQ5 and reading the result clears it. The existing ATA, Compaq and IBM WD1003
smokes retain their separate PIO/IRQ semantics. The changed implementation is
`+20/-1` lines; its focused regression is `+27/-0` lines. The status-packet
correction in the active S7 record is documentation only.

## Timing and transfer boundary

No selected HDC personality publishes a newly invented mechanical service
deadline. The existing Core readiness callback remains the only causal
completion progression path. IBM/ATA values that define register ordering and
the IBM WD1003 step selector remain L3 at their stated boundary; 86Box's
controller delays and PCjs status workarounds are not used. Therefore no HLT
path is falsely declared physically timed merely because an HDC is configured.
