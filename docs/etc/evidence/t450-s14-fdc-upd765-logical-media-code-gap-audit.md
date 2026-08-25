# T450 S14 uPD765 FDC And Logical-Media Current-Code Gap Audit

## Scope And Method

This audit is the code-side companion to the frozen fifteen-row S13 source
universe.  It inspected the sole controller owner (`core/machine/fdc`), the
frozen profile-neutral media registry, board DMA/PIC binding, scheduler route,
VM media provider/composition and the focused FDC tests.  It makes no source,
test, media or runtime change.

`Implemented` below means the named observable is present in the current
owner; it does not turn a bounded logical model into an electrical-drive or
board-timing claim.  Each incomplete, unallocated, or source-bounded row has
the one receiver **M5 Core uPD765 And Logical Media Phase Contract**.  That
receiver must retain the existing single Core controller/media path rather
than add a profile-local parser, scheduler, or DMA/IRQ path.

T465 S1 reread the rendered NEC original rather than relying on its OCR text:
its pin/RESET page confirms physical signal relations and the 4/8-MHz clock;
Table 4 confirms uPD765B Version `90h` and uPD765A/A-2 `80h`. T465 S3 then
corrected the selected-device authority: `CORE_MACHINE_DEVICE_FDC` is Intel
8272A-compatible, whose original command table makes `10h` invalid with ST0
`80h`; NEC Version is conditional, not a current command gap. 86Box, MAME,
PCjs, Bochs and QEMU corroborate only labelled observations; QEMU's 82078
model cannot define a uPD765/AT requirement. The following disposition states
the exact source tier for every row and the one receiver for every gap.

## Row Dispositions

