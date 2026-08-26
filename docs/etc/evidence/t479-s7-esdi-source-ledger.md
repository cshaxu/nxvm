# T479 S7 WD1007A-WAH ESDI Source Ledger

`M5:T479:S7:ESDI-SOURCE:OK`

## Source Quality And Scope

| Source | Quality and scope | Use |
| --- | --- | --- |
| Western Digital, *WD1007A-WAH/2 OEM Manual / Engineering Specification*, February 1988, [scan](https://bitsavers.computerhistory.org/pdf/westernDigital/pc_disk_controller/WD1007A_AT_ESDI_AM8753/WD1007-WAH_OEM_Manual_198802.pdf) | Primary 140-page scan with OCR. Register, command and control tables are used only where the printed text/table is legible; the document itself identifies some material as preliminary. | Normative controller-specific host contract. |
| 86Box, read-only `src/disk/hdc_esdi_at.c` | Other-L3 implementation observation of a distinct AT ESDI family. Its controller variants, timers and firmware assets are not evidence. | Cross-check only; nothing imported. |
| PCjs, Bochs, MAME, QEMU | No bounded implementation of this exact controller was inspected in this S. | No claim derives from absence. |

The selected unit is the **WD1007A-WAH hard-disk-only** controller, not the
WA2 floppy-inclusive variant and not a generic ESDI category.  The primary
manual requires a named drive table and optional controller BIOS; neither is a
license to select a runtime profile or import its ROM.

## Finite Host Contract

| ID | Required observable | Primary basis | Cross-model result | Level and S7 disposition |
| --- | --- | --- | --- | --- |
| E1 | WAH connects up to two ESDI drives to an IBM PC/AT-compatible 16-bit host; it is hard-disk-only. | pp. 2, 20, 63. | 86Box has a distinct AT ESDI family. | Manual L3 identity; needs a named profile. |
| E2 | Primary task-file is `1F0h`--`1F7h`, control/status is `3F6h`--`3F7h`; jumpers can select a secondary range. | pp. 32, 35, 107, 129. | 86Box retains an AT ESDI route. | Manual L3 port map, not ATA identity. |
| E3 | `1F0h` programmed transfers use 16-bit words; data pipeline/ring buffer permits controller/host concurrency. | pp. 38--39, 118. | External timing constants are not adopted. | Manual L3 transfer width; internal cache timing is not implemented. |
| E4 | Fixed-disk control includes reset and interrupt-disable behavior; reset requires at least 10 us and a disabled pending interrupt reappears on re-enable. | pp. 37--38, 117--118. | No exact-controller cross-model timing adopted. | Manual L3 relation; source physical time is not connected to a selected Core profile axis. |
| E5 | Standard command family is restore, seek, read/write sector, format track, read verify, diagnostics and set parameters. | pp. 48--49, 82. | 86Box distinguishes ESDI command/state logic. | Manual L3 grammar; no ATA alias. |
| E6 | Seek selects the task-file cylinder, completes with SKC and raises IRQ14 on successful ESDI transfer. | p. 49. | No primary-backed profile route exists. | Manual L3 controller behavior; no current receiver. |
| E7 | Read/write supports one through 256 sectors, implied seek and interrupt per ready sector; data may cross track/cylinder boundaries. | pp. 49, 82. | Controller cache behavior is model-specific. | Manual L3 logical transfer semantics; physical media/cache remains absent. |
| E8 | Read/write long, ECC correction/retries, error-sector transfer, cache control and ESDI-specific commands exist. | pp. 49, 69, 82--89. | No source-qualified logical-image mapping for these features. | Explicit unsupported physical/controller-firmware boundary. |
| E9 | Normal WD1007 mode forces a 10 MHz drive to 35 sectors/track; WD1005 replacement mode differs. | pp. 3, 33. | No selected drive/profile geometry exists. | Manual L3 mode fact; unavailable as configuration. |
| E10 | Optional controller ROM provides parameter, formatting and analysis functions, normally at `C8000h`--`C9FFFh`. | pp. 2, 9, 32. | 86Box needs its own ROM assets. | Manual L3 fact; future BYOB profile receiver only. |
| E11 | The manual is an AT installation/controller specification, not a profile definition for default-at, Model 339 or DeskPro Model 40. | pp. 2, 37--40 and existing profile declarations. | No model overrides this boundary. | No existing immutable receiver: transfer. |

## Cross-Validation Result

The primary document proves a distinct controller whose host register shape is
AT task-file-like but whose command, cache, ESDI-drive and firmware semantics
are controller-specific.  It therefore may share only already-proven Core
task-file mechanics after a selected profile admits it; it cannot be relabelled
as ATA, selected by `default-at`, or attached to the existing IBM/Compaq
profiles.  The absent receiver is a configuration admission gap, not a reason
to add a generic ESDI object.
