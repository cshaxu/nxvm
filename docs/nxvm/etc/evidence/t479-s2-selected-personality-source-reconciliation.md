# T479 S2 Selected Fixed-Disk Personality Reconciliation

`M5:T479:S2:HDC-PROFILE-AUDIT:OK`

This S2 record supersedes neither of the accepted S1 ledgers.  It reconciles
their finite pre-expansion universe with the owner's subsequent requirement to
support fixed disks for IBM 5160, IBM 5170, DeskPro 386 and `default-at`.
Source facts are normative only where a primary document identifies them;
86Box, PCjs, Bochs, MAME and QEMU remain corroboration, never a substitute.

## Selected configurations, not mutable options

| Consumer | Source-qualified controller fact | Current fact | S2 disposition |
| --- | --- | --- | --- |
| `default-at` | ATA-3 defines the retained ATA PIO task-file contract. | One ATA PIO `core_machine_hdc` route exists. | Retain as the explicit ATA personality and regression corpus. |
| DeskPro 386 Model 40 | T386's Compaq primary ledger fixes its WD integrated controller, `1F0h`--`1F7h`, `3F6h`, IRQ14 and shared `3F7h` read. | One Compaq personality and one HDC state exist. | Retain, migrate only through the common immutable personality input. |
| IBM PC/XT 5160-268 | IBM's 1988 product directory lists XT-268 among systems using the IBM 20 MB adapter; the IBM options/adapters reference defines the distinct `320h`--`32Fh` fixed-disk range, an 8-bit command/data register and controller-select write at `322h`. | Historical T373 classified the unimplemented selected composition as no fixed disk. | Treat that prior absence as a baseline-selection conflict, not evidence that ATA may substitute. S3/S6 must verify the exact 5160-268 shipment/adapter pairing and select a frozen fixed-disk configuration before profile binding. |
| IBM PC/AT 5170 Model 339 | T476's accepted IBM source selection fixes Model 339 as no fixed disk. IBM's Fixed Disk and Diskette Drive Adapter documentation separately defines the MFM/ST-506 task-file family. | The sole 5170 profile deliberately rejects HDD. | Preserve Model 339 unchanged; S5 introduces only a separately named source-defined 5170 fixed-disk configuration. |
| Selected ESDI configuration | WD1007A-WAH OEM Manual (1988) identifies a PC/AT-compatible ESDI controller with `1F0h`--`1F7h`, `3F6h`, `3F7h`, 16-bit fixed-disk data and IRQ14. | No ESDI personality exists. | S2 selects WD1007A-WAH as the only ESDI research target. S7 may reuse only source-proven task-file operations; it must not equate the controller with ATA. |

## Owner and direction conclusion

`core_machine_hdc` remains the only proposed mutable owner.  Its copied
construction input must become a discriminated personality because the IBM XT
adapter's byte-stream command/status ports and DMA relationship are not a
task-file variation.  A profile selects one valid immutable personality before
machine construction; Core then owns its registers, command phase, DRQ, IRQ
and future deadline.  VM HDD media continues to own only image bytes and
durable persistence.

The direction is therefore:

`profile descriptor -> copied Core HDC personality -> one core_machine_hdc -> copied IRQ/DRQ effects`

No runtime profile mutation, ATA fallback, second CHS/media cache, or per-card
controller object is admissible.

## Evidence quality and limits

| Claim class | Primary record | Cross-model check | L-level/disposition |
| --- | --- | --- | --- |
| XT port range and command/status split | IBM *PC/XT Technical Reference* (April 1984), I/O map; IBM *Options and Adapters*, Fixed Disk Adapter. | 86Box has a separate XT ST-506/Xebec device, distinct from its AT WD1003 and IDE devices. | Manual L3 for the ports/register grammar; controller service duration remains unclaimed. |
| XT controller-select at `322h` | IBM *Options and Adapters*, Fixed Disk Adapter programming considerations. | No implementation supplies normative timing. | Manual L3 for the write effect; physical delay is unsupported. |
| AT MFM task-file, PIO, sectors and geometry envelope | IBM *PC AT Fixed Disk and Diskette Drive Adapter* (31 August 1984). | 86Box and PCjs keep AT ST-506 distinct from IDE. | Manual L3 per source-ledger row; 86Box's rough sector delay is not adopted. |
| Compaq Model 40 task-file/shared status | Compaq source retained by T386 S5. | Existing accepted Model-40 route is a behavior regression corpus. | Manual L3 for the retained logical controller facts. |
| ATA task-file | ATA-3 retained by T468. | Existing ATA tests and bounded implementations corroborate only. | Manual L3 for accepted T468 rows. |
| WD1007A-WAH host map and bus facts | Western Digital *WD1007A-WAH OEM Manual* (February 1988), register map and system-interface sections. | 86Box has a distinct AT ESDI family; no third-party timing estimate is normative. | Manual L3 for named registers/IRQ/bus-width facts; command coverage and service time remain S7 work. |

The S2 selection deliberately leaves raw flux, ECC correction, controller
firmware internals, drive mechanics, and any unsourced service deadline outside
the implementation universe.  An external implementation that supplies a
number without the primary contract is not an L3 input.
