# T479 S4 IBM 5170-339 Fixed-Disk Source Ledger

`M5:T479:S4:IBM5170-SOURCE-LEDGER:OK`

## Source Set And Correction

The earlier current baseline said that IBM 5170-339 had no fixed disk. That is
false. IBM's *PS/2 and IBM PC Product Reference*, Version 4.0 (April 1987),
lists 5170-339 as an 8 MHz, 512 KB system with a high-capacity diskette drive,
**one 30 MB fixed disk and adapter**, and a serial/parallel adapter. This S4
record supersedes that historical premise for current work; historical evidence
is preserved unchanged and is not a source for the new configuration.

| Source | Role and quality | Use |
| --- | --- | --- |
| IBM *PS/2 and IBM PC Product Reference*, Version 4.0, April 1987, 5170 model table (`bitsavers.org/pdf/ibm/pc/PS2_and_IBM_PC_Product_Reference_Version_4.0_Apr87.pdf`) | Primary IBM publication; externally hosted scan. | Selects 5170-339, its one 30 MB disk and adapter. |
| IBM *PC/AT Technical Reference* 1502243, March 1984, local `../../../../nxvm-assets/manuals/platform/ibm-5170-technical-reference-mar1984.pdf`, system-board I/O/IRQ/CMOS and BIOS fixed-disk sections | Primary OCR text; tables are legible enough to transcribe but every numeric row is tied to its printed table. | AT integration facts, CMOS types and firmware vectors. |
| IBM *PC/AT Fixed Disk and Diskette Drive Adapter*, 31 August 1984 (`minuszerodegrees.net/oa/OA%20-%20IBM%20PC%20AT%20Fixed%20Disk%20and%20Diskette%20Drive%20Adapter.pdf`), printed pages 1--10 | Primary IBM scan with independently readable text and rendered tables. | Normative adapter task-file, command, reset, status, interrupt, media and step-rate facts. |
| 86Box `src/disk/hdc_st506_at.c`, read-only local checkout | Other L3 corroboration of logical WD1003 command sequencing and AT task-file separation. Its `MFM_TIME`/`SECTOR_TIME` constants are implementation estimates. | Cross-check only; never imports code or timing. |
| PCjs `machines/pcx86/modules/v2/hdc.js`, read-only local checkout | Other L3 corroboration of AT controller port/task-file modeling. | Cross-check only; never imports code or timing. |
| MAME, Bochs, QEMU | No bounded local source was inspected in this S. | Not evidence; no row is promoted from their absence. |

The direct adapter manual is primary even though its scan requires visual/page
verification. An OCR string alone does not establish a fact. No third-party
source, firmware, media, or timing constant enters the repository.

## Finite Functional And Timing Universe

| ID | Required observable | Primary basis | Cross-model result | Level and S5 disposition |
| --- | --- | --- | --- | --- |
| A1 | 5170-339 includes one 30 MB fixed disk and adapter. | IBM product-reference model row. | 86Box/PCjs do not select this IBM SKU; not required. | Manual L3; correct frozen profile. |
| A2 | Fixed-disk I/O range is `1F0h`--`1F7h`; fixed disk uses IRQ14. | 5170 TR I/O and IRQ tables. | Both models use an AT task-file route. | Manual L3; configure one Core personality and IRQ route. |
| A3 | CMOS byte `12h` encodes fixed-disk types for C and D. | 5170 TR CMOS table. | Not a controller substitute. | Manual L3; select drive-C type only after exact 30 MB type is proved. |
| A4 | ROM vectors `41h` and `46h` point at first/second fixed-disk parameters. | 5170 TR BIOS vector text. | PCjs carries AT fixed-disk BIOS support. | Manual L3; bind existing firmware table or repair it in S5. |
| A5 | BIOS type 3 is 615 cylinders, 6 heads, write-precompensation 300, landing zone 615 and 17 sectors/track. It is the 30 MB class selected for Model 339. | 5170 TR BIOS fixed-disk-parameters table and `FD_TBL` listing plus product-reference Model-339 30 MB row. | PCjs corroborates type 3 as the IBM 30 MB class. | Manual L3; bind type 3 for drive C. |
| A6 | The adapter provides a host task-file and programmed I/O sector transfer. | IBM adapter manual, task-file/programming sections. | 86Box and PCjs retain a distinct AT ST-506 controller path. | Manual L3; do not call the personality ATA. |
| A7 | Data transfer units are 512-byte sectors. | IBM adapter manual, interface/sector description. | 86Box/PCjs model sector transfers. | Manual L3; retain VM logical-sector provider as bytes-only owner. |
| A8 | The adapter supports up to two drives, up to 16 heads and 1024 cylinders. | IBM adapter manual, hardware/interface specification. | 86Box validates bounded geometry. | Manual L3; S5 validates only the selected one-drive geometry. |
| A9 | Restore (`10h|R`), read (`20h|L|T`), write (`30h|L|T`), verify (`40h|T`), format (`50h`), seek (`70h|R`), diagnostics (`90h`) and set parameters (`91h`) are the finite command families. | Adapter printed pages 6--7 command table. | 86Box implements the same WD1003 families; PCjs names matching legacy AT-controller forms. | Manual L3 for command grammar; unsupported commands return the IBM aborted-command result, never ATA fallbacks. |
| A10 | Command write resets IRQ14; status read clears IRQ14. Busy makes other status bits invalid; DRQ gates PIO data transfer. | Adapter printed pages 4--6. | 86Box and PCjs distinguish command/status behavior. | Manual L3; focused acknowledgement and DRQ tests. |
| A11 | `3F6h` is the IBM fixed-disk register: bit 3 extends head selection to 8--15. It is not an ATA device-control/alternate-status alias. | Adapter printed pages 4 and 10; the `3F7h` read is a diskette diagnostic register. | 86Box's WD1003 route corroborates separate controller state. | Manual L3; implement only the high-head function. |
| A12 | Read/write data phase advances CHS sectors and reports invalid CHS/media errors through the one controller state. | IBM adapter read/write/error descriptions. | 86Box corroborates CHS validation and multi-sector behavior. | Manual L3 for grammar/error state; no mechanical timing. |
| A13 | Format, ECC correction, long-sector behavior, raw MFM encoding and recovery are not represented by a plain logical image. | Adapter manual describes physical-controller capabilities; project media is sector bytes only. | 86Box has richer drive-model choices. | Explicit unsupported boundary; no false media claim. |
| A14 | Restore/seek step-rate selector is `35 us`, then `0.5` through `7.5 ms` in `0.5 ms` increments; reset/diagnose selects `7.5 ms`. Restore is bounded by 1023 step pulses. Full seek completion and rotation still depend on drive signals/media. | Adapter printed pages 7--8. | 86Box provides its own delay algorithm, not a substitute. | Manual L3 for the selector/step bound; L1 only for unmodelled drive completion and rotation. |
| A15 | Adapter parity/I/O-channel-check behavior is not established for this selected adapter/profile. | No primary row has yet been identified. | No bounded corroboration adopted. | Unsupported; transfer to physical-media/board work. |

## Cross-Validation Result

The independent models agree on the important architectural distinction:
AT WD1003/ST-506 is a controller personality with a task-file/CHS state
machine; it is not the ATA personality. Their delay algorithms, drive models,
and compatibility extensions are not specification. Therefore S5 may share
only the existing one Core HDC owner's genuinely common PIO/CHS mechanics, and
must add IBM-specific grammar/validation only where the primary table proves
it. It must not add a second controller object, CHS cache, media cache or host
latency path.