| S13 ID | Current owner and observed behavior | Focused proof | Disposition and sole receiver |
| --- | --- | --- | --- |
| FDC-R1 | `fdc.c` owns `core_machine_fdc_msr`, command/result queues, DOR/DIR/CCR data and one DOR reset helper. That helper preserves Specify SRT/HUT/HLT across both reset edges; port topology binds 03F2/03F4/03F5/03F7 once. | `core-machine-fdc-smoke` prints `M5:T465:S2:FDC-reset:OK` after both reset edges and existing reset-Sense drain; topology smoke exercises ports. | Manual L3 for declared ports/phases and specified reset preservation. Exact reserved bits remain fallback to L2 pending a selected chip/board register contract; one FDC receiver. |
| FDC-R2 | `core_machine_fdc_command_length`/`core_machine_fdc_execute` implement all fifteen selected 8272 command forms: Specify, Sense, Recalibrate, Seek, Read ID, read/write/deleted, Scan, Format and bounded Read Track. `10h` uses the sole invalid-command route. | `core-machine-fdc-smoke` proves retained read/write/deleted/scan/format and `10h -> 80h`, no IRQ and command-phase return. | Manual L3 selected command/invalid dispatch. NEC-only Version/Read Diagnostic requires immutable controller/media selection for Other/board L3; otherwise fallback to L2. No second dispatch path. |
| FDC-R3 | `core_machine_fdc_start_transfer`, DMA provider and non-DMA byte path carry results and sector data; deleted address marks and format callbacks are modeled. The path constrains sectors to N=2/512 bytes, limited CCR values and logical CHS geometry. | `core-machine-fdc-smoke`; `vm-fdc-read-track-dos-smoke` exercises read-track/DMA through a guest. | Manual L3 for chip transfer/result phase; the selected media/CHRN error mapping is Other/board L3 only when admitted, otherwise fallback to L2. One receiver owns both. |
| FDC-R4 | Specify stores SRT/HUT/HLT/ND; Seek/Recalibrate schedule fixed `CORE_MACHINE_FDC_SEEK_TRACK_TICKS`; Sense Interrupt drains reset/seek state; Sense Drive Status derives ready/write-protect/track-zero from media. | `core-machine-fdc-smoke`, `core-machine-fdc-media-change-port-smoke`, and topology smoke cover Sense/reset/seek state. | Manual L3 for register formula and logical completion. Selected physical drive motion/head timing is Other/board L3 only when supplied, otherwise fallback to L2; do not encode it in VM. |
| FDC-R5 | The selected data model names Intel 8272A and its `10h` path is the existing one-byte invalid ST0 `80h` result; deleted/scan stay on that controller's one command path. No uPD765A/B variant is selected. | `core-machine-fdc-smoke` proves `10h -> 80h`, no IRQ and command-phase return; no A/B overrun test can exist without a selected variant. | Manual L3 for selected 8272 invalid result. uPD765A/B Version/DRQ-overrun is Other/board L3 only when a future immutable configuration names that model, otherwise fallback to L2. |
| FDC-F1 | `core_machine_fdc_drive_bindings` and media query represent four logical drives, READY/change/write-protect/geometry; DOR bits select the binding. INDEX, STEP/DIR, head-load, raw RDATA/WINDOW, write-data and precompensation lack a physical-drive owner. | Topology and media-change smokes prove logical selection, ready/change and reset; no pin-level proof exists. | Manual L3 pin relations; selected mechanics are Other/board L3 only when supplied, otherwise fallback to L2. |
| FDC-F2 | One DMA provider binds through `core_machine_configure_dma`; request assert/deassert reaches DMA2, terminal count completes transfer, and one PIC IRQ source signals completion/Sense. Non-DMA byte gating remains. | `core-machine-fdc-smoke`, `vm-fdc-dma-boundary-smoke`, `vm-fdc-authority-smoke`. | Manual L3 logical DRQ/DACK/TC/INT relation; board bus service phase is fallback to L2. |
| FDC-F3 | `CORE_MACHINE_FDC_500K_BYTE_TICKS` and CCR acceptance provide deterministic byte pacing, but do not derive it from the documented 4/8-MHz clock or accepted broader rate/encoding cases. | `vm-fdc-read-track-dos-smoke` compares deterministic execution quanta; no selected-clock formula proof exists. | Manual L3 for stated clock facts. Conversion to a Core deadline is Other/board L3 only when selected inputs provide it, otherwise fallback to L2. |
| FDC-F4 | Per-drive cylinder state and pending seek retain a seek state while another controller phase may be active. No selected physical-drive model proves full parallel-seek timing/interleavings. | Reset/Sense and selected-drive tests only. | Manual L3 logical parallel-seek state; mechanics are Other/board L3 only when supplied, otherwise fallback to L2. |
| FDC-F5 | Command bytes expose MF/MT/SK and the logical provider exposes geometry/address marks. FM/MFM encoding, gaps, index/rotation, CRC and exact media format are absent. | `core-machine-fdc-smoke` proves deleted marks and logical sectors only. | Manual L3 chip capability; selected grammar is Other/board L3 only when admitted, otherwise fallback to L2. Retain one media mapping receiver. |
| FDC-T1 | Default PC/AT and Model-40 composition validate 03F0--03F7, IRQ6 and DMA2. | `vm-fdc-authority-smoke`, `vm-fdc-port-smoke`, `vm-fdc-dma-boundary-smoke`. | Manual L3 topology. PIC/DMA service phase remains fallback to L2 at its existing owners. |
| FDC-T2 | Default PC/AT writes profile `cmos.floppy_type`; Model-40 selects its FDD profile. Core FDC consumes media binding/geometry, not CMOS categories. | `vm-default-pc-at-apply-smoke`, `vm-model40-fdd-s18-smoke`. | Manual L3 for declared CMOS categories; exact drive mechanics/mapping is fallback to L2 until an admitted drive source provides Other/board L3. |
| FDC-T3 | Core DOR motor bits influence selected-drive readiness/execution cancellation; VM owns image/provider lifecycle. BIOS motor/retry/parameter/timeout policy is not an admitted contract. | `vm-fdc-read-track-dos-smoke` observes a guest path but does not establish BIOS policy. | fallback to L2: the manuals do not select the policy. Keep it outside Core FDC. |
| FDC-T4 | `media_interface` supplies one frozen provider registry, geometry, removable/change/read-only/address-mark operations; raw-image adapts admitted geometry. No complete selected image grammar maps flux/CHRN/gaps/rate to commands. | `core-machine-media-provider-smoke`, `vm-media-provider-smoke`, Model-40 FDD smoke. | Other/board L3 only after a selected grammar; otherwise fallback to L2. Do not add a parallel image parser. |
| FDC-T5 | Scheduler calls `core_machine_fdc_advance_at`/`refresh`; board owns DMA wiring; PIC owns IRQ delivery; VM submits frozen topology and observes results. | `vm-fdc-authority-smoke`, `vm-fdc-dma-boundary-smoke`, `vm-fdc-read-track-dos-smoke`. | Other/board L3 only when readiness/motor/rotation timing inputs are selected; otherwise fallback to L2. Existing owner direction remains conforming. |

## Completeness, Minimality And Transfer

All `FDC-R1`--`FDC-R5`, `FDC-F1`--`FDC-F5` and `FDC-T1`--`FDC-T5` rows are
present exactly once.  The audit deliberately does not mistake the existing
logical-sector implementation for raw magnetic media, electrical timing, or a
chosen uPD765 revision.  Conversely, it does not propose a second controller,
media registry, scheduler, command parser, or profile-local DMA/IRQ route:
the existing Core path is the sole production path to complete.

No build was required because S14 changes documentation only.  P review must
run documentation governance and `git diff --check`; later runtime work must
prove each adopted manual term through focused owner tests before claiming L3
or L2 closure.
