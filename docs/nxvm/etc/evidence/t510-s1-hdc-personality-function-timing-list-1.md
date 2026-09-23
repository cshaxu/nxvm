# T510 S1 HDC Personality Function And Timing List 1

`M5:T510:S1:HDC-LIST-1:OK`

## Source Qualification

| Personality | Primary source | External cross-check | Source result |
| --- | --- | --- | --- |
| ATA PIO | X3T13 ATA-3, 2008D Rev. 7b, SHA-256 `FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237` | 86Box `src/disk/hdc_ide.c`; Bochs ATA task-file model | Manual-L3 for host protocol.  The 86Box generic `200 * IDE_TIME` completion is Other-L2 only. |
| IBM WD1003/ST-506 | IBM *PC AT Fixed Disk and Diskette Drive Adapter* (31 August 1984), as qualified by T479; IBM 5170 TR 1502243, SHA-256 `F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D` | 86Box `src/disk/hdc_st506_at.c`; PCjs AT controller path | Manual-L3 for task-file/PIO/IRQ/step-selector protocol.  86Box's `200 * MFM_TIME` and 980-us sector estimate are Other-L2 only. |
| Compaq/WD 40 MB | Compaq *DeskPro 386 Technical Reference Guide*, Vol. II, chapter 7, as qualified by T386 S5 | 86Box DeskPro-386 construction plus its separate IDE/MFM controllers | Manual-L3 for the selected port, IRQ and shared-`3F7h` boundary.  Neither source specifies this raw-image controller's service duration; a generic completion quantum requires an explicit Other-L2 model, never a Manual-L3 claim. |
| IBM/Xebec XT | IBM *20MB Fixed Disk Drive Adapter* 6139790, reproduced in *Technical Reference: Options and Adapters, Vol. 2*, SHA-256 `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F` | 86Box `src/disk/hdc_st506_xt.c`; PCjs Xebec byte-stack model | Manual-L3 for DCB, byte stack, DMA3/IRQ5 wiring and result protocol.  86Box's `ST506_TIME` 250-us controller timer is Other-L2 only. |

The IBM 6139790 PDF is an OCR-bearing scan.  Its printed pages 1, 3, 7,
14 and 15 were visually qualified by T494; OCR is not used to manufacture a
numeric timing relation.  No third-party code, firmware or media is imported.

## Complete Selected Universe

| Row | Personality | Function and timing relation | Level and required disposition |
| --- | --- | --- | --- |
| A1 | ATA | Task-file capture, BSY before completion, PIO DRQ, Status acknowledgement, Alternate Status non-acknowledgement, nIEN/SRST and LBA28 selection. | Manual-L3; one task-file phase owner. |
| A2 | ATA | Generic command/sector completion is not immediate. | Other-L2: 86Box's generic `200 * IDE_TIME` model supplies the existing frozen 200-step quantum; it is not mechanical or wall-clock time. |
| W1 | WD1003 | `1F0h`--`1F7h`, IRQ14, CHS PIO, command/status acknowledgement and `3F6h` head-bit extension are distinct from ATA control. | Manual-L3; one WD1003 task-file owner. |
| W2 | WD1003 | Restore, read, write, verify, format, seek, diagnose and set-parameters command families; 512-byte logical data phase and bounded CHS. | Manual-L3 for supported logical form; raw MFM/ECC/format details remain unsupported by the raw-image medium. |
| W3 | WD1003 | The 8-MHz Model-339 macro axis can express 86Box's 2-ms generic command minimum as 16000 elapsed units and its 980-us sector estimate as 7840 units. | Other-L2, profile-frozen values only; no physical-clock assertion. |
| C1 | Compaq | `1F0h`--`1F7h`, `3F6h`, IRQ14 acknowledgement and low-seven-bit fixed-disk contribution at shared `3F7h`; FDC remains bit-7 owner. | Manual-L3; no ATA, IBM-WD1003 or FDC alias. |
| C2 | Compaq | Normal CHS PIO read/write, verify, recalibrate, seek, initialize and diagnostic-success `01h`; Long/format/ECC/recovery are outside logical-media representation. | Manual-L3 logical subset; unsupported physical commands must fail in the same owner. |
| C3 | Compaq | A service duration is absent from the selected manual and the local DeskPro reference does not select an internal HDC model. | Retain explicit L1 until an admitted Other-L2 controller model is justified; do not borrow ATA or WD1003 timing merely because ports overlap. |
| X1 | Xebec | `320h`--`323h`, select then six-byte DCB, response/sense byte stack, reset, AEN/mask, DMA3 and optional IRQ5. | Manual-L3; one Xebec branch, no task-file alias. |
| X2 | Xebec | Selected Type-2 logical read/write sector transfer uses the existing DMA3 provider; completion/result consumption clears the same IRQ source. | Manual-L3 for ordering and wiring; raw MFM, seek, ready/index and ROM-driven behavior are excluded. |
| X3 | Xebec | Command service must be observable before DMA/response, not instantaneous at the sixth DCB byte. | Other-L2: 86Box's named `ST506_TIME` 250-us timer is a controller compatibility quantum; it is not a claim about physical drive time. |

## T510 S1 Decision

The selected universe proves one common structural requirement: every
personality must publish its own HDC-owned completion deadline before it
publishes DRQ, DMA or an IRQ-visible result.  It does **not** prove one common
latency constant.  T510's implementation batch may therefore apply the ATA,
WD1003 and Xebec external-L2 relations only at their frozen VM constructions,
and must leave the Compaq duration explicitly non-qualified unless distinct
evidence is found.
