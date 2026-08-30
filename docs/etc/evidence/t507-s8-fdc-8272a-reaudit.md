# T507 S8 Intel 8272A FDC Re-Audit

`M5:T507:S8:FDC-8272A:P1`

## Sources and Method

The selected-controller authority is Intel *8272A Floppy Disk Controller,
Preliminary*, order `210608-001` (1982).  Its scanned original is archived as
`assets/manuals/controllers/intel/210608-001_8272A_Floppy_Disk_Controller_1982.pdf`;
acquired 2026-08-29; SHA-256
`C03A1FABE42FE43BB47FBD84042840F7B78B1DDD03E8A117FC7059C3453CC398`.
Rendered pp. 2, 3, 9, and 11 were read visually; OCR located pages only.  The
document is preliminary, so it controls only the relations it explicitly
states.

Read-only comparison covered 86Box `src/floppy/fdc.c`, Bochs
`iodev/floppy.cc`, and PCjs `machines/pcx86/modules/v2/fdc.js`.  All retain one
controller command/result state, route DRQ through DMA, cancel DRQ on terminal
completion, and enumerate four reset Sense-Interrupt reports.  They corroborate
but do not replace Intel.  MAME and QEMU FDC sources are not present in the
approved local corpus.  No external source was imported.

## List 1: Source Contract

| ID | Source-qualified relation | Level | Sole NXVM owner/receiver |
| --- | --- | --- | --- |
| F1 | RESET makes the FDC idle, drives FDD outputs low, and retains the last Specify command. | Manual L3 | `fdc.c` reset/cancel state. |
| F2 | The data/status selection, command/result phases, and 15-command controller contract are FDC behavior. | Manual L3 | `fdc.c` port handlers and FIFO state. |
| F3 | DRQ requests DMA, active-low DACK acknowledges it, and TC terminates a command execution phase. | Manual L3 | `fdc.c` typed DMA2 request binding and transfer completion. |
| F4 | DMA uses controller/DMA byte transfer; Non-DMA interrupts the processor for each byte transfer. | Manual L3 | `fdc.c` DMA and 3F5h execution branches. |
| F5 | The required byte-service bound is 31 us in FM and 15 us in MFM. | Manual L3 where a frozen board conversion is supplied; L2 otherwise | `core_machine_fdc_dma_byte_ticks`. |
| F6 | Seek/recalibrate operations may overlap on up to four drives; SRT is 1--16 ms in 1-ms increments. | Manual L3 where a frozen board conversion is supplied; L2 otherwise | FDC per-drive seek state and due ticks. |
| F7 | Recalibrate steps toward Track 0, reports Seek End, and reports Equipment Check after 77 unsuccessful steps. | Manual L3 causal relation | FDC seek completion/result queue. |
| F8 | Specify programs HUT 16--240 ms in 16-ms increments, HLT 2--254 ms in 2-ms increments, and SRT; values double at 4 MHz. | Manual L3 formula; L2 until the board supplies the relevant clock conversion | FDC Specify fields. |
| F9 | Ready changes, seek/recalibrate completion, normal result entry, and Non-DMA byte service are interrupt causes; Sense Interrupt Status identifies the applicable cause. | Manual L3 | FDC IRQ6/result and Sense-Interrupt path. |
| F10 | Controller drive selection has four logical units.  Four post-reset Sense-Interrupt reports are corroborated by 86Box, Bochs, and PCjs; the Intel pages do not state that exact post-reset report cardinality. | Other L3 | `reset_sense_mask` in the FDC owner. |
| F11 | Board drive presence, Track 0, Ready, write-protect, media change, wiring, and port decoding are board inputs rather than a second FDC state owner. | Manual L3 boundary; profile values vary | Frozen FDC topology/drive bindings. |
| F12 | Rotation, address-mark/CRC electronics, raw FM/MFM stream layout, and host media-change timing need selected drive/media/board evidence not supplied by this controller data sheet. | L2 | Existing logical-media provider boundary; no fabricated deadline. |

## List 2: Current Code and Repair Disposition

| ID | Current path and audit result | Disposition |
| --- | --- | --- |
| G1 | `fdc.c` solely owns command phase, registers, result FIFO, seek/reset/byte due ticks, DRQ and IRQ6 publication.  `machine_board.c` only validates the Core-internal DMA2 binding. | Retained one owner. |
| G2 | `machine_scheduler.c` queries the FDC's earliest due tick and advances that same owner at the selected Core due tick.  VM neither supplies FDC time nor holds a second queue. | Retained one scheduler path. |
| G3 | The former `CCR == 0` path scheduled every byte at 16 us, including FM commands.  This contradicted the Intel 31-us FM / 15-us MFM service limits. | **Repaired:** the one existing byte-gate formula now selects 31 or 15 us from the command MFM bit; no state or clock path was added. |
| G4 | Reset cancellation clears DRQ, pending byte gates and execution before the preserved Specify fields are restored.  TC completes through the same result/IRQ owner. | Retained Manual-L3 causal path. |
| G5 | The logical image path intentionally accepts only its declared 512-byte media grammar and does not claim raw floppy encoding, rotational position, CRC or mechanical timing. | Explicit L2 boundary, not a false L3 claim. |
| G6 | A zero `ticks_per_microsecond` has no source-qualified physical conversion.  It retains the Core's next-progression logical handoff rather than inventing a duration. | Explicit L2 fallback. |

## Proof and Simplicity

`core_machine_fdc_smoke` now asserts the 15-us MFM and 31-us FM paths, including
DMA and direct 3F5h service.  The repair replaces one wrong constant with the
manual-selected formula; it adds no device, profile mutation, media cache,
callback, VM timing input, or parallel completion route.  The required complete
repository-only unit replay passes 312/312.
