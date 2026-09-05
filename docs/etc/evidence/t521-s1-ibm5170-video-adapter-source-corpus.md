# T521 S1 IBM 5170 Video-Adapter Source Corpus (List 1)

`M5:T521:S1:IBM5170-VIDEO-CORPUS:OK`

## Method And Source Boundary

This is the finite source corpus for the configurations that may be attached
to the IBM 5170 Model 339. It does not turn a system-board BIOS, a host
renderer bitmap, or a later VGA card into another adapter. It also does not
admit a firmware byte merely because a local research checkout contains it.

The original scans remain owner-managed and read-only beneath
`../nxvm-assets/manuals`; no manual or firmware byte is copied into this
repository. Direct page rendering was used for the decisive CGA character-ROM,
EGA character-ROM/BIOS, and 5170 adapter-ROM passages. OCR was used only to
locate pages and was checked against the rendering.

| ID | Source and archive identity | Form and direct visual check | Usable facts |
| --- | --- | --- | --- |
| M1 | IBM, *Technical Reference: IBM Personal Computer AT*, first edition, Mar. 1984; `platform/ibm-5170-technical-reference-mar1984.pdf`; SHA-256 `F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D` | Paper-capture scan with OCR; rendered system-BIOS p. 5-11 (PDF p. 153) checked. | POST scans C8000h--E0000h in 2-KiB units for `55h AAh`; byte 2 gives 512-byte blocks; execution enters by far call; all bytes sum to zero modulo 256. |
| M2 | IBM, *Technical Reference: Options and Adapters, Volume 2*, revised Apr. 1984; `platform/ibm-pc-options-adapters-technical-reference-vol2-apr1984.pdf`; SHA-256 `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F` | Paper-capture scan with OCR; rendered CGA p. 5 (PDF p. 46) and EGA p. 1 (PDF p. 84) checked. | IBM MDA, CGA and EGA board topology, register/aperture and character-generator facts. |
| M3 | Video Seven, *V7 VGA Technical Reference Manual*, 30 Jun. 1988; `v7-vga-technical-reference-manual-jun88.pdf`; SHA-256 `1E702B0B450A90CEB43220C5C5AD7DA48D124DA76D70C4EAAB35AC1D9E38FFC8` | Paper-capture scan with OCR; rendered contents page (PDF p. 5) checked. | A later, vendor-specific VGA register/BIOS/font reference; not an IBM 5170 original-adapter source. |

The `Paper Capture` producer, rendered typography and OCR errors make M1--M3
scan/OCR sources, not authoritative text layers. Cited values below come only
from visually checked pages or the earlier visually checked finite IBM CGA/EGA
ledgers cited in each row.

## Configuration Corpus

