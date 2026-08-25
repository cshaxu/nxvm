# M5 T468 Core HDC ATA Phase Contract

## Active Record

T468 evaluates a bounded generic ATA-3 PIO task-file and logical-media phase
contract through the existing Core HDC owner. It explicitly excludes IBM
MFM/ST-506 and selected-machine fixed-disk claims.

## S1 Accepted Evidence

Checklist 1 retains the frozen fifteen-row `ATA-R1`--`ATA-R5`,
`ATA-F1`--`ATA-F5`, `ATA-T1`--`ATA-T5` universe. Its normative source is the
rendered, born-digital X3T13 ATA-3 2008D Rev. 7b manual, with its archived
SHA-256 recorded in the task proposal and checklist. Rendered pages establish
the Status/Alternate-Status interrupt distinction, SRST/nIEN, Command-write
semantics and the IDENTIFY 256-word PIO lifecycle; extracted text was used
only to locate pages.

86Box `4fef696`, local Bochs 2.6 compatibility source, current QEMU IDE core,
and MAME's selected-controller integration corroborate task-file, PIO and
board-binding boundaries. PCjs has no ATA/IDE controller in the checked
revision. None selects an NXVM controller, device identity, media backing or
timing value. Accordingly manual interface facts are Manual L3; T1--T4 stay
L2 until S2 proves a source-qualified board/device/media input and its one
existing HDC consumer. A verified receiver promotes its supplied term to Board
L3 without creating a second owner. T5 prohibits ATA-to-MFM substitution. No
runtime code, ABI, source import or product artifact changed.

## S2 Accepted Evidence

Checklist 2 traces every inherited ID exactly once through the existing HDC,
media, PIC, timeline, plan and profile paths. It confirms one ownership chain
and identifies two finite repairs only: S3 gates generic task-file writes while
BSY/DRQ is active, preserves an interrupt condition across nIEN and restores
the ATA logical reset signature; S4 removes the erroneous Data-Out dependency
on a successful prior media read by sharing address validation with the one
media owner. Neither repair chooses a controller, device identity, image
grammar, DMA path or timing number. All remaining L2 boundaries have their
existing or named future receiver in the audit.
