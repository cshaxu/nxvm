# T494 S1 IBM 5160 Xebec Original-Source Ledger

`M5:T494:S1:XEBEC-PRIMARY-SOURCE:OK`

## Qualified Primary Source

| Publication | Asset identity and visual review | Authority admitted for T494 | Explicit limit |
| --- | --- | --- | --- |
| IBM, *20MB Fixed Disk Drive Adapter*, publication 6139790, 17 March 1986, printed pp. 1--17, reproduced as the adapter chapter in IBM, *Technical Reference: Options and Adapters, Volume 2* | Approved asset SHA-256 `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`. The PDF reports an Acrobat Paper Capture producer, so OCR is discovery only. Rendered PDF pp. 408, 410, 414, 421 and 422 were visually read against their printed pp. 1, 3, 7, 14 and 15; text, tables and signal labels are legible. | The finite XT Xebec adapter protocol: byte stream/DCB, type table, command/status/reset/select/mask ports, DMA3/IRQ5/AEN wiring, option-ROM window and stated signal ordering. | It is not an ATA, WD1003 or ESDI source; it does not specify a complete mechanical MFM/drive-service model or license importing controller ROM/media. |

The approved source is an IBM publication containing the original dated adapter
chapter, not a third-party transcription. The asset root and temporary render
paths are intentionally not recorded here; the hash, publication identity and
printed/page correspondence make the review reproducible without committing a
machine-local path or protected material.

## Scan And OCR Disposition

The rendered prose and tables used by the following List 1 are clear enough
for direct reading: printed p. 1 confirms the eight-bit data stack, DMA and
IRQ5; p. 3 confirms the four printed drive types; p. 7 confirms the six-byte
DCB layout; p. 14 confirms `320h`--`323h` and AEN; p. 15 confirms ROM,
IRQ5 and DRQ3/DACK3. Diagrams and physical-drive material are not promoted
from OCR or inferred. A List 1 row may use a direct printed fact/formula as
Manual L3; a missing physical/electrical value remains an explicit lower-level
or unsupported boundary, never a guessed one.

## Finite Source Universe For List 1

| Source family | Primary basis | T494 List-1 disposition |
| --- | --- | --- |
| Host protocol and data shape | Printed pp. 1, 4, 7, 10--16 | Six-byte DCB, byte-stream results/sense and finite command families are source-qualified. |
| Ports and board wiring | Printed pp. 14--15 | `320h`--`323h`, AEN, DMA3, IRQ5 and the `C8000h`--`C9FFFh` option window are source-qualified. |
| Logical media selection | Printed pp. 2--3 | Types 1, 2, 13 and 16 and their printed geometry/control values are source-qualified; profile selection remains an immutable construction input. |
| Service and physical media | Printed pp. 4--16 | Physical ECC/MFM, index/ready/seek signalling, long sectors, diagnostics and any unsourced service deadline are not admitted as a logical-image claim. |

## Cross-Reference Boundary

Earlier T479/T484 records are retained inventory, not a substitute for this
S1 review. Read-only emulator implementations may later corroborate one
already-qualified row as `Other L3`, but cannot supply undocumented status
bits, timing constants, geometry, ROM, firmware, or the normative contract.
No third-party source, firmware, guest media or implementation text is
imported by S1.

## S1 Conclusion

The primary source basis is complete enough to freeze the finite Xebec List 1
in S2. It supports the selected adapter's logical controller protocol and
board wiring at Manual L3 where the printed material is direct. It does not
authorize an ATA compatibility path, a second HDC/media owner, a controller
ROM import, or inferred physical timing.
