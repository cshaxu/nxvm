# T479 S1 AT Fixed-Disk Source And Interface Ledger

`M5:T479:S1:HDC-SOURCE-LEDGER:OK`

## Source quality

| Source | Quality and permitted use |
| --- | --- |
| IBM 5170 Technical Reference, 1502243 (March 1984), SHA-256 `F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D` | Scanned/OCR PDF: page images are primary evidence for selected IBM board facts, but current text extraction is inconsistent. It does not establish a WD1003 command/timing contract here. |
| Compaq DeskPro 386 Technical Reference Guide, Vol. II, chapter 7 | Primary material already reviewed by T386 S5; that evidence retains the source-backed register/command facts. |
| X3T13 ATA-3 2008D Rev. 7b | Born-digital primary manual retained by T468 for ATA PIO task-file, PIO, reset and status rows. |
| 86Box `hdc_st506_at.c` and `hdc_ide.c` | Read-only corroboration. It separates WD1003/ST-506 from IDE and labels its 980 us sector delay a rough estimate. |
| PCjs `machines/pcx86/modules/v2/hdc.js` | Read-only corroboration: separates the AT combo-card fixed-disk portion from FDC and records a 1F0h task-file model. |
| Bochs 2.6 compatibility tree | Present, but no selected XT/AT MFM controller model was found. |
| MAME and QEMU | No local checkout was available; no claim is derived from them. |

## Frozen controller universe

`Manual L3` is a primary-document function/number/formula. `Other L3` is
corroboration only. `Input L3` is a real immutable receiver, not proof that a
profile supplies the value. L2 is an internal estimate; unsupported has no
implementation claim.

| ID | Fact and boundary | Evidence / level | T479 disposition |
| --- | --- | --- | --- |
| IBM-1 | Selected Model-339 has no fixed disk. | T476 S1 profile ledger; Manual L3. | Preserve absence; no default ATA/MFM bind. |
| IBM-2 | IBM AT fixed disk is a combo-card/ST-506 family, not ATA. | 86Box/PCjs; Other L3 only. | S3 needs a controller-primary source before a WD1003 personality. |
| IBM-3 | WD1003 commands are Restore, Read, Write, Verify, Format, Seek, Diagnose, Set Parameters. | 86Box; Other L3 only. | No implementation from this source alone. |
| IBM-4 | WD1003 CHS requires geometry and physical track state. | 86Box; Other L3 only. | Logical RAW-IMG cannot claim physical-track behavior. |
| IBM-5 | 86Box uses IRQ14/nIEN-style masking. | Other L3 only. | Core has an IRQ receiver; board value remains unselected. |
| IBM-6 | 86Box 980 us sector interval is explicitly estimated. | L2. | Never promote or reuse as pacing. |
| CPQ-1 | DeskPro task-file is 1F0h--1F7h and 3F6h is alternate-status/device-control. | T386 S5 Compaq primary; Manual L3. | Retained `COMPAQ_WD_40MB`. |
| CPQ-2 | Fixed low seven bits share FDC direction port 3F7h; FDC supplies bit 7. | T386 S5 primary; Manual L3. | Retained FDC owner plus bounded HDC wired-OR. |
| CPQ-3 | Status acknowledges IRQ14; alternate status does not. | T386 S5 primary; Manual L3. | Retained one HDC port owner. |
| CPQ-4 | CHS read/write, verify, recalibrate, seek, initialize and diagnostics are selected; diagnostic success is 01h. | T386 S5 primary; Manual L3. | Retained explicit Compaq subset. |
| CPQ-5 | Long/format/ECC/recovery/physical identity need a drive/media model. | T386 S5 primary; unsupported. | Do not alias them to ATA PIO. |
| ATA-R1--R5 | Task-file owner, status/IRQ, 512-byte PIO, BSY/DRQ command gate, 256-word Identify. | ATA-3; Manual L3. | Retained one `hdc.c` ATA path. |
| ATA-F1--F2 | Cable signals and DMA handshake. | ATA-3 Manual L3; no selected board data. | Unsupported; no invented cable/DMA state. |
| ATA-F3 | SRST/nIEN/reset ordering. | ATA-3; Manual L3. | Logical reset retained; physical delay unselected. |
| ATA-F4--F5 | PIO ordering and mode timings. | ATA-3 Manual L3. | Order retained; duration/deadline lacks an input. |
| ATA-T1--T3 | Immutable port/IRQ construction, media binding and media error route. | Existing copied plan/registry; Input L3. | Retained one construction and media owner. |
| ATA-T4 | Service progression uses Core timeline order. | Existing Core owner; L1. | No deadline until a qualified service plan exists. |
| ATA-T5 | ATA and Compaq are distinct personalities. | T386/T468; Manual/Other L3. | Retained; no compatibility shim. |
| ESDI-1 | No concrete WD1005/WD1007 or other ESDI controller selected. | No primary controller source. | Unsupported; future source-gated admission only. |

The finite universe is 28 rows. S2 traces every row through construction,
ports, media, IRQ and timeline owners; it does not replace a missing manual
with an emulator implementation.