| Row | Candidate identity and relation to IBM 5170 | Manual-defined board facts | Firmware / INT 10h / character-generator disposition | Source tier and admission result |
| --- | --- | --- | --- | --- |
| V1 | 5170 system board plus an installed display adapter. The board is the host, not a display adapter. | M1 gives the exact Option-ROM search/validity protocol. | System BIOS establishes default vectors, then may call a valid adapter ROM. M1 does not identify a particular installed display card or prove a generic in-code INT 10h provider. | **Manual-L3** for scan protocol. No selected video configuration yet. |
| V2 | IBM Monochrome Display and Printer Adapter (MDA), optional ISA adapter; not an intrinsic 5170 display device. | M2 MDA pp. 1--9: 4 KiB display buffer at B0000h, 6845 ports and an 8-KiB character generator for 256 codes. | M2 describes an adapter character generator but does not give S1 a source-qualified external ROM identity or an adapter Option-ROM/INT 10h ownership claim. | **Manual-L3** for topology/register/font-hardware facts. **Unsupported for product admission** until a lawful, source-qualified character-generator asset and firmware ownership are supplied. |
| V3 | IBM Color/Graphics Monitor Adapter (CGA), optional ISA adapter; compatible with the 5170 bus but not original system-board video. | M2 CGA pp. 1--25: 16 KiB B8000h display storage, 6845/3D8h/3D9h/3DAh interface, and an 8-KiB ROS character generator. The ROS is not program-readable or writable; its two selected fonts are jumper-selected. | M1 permits a valid adapter ROM where one exists, but M2's CGA chapter does not establish an adapter Option-ROM. The 5170 system BIOS listing may contain display services; this source corpus does not permit an unnamed generic Core/VM INT 10h replacement. | **Manual-L3** for CGA hardware facts. **Unsupported for product admission** until system-BIOS versus adapter firmware ownership and a lawful, source-qualified character-generator asset are proved. |
| V4 | IBM Enhanced Graphics Adapter (EGA), optional ISA adapter; a valid period-correct expansion, but not system-board video. | M2 EGA pp. 1--4 and pp. 12--68: four-plane storage/register architecture, selectable aperture/ports, adapter BIOS, and two on-adapter ROM character generators for 256 characters. EGA topology includes its 16-KiB ROM at C0000h. | The adapter BIOS is linked with the system BIOS and owns its video service initialization; EGA character data is adapter state, not renderer state. The existing EGA function ledger identifies source-defined planar/attribute behavior. | **Manual-L3** for IBM EGA topology, firmware and character-generator facts. **Unsupported for product admission**: no IBM EGA firmware/character-generator asset has been provenance-admitted for the 5170 profile. |
| V5 | Video Seven V7 VGA, a 1988 later ISA VGA expansion research candidate. It is neither the original 1984 IBM 5170 configuration nor IBM CGA/EGA. | M3 documents a vendor-specific VGA chip, VGA BIOS and font behavior. | A local SoftPC/86Box research corpus contains V7-like ROM candidates, but their provenance/licensing is not admitted and their bytes are not a product input. | **Manual-L3 only for a future specifically identified V7 card's documented semantics**; **not admitted** for IBM 5170 or current product YAML. No Other-L2 reference is promoted into a configuration. |
| V6 | Current `assets/default-cp437-8x16.bin`, SHA-256 `C9631BDB9FB00E02907CE4BCEAACD5E33C3D70D431CDD92309635218A69C6337`, 4096 bytes. | No manufacturer, adapter identity, original ROM dump provenance or manual locator is recorded. Its size does not establish either the 8-KiB MDA/CGA ROS or the distinct IBM EGA generators. | It is a repository-local renderer input, not a source-qualified adapter character-generator ROM, Option ROM or INT 10h implementation. | **Source-unallocated; not L1/L2/L3 and not admissible as production hardware.** S3 must remove this path only as part of an admitted VADP-owned replacement, never by inventing a second font path. |

## Existing Evidence And External Reference Reconciliation

`t467-s1-cga-function-timing-checklist.md` and
`t450-s15-vadp-function-timing-checklist.md` already preserve the complete
manual-first CGA and IBM-EGA register/function universes. Their local 86Box,
PCjs, Bochs, MAME and QEMU observations remain bounded corroboration: they
cannot prove an IBM board's ROM identity, font bytes, system-BIOS ownership or
an exact L3 timing value. No source from any of those projects is imported.

The S1 claim sweep searched all tracked production source, tests, session
YAML, proposals, history and current evidence for `5170`, `CGA`, `EGA`,
`VGA`, `Option ROM`, `INT 10` and `font`:

| Claim class | Current disposition | S1 result |
| --- | --- | --- |
| VADP ports, VRAM, modes and copied snapshot | Frozen manual-led CGA/EGA ledgers and T507 VADP audit. | Retained. S1 adds no second guest state or renderer path. |
| 5170 and Default-PC/AT profile in-code `VIDEO_INT10` service registration | A current profile firmware-service declaration, not an asset identity. | S2 must determine exact callers/mutation and assign it to a source-backed system-BIOS route or delete it. |
| YAML `firmware.video` | The current 5170 YAML is explicit `null`; no adapter ROM is silently mapped. | Retained until S2/S3. No new YAML field or fallback. |
| YAML `firmware.font` and Win32 CP437 loader | Every current profile references the repository-local V6 bitmap. | Identified as one duplicate, source-unallocated production path; no present configuration may call it a CGA/EGA ROM. |
| Local V7 and emulator ROM files | Read-only research material outside NXVM's product assets. | Not catalogued as a product manifest, copied, downloaded or selected. |

## S1 Decision And Transfer

The source corpus permits no new admitted IBM 5170 video configuration yet.
It proves a narrow architectural result instead: hardware font ROM, Option
ROM/BIOS and VADP guest state are distinct roles. The next S must audit the
current VADP, firmware, YAML and renderer call graph against that result before
any code or asset change. In particular, it must not replace V6 by another
unproven bitmap merely to preserve host display output.
