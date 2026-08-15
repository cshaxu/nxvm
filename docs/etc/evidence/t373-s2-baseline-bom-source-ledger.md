# T373 S2: Baseline-Machine BOM And Source Ledger

## Source Boundary

This ledger records configuration facts, not timing values. IBM and Compaq
technical references are the required primary-document classes. Preserved-host
pages and contemporary publications are used only to locate or cross-check
those documents; they are never implementation authorities. A field without a
machine-specific primary fact remains unknown and cannot be filled from the
generic NXVM PC/AT descriptor, an emulator configuration, a ROM, or guest
media.

## Qualified Source Set

| Source | Class and use | Admitted facts | Explicit non-use |
| --- | --- | --- | --- |
| IBM PC/AT Technical Reference, September 1985 and March 1986 | IBM primary board/reference manuals, already admitted by T366/T372. | Model 339 Type-3/8 MHz family, board topology, and selected Model-339 baseline fields. | No project-tick or device-duration scalar. |
| IBM PC/XT 5155/5160 Technical Reference, 6139821, 1986 edition; IBM PC/XT Guide to Operations, 6137861 | IBM primary technical/operations manuals preserved by the IBM-PC manual archive. | The required primary documents for 5160 board, ports, expansion, keyboard, storage and display-option qualification. | Does not by itself make every option a selected 5160-268 device. |
| IBM Personal System/2 and IBM PC Product Reference, version 4.0, April 1987 | IBM product-reference catalogue. | 5160-268 is listed as a 256 KB enhanced-keyboard system unit with one half-height diskette drive; it is a no-fixed-disk machine configuration. | No 5160 board timing or display-adapter selection. |
| COMPAQ DeskPro 386 Technical Reference Guide, Volumes I and II, September 1986, First Edition | Compaq primary technical-reference guide; a preserved copy is indexed by PCjs. | Required primary document for original DeskPro 386/16 board, ROM, keyboard, controller, drive, display and expansion qualification. | The PCjs emulator/configuration and its sources are not a DeskPro hardware oracle. |
| BYTE, November 1986, contemporary product description | Secondary cross-check only. | Model 40 is described as a 16 MHz 80386 system with 1 MB RAM, 128 KB ROM, one 1.2 MB 5.25-inch floppy and 40 MB fixed disk. | No board timing, controller interface, display selection, IRQ/DMA or ROM mapping fact. |

Stable source locations: [IBM 5160 manual index](https://www.ibm-pc.org/manuals/ibm/5160/5160.htm),
[IBM Product Reference](https://bitsavers.org/pdf/ibm/pc/PS2_and_IBM_PC_Product_Reference_Version_4.0_Apr87.pdf),
[DeskPro 386 technical-reference index](https://www.pcjs.org/machines/pcx86/compaq/deskpro386/),
and [BYTE November 1986](https://www.worldradiohistory.com/Archive-Byte/80s/Byte-1986-11.pdf).
No source, ROM, media, PDF, or derived data is imported into the repository.

## Machine BOM And Unknown-Field Matrix

| Field | IBM PC/AT 5170 Model 339/Type 3 | COMPAQ DeskPro 386 Model 40 (1986 original) | IBM PC/XT 5160-268 | Required later receiver |
| --- | --- | --- | --- | --- |
| Identity | Model 339/Type 3, selected by T366/T372. | Original DeskPro 386 Model 40, not later /20, /25, /s or 486 family. | 5160-268, not 5150 or a fixed-disk XT variant. | T373 S3 locks these identities in the final ledger. |
| CPU and nominal clock | 80286 at 8 MHz. | 80386 at 16 MHz. | 8088 family selected by owner; exact board/oscillator relation must be extracted from the IBM technical reference before timing work. | 5170 timing candidate; DeskPro CPU/profile audit; 8088 CPU profile and XT profile audit. |
| Base memory | 512 KB planar RAM. | 1 MB RAM in the Model 40 product configuration; expansion detail remains a later profile fact. | 256 KB system unit as catalogued; maximum/installed expansion is not selected. | Per-machine profile/capability audit. |
| Firmware | Rev.3 IBM slot; external owner-managed ROM only. | 128 KB system ROM is reported by contemporary documentation; exact ROM revision/slot mapping remains primary-reference work and external-only. | BIOS/ROM revision and mapping unknown in this S; external-only when later selected. | Per-machine profile audit and external-ROM admission boundary. |
| Keyboard/input | 101-key AT keyboard selected. AUX/mouse is not selected. | Exact shipped keyboard/controller and any pointing-device choice remain unknown. | Enhanced keyboard is part of 5160-268 product description; controller/revision and any mouse are unknown/unselected. | Per-machine profile audit; current-product input closure for retained AUX. |
| Diskette | FDC with 1.44 MB drive only as an aftermarket field upgrade; not factory configuration. | One 1.2 MB 5.25-inch double-sided floppy is listed for Model 40. | One half-height diskette drive is listed; density/type must come from IBM technical-reference/operations detail. | Per-machine functional/profile audit; never infer drive timing. |
| Fixed disk/storage | None; IBM MFM/ST-506 remains excluded TODO. | 40 MB fixed disk is a Model-40 product fact; controller/drive interface, IRQ/DMA, error and reset semantics remain unknown. | None in the 5160-268 product description. | DeskPro profile/capability audit; current ATA closure is not evidence for any of these paths. |
| Display | CGA selected; EGA absent. | No display adapter is selected by the locked Model-40 name alone. | No display adapter is selected by the 5160-268 system-unit description. | DeskPro/XT profile audits must choose a documented adapter/corpus; no default-PC/AT EGA or Model-339 CGA inheritance. |
| DMA/PIC/PIT/RTC/NMI/reset | T366--T372 establish selected logical topology; physical conversion remains open. | Board/chip/revision and route details unqualified here. | Board/chip/revision and route details unqualified here. | T373 S3 classification, then each machine's functional and timing candidates. |
| External ROM/media | IBM pair exists only externally and is not a repository dependency. | Required BIOS is external owner-managed only; no local asset identity recorded. | Required BIOS is external owner-managed only; no local asset identity recorded. | Later profile-local asset admission, never this ledger. |

## Decision And Transfers

The S2 result is sufficient to prevent three common false inferences:

1. the generic 80386/EGA/ATA PC/AT descriptor is not a DeskPro Model 40;
2. the Model-339 aftermarket 1.44 MB route does not turn ATA/HDC into IBM MFM;
3. a 5160-268 system unit does not select a display adapter, a fixed disk, or a
   generic PC/AT controller set.

S3 must merge this BOM with S1's actual code support and classify each row as
complete, partial, empty, test-only, unsupported, or explicitly removed. It
must name the earliest implementation owner and the later timing owner without
selecting an unknown field. A later per-machine profile audit may make an
explicit documented selection where the user has not fixed one.
