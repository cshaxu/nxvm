# T507 S10 HDC Personality Re-Audit

`M5:T507:S10:HDC-PERSONALITIES:P1`

## Sources And Boundaries

The retained primary ledgers are ATA-3
[`t468-s1-ata3-function-timing-checklist.md`](../research/t468-s1-ata3-function-timing-checklist.md),
IBM's 1984 *Options and Adapters, Volume 2* Xebec chapter
[`t494-s1-xebec-original-source-ledger.md`](t494-s1-xebec-original-source-ledger.md),
the IBM AT fixed-disk adapter ledger
[`t479-s4-ibm5170-fixed-disk-source-ledger.md`](t479-s4-ibm5170-fixed-disk-source-ledger.md),
and the selected Compaq route ledger
[`t386-s5-deskpro-storage-controller.md`](t386-s5-deskpro-storage-controller.md).
The IBM source is the visually checked archival scan SHA-256
`B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F` in
`assets/manuals/controllers/ibm`; ATA-3 is the born-digital source SHA-256
`FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237` in
`assets/manuals/controllers/ata`.  The existing Compaq scan remains an
owner-managed research asset; it establishes the selected port/IRQ route, not
a drive-service duration.

Read-only local 86Box sources retain distinct IDE, AT WD1003 and XT Xebec
implementations; PCjs corroborates the separate Xebec byte-stack route; Bochs
corroborates ATA task-file/PIO shape only.  Their drive delays are model
choices, not adopted timing.  No local selected MAME/QEMU HDC source is
available and no external code is imported.

## List 1 And List 2 Reconciliation

| Personality | Manual/Other-L3 relation | Sole current route and result |
| --- | --- | --- |
| ATA PIO | ATA-3 defines task-file direction, command capture, PIO DRQ, Status versus Alternate Status, nIEN/SRST and logical phase order: Manual L3.  Device identity, cable/adapter and service duration remain L2/L1. | `hdc.c` owns one task file, phase and PIO buffer; media owns bytes; PIC owns IRQ14.  No DMA binding exists. |
| IBM WD1003/ST-506 | IBM adapter source defines distinct task-file/CHS command grammar and 3F6h high-head bit: Manual L3.  Rotation, MFM/ECC and service duration are not defined by this logical-image route. | The explicit WD1003 personality owns its task file, CHS and step selector; 3F6h is not ATA control.  It uses PIO/IRQ14 only. |
| Compaq WD 40 MB | Selected Compaq documentation defines 1F0h--1F7h, 3F6h, IRQ14 and the 3F7h shared-read relation: Manual L3 at that interface boundary.  Media mechanics/service remain L1/L2. | The explicit Compaq personality retains its own selection rule; Core combines only its low 3F7h contribution with the FDC-owned bit 7.  It has no ATA LBA or DMA alias. |
| IBM/Xebec XT | IBM defines 320h--323h, DCB/result stack, AEN, DMA3 and optional IRQ5: Manual L3.  Hardware status bits and mechanical service duration remain L1. | The explicit Xebec personality owns byte-stack, response, DMA3 binding and IRQ5 mask.  It neither reuses task-file state nor accesses media through a second cache. |

## Scheduler Disposition

The scheduler calls `core_machine_hdc_advance()` only while its one HDC phase
is pending.  None of the selected primary sources supplies a usable conversion
from command/sector service to the Core source axis.  Therefore a non-idle HDC
is correctly recorded as a causal L1 blocker and Turbo alone may use the
existing bounded Core compatibility progression.  It is not a deadline, and
Standard must not manufacture guest time from host time.  This is the same
explicit boundary for every personality, not a per-profile workaround.

No owner defect is found in the complete selected routes: ATA/Compaq/WD1003
are PIO/IRQ14 only; Xebec alone binds DMA3/IRQ5; status/result acknowledgement
clears the same HDC-owned pending IRQ; reset/finalize clears the same owner;
and `machine_board.c` constructs all ports/bindings once before exposing the
controller.  The remaining L1 duration is transferred to S12's whole-Core
scheduler matrix, which must retain it as a bounded compatibility receiver
unless a source-qualified personality deadline is admitted.

## Verification

The focused HDC, Compaq wiring, Xebec wiring and VM HDC/profile tests pass;
the complete repository-only unit replay and P2 actual-diff review are
recorded at closure.  This audit makes no code change because no source-backed
defect or timing value was found.
