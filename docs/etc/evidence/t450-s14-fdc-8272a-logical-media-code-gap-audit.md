# T450 S14 Intel 8272A FDC And Logical-Media Current-Code Gap Audit

## Scope And Reading Rule

This is the code-side audit of the fifteen-row Intel 8272A checklist. Intel
defines the selected chip; IBM AT defines board facts; 86Box, MAME, PCjs,
Bochs and QEMU are comparison evidence only. Their uPD765, enhanced,
limited-subset and later-controller paths are not requirements for
`CORE_MACHINE_DEVICE_FDC`.

`Implemented` means behavior exists in the single `core/machine/fdc` owner,
not electrical media or board timing. Every incomplete row names its existing
receiver; no row authorizes a second controller, parser, scheduler or DMA/IRQ
route.

## Row Dispositions

| ID | Current code and proof | Accurate disposition and receiver |
| --- | --- | --- |
| FDC-R1 | `fdc.c` owns MSR, data queues, DOR/DIR/CCR and the sole DOR reset helper. `core-machine-fdc-smoke` proves reset preserves SRT/HUT/HLT and drains reset Sense state (`M5:T465:S2:FDC-reset:OK`). | Implemented Manual L3 phase/reset retention. Reserved board details are board L3 only if selected, otherwise fallback to L2. |
| FDC-R2 | `core_machine_fdc_command_length`/`core_machine_fdc_execute` contain fifteen Intel forms and one invalid route. Smoke proves `10h -> 80h`, no IRQ and command-phase return (`M5:T465:S3:FDC-8272-command:OK`). | Implemented Manual L3. Version, Read Diagnostic and vendor extensions fallback to L2 unless a future immutable controller contract selects them. |
| FDC-R3 | Sole transfer path carries DMA/non-DMA bytes, result status, deleted marks and format. It accepts N=2/512-byte logical sectors, bounded rates and provider geometry; FDC smoke and `vm-fdc-read-track-dos-smoke` cover it. | Manual L3 shape implemented. FM form, arbitrary N, raw CRC/gaps and CHRN/error grammar are gaps at the Core media receiver: fallback to L2 pending one board/media contract. |
| FDC-R4 | Specify stores fields; reset preserves SRT/HUT/HLT. Per-drive Seek/Recalibrate state records target/due/busy and queues completion for the existing Sense command; `M5:T465:S5:FDC-parallel-seek:OK` proves two interleaved drives. | Implemented Manual L3 fields/order/parallel completion. Fixed tick conversion remains fallback to L2. |
| FDC-R5 | Public identity is Intel 8272A and `10h` is invalid as required. | Implemented Manual L3. No uPD765 selector is needed; non-selected identity behavior fallback to L2. |
| FDC-F1 | Logical bindings provide four units, ready/change/write-protect/geometry and DOR selection. Physical INDEX, STEP/DIR, head-load, raw read/write and precompensation are absent. | Manual L3 relation is not fully represented. Receiver: existing drive/media boundary; fallback to L2 until selected board drive signals exist. |
| FDC-F2 | One DMA provider binds through `core_machine_configure_dma`; TC completes transfer; one PIC source publishes IRQ; non-DMA gating remains. FDC/DMA-boundary smokes cover it. | Implemented Manual L3 relation. ISA arbitration/service timing is owned outside FDC and fallback to L2. |
| FDC-F3 | `CORE_MACHINE_FDC_500K_BYTE_TICKS` and `CORE_MACHINE_FDC_SEEK_TRACK_TICKS` are deterministic constants, not Intel-value conversions. | Chip values are Manual L3, but Core-clock conversion is a gap at the FDC timing receiver: fallback to L2 pending board inputs. |
| FDC-F4 | Source has per-drive cylinders, target/due/busy state and one bounded completion FIFO consumed only by Sense Interrupt Status. | Implemented Manual L3 logical parallel seek. Mechanics remain fallback to L2 until board drive timing exists. |
| FDC-F5 | Command bytes retain MF/MT/SK; media exposes logical geometry/address marks. FM/MFM stream encoding, gaps, rotation, CRC and complete format are absent. | Manual L3 capability bounded by missing grammar. Receiver: Core media mapping; fallback to L2 until one grammar is admitted. |
| FDC-T1 | PC/AT and Model-40 compose 03F0--03F7, IRQ6 and DMA2; authority smokes cover binding. | Implemented Manual L3 topology. PIC/DMA phase stays at existing owners; fallback to L2 where no board phase is selected. |
| FDC-T2 | Profiles write CMOS floppy type while Core consumes media binding/geometry. | IBM category fact is present; category-to-mechanics mapping fallback to L2 at profile/drive boundary. |
| FDC-T3 | DOR motor affects Core readiness/cancellation; VM owns images. No BIOS policy is controller law. | Correct fallback to L2; no Core receiver. |
| FDC-T4 | `media_interface` is the sole frozen provider registry and raw-image adapter. | Correct single path but no selected grammar: fallback to L2 at that receiver. |
| FDC-T5 | FDC advances through Core scheduler; board owns DMA/PIC wiring; VM composes frozen topology. | Correct ownership. Deadline/readiness/motor/rotation fallback to L2 until selected; no new scheduler. |

## Outcome

Every source row appears once. This re-audit removes the earlier uPD765-led
selection: only Intel 8272A rules are Manual L3. Board-validated timing
conversion remains the only timing boundary; unsourced physical/media/firmware
details truthfully fallback to L2.
