# T450 S14 uPD765 FDC And Logical-Media Current-Code Gap Audit

## Scope And Method

This audit is the code-side companion to the frozen fifteen-row S13 source
universe.  It inspected the sole controller owner (`core/machine/fdc`), the
frozen profile-neutral media registry, board DMA/PIC binding, scheduler route,
VM media provider/composition and the focused FDC tests.  It makes no source,
test, media or runtime change.

`Implemented` below means the named observable is present in the current
owner; it does not turn a bounded logical model into an electrical-drive or
board-timing claim.  Each incomplete, unallocated, or source-blocked row has
the one receiver **M5 Core uPD765 And Logical Media Phase Contract**.  That
receiver must retain the existing single Core controller/media path rather
than add a profile-local parser, scheduler, or DMA/IRQ path.

## Row Dispositions

| S13 ID | Current owner and observed behavior | Focused proof | Disposition and sole receiver |
| --- | --- | --- | --- |
| FDC-R1 | `fdc.c` owns `core_machine_fdc_msr`, command/result queues, DOR/DIR/CCR data and DOR reset; port topology binds 03F2/03F4/03F5/03F7 once. | `core-machine-fdc-smoke`, `core-machine-fdc-topology-port-smoke` exercise status/data/result and reset. | Implemented for the declared ports/phases; exact register reserved-bit and reset-parameter fidelity remains receiver work. |
| FDC-R2 | `core_machine_fdc_command_length`/`core_machine_fdc_execute` implement Specify, Sense, Recalibrate, Seek, Read ID, read/write/deleted, Scan, Format and a bounded Read Track.  Version and a distinct Read Diagnostic path fall through to `80h`; command modifier semantics are not complete. | `core-machine-fdc-smoke` covers the retained read/write/deleted/scan/format paths; no Version/diagnostic proof exists. | Partial.  Complete every admitted command form and documented invalid outcome in the one receiver. |
| FDC-R3 | `core_machine_fdc_start_transfer`, DMA provider and non-DMA byte path carry results and sector data; deleted address marks and format callbacks are modeled.  The path constrains sectors to N=2/512 bytes, limited CCR values and logical CHS geometry, so it is not a full FM/MFM/CHRN/error model. | `core-machine-fdc-smoke`; `vm-fdc-read-track-dos-smoke` exercises read-track/DMA through a guest. | Partial.  Receiver owns the missing command-field, result/error and physical-media interpretation as one phase contract. |
| FDC-R4 | Specify stores SRT/HUT/HLT/ND; Seek/Recalibrate schedule one fixed `CORE_MACHINE_FDC_SEEK_TRACK_TICKS`; Sense Interrupt drains reset/seek state; Sense Drive Status derives ready/write-protect/track-zero from media.  The manual SRT/HLT/HUT formula is not used to schedule physical motion or head load/unload. | `core-machine-fdc-smoke`, `core-machine-fdc-media-change-port-smoke`, and topology smoke cover Sense/reset/seek state. | Partial.  Receiver must derive selected timing only after selected drive facts exist; do not encode formulas in VM composition. |
| FDC-R5 | The data model names the device Intel 8272A and implements deleted/scan state, but has no selected uPD765A/B revision, Version response, or A/B DRQ-overrun behavior. | No revision/Version/overrun-variant proof found. | Missing.  Receiver selects a documented controller revision and supplies its behavioral tests. |
| FDC-F1 | `core_machine_fdc_drive_bindings` and media query represent up to four logical drives, READY/change/write-protect/geometry; DOR drive and motor bits select the current binding.  INDEX, STEP/DIR, head-load, raw RDATA/WINDOW, write-data and precompensation pins have no physical-drive owner. | Topology and media-change smokes prove logical selection, ready/change and reset; no pin-level proof exists. | Partial.  Receiver must either acquire selected drive facts and model them at the Core boundary or retain the physical-pin boundary explicitly. |
| FDC-F2 | The one DMA provider binds through `core_machine_configure_dma`; request assert/deassert reaches DMA2, terminal count completes the transfer, and the single PIC IRQ source is raised/deasserted by controller completion/Sense.  Non-DMA byte gating is retained. | `core-machine-fdc-smoke`, `vm-fdc-dma-boundary-smoke`, `vm-fdc-authority-smoke`. | Implemented for the logical Core transaction path; AT arbitration/service phase remains receiver work. |
| FDC-F3 | `CORE_MACHINE_FDC_500K_BYTE_TICKS` and CCR acceptance provide deterministic byte pacing, but do not derive it from the admitted 4/8 MHz or AC tables and reject broader rate/encoding cases. | `vm-fdc-read-track-dos-smoke` compares deterministic execution quanta; no AC/clock formula test exists. | Partial.  Receiver owns any selected-clock/AC-to-board timing contract; it must not claim unselected electrical values. |
| FDC-F4 | Per-drive cylinder state and pending seek allow the controller to retain a seek state while its phase model is otherwise active.  There is no proof or selected physical-drive model for full parallel-seek timing/interleavings. | Reset/Sense and selected-drive tests only. | Partial.  Receiver owns any full parallel-seek contract once drive facts are admitted. |
| FDC-F5 | Command bytes expose MF/MT/SK positions, and the logical provider exposes geometry/address marks.  FM/MFM encoding, gaps, index/rotation, CRC and exact selected media format are not represented. | `core-machine-fdc-smoke` proves deleted marks and logical sectors only. | Partial/blocked by selected media facts.  Receiver owns one selected logical-media grammar plus controller mapping. |
| FDC-T1 | Default PC/AT and Model-40 composition create a validated FDC topology with 03F0--03F7, IRQ6 and DMA2; board code rejects a mismatched binding. | `vm-fdc-authority-smoke`, `vm-fdc-port-smoke`, `vm-fdc-dma-boundary-smoke`. | Implemented topology.  Exact PIC/DMA bus service belongs to their controllers and the receiver only for cross-controller phase terms. |
| FDC-T2 | Default PC/AT session writes profile `cmos.floppy_type`; the selected Model-40 composition selects its FDD profile.  Current Core FDC consumes media binding/geometry, not CMOS categories directly. | `vm-default-pc-at-apply-smoke`, `vm-model40-fdd-s18-smoke`. | Partial board/firmware boundary.  Receiver must reconcile selected drive categories with an admitted drive source, without making Core own CMOS policy. |
| FDC-T3 | Core DOR motor bits only influence selected-drive readiness/execution cancellation; VM owns image/provider lifecycle.  No BIOS motor start/off, retry, parameter-table or timeout formula is implemented as a manual-selected contract. | `vm-fdc-read-track-dos-smoke` observes a guest path but does not establish BIOS policy. | Blocked by selected firmware/drive facts; receiver must keep guest policy outside FDC Core. |
| FDC-T4 | `media_interface` supplies one frozen provider registry, geometry, removable/change/read-only/address-mark operations; VM raw-image provider adapts admitted geometry.  No source-selected complete image grammar maps flux/CHRN/gaps/rate to commands. | `core-machine-media-provider-smoke`, `vm-media-provider-smoke`, Model-40 FDD smoke. | Blocked exactly as S13 records.  Receiver owns the selected grammar and error mapping, or retains the boundary; no parallel image parser. |
| FDC-T5 | Scheduler calls `core_machine_fdc_advance_at`/`refresh`; board owns DMA request wiring; PIC owns IRQ delivery; VM composition only submits frozen topology and observes terminal results.  This is a single directed owner path. | `vm-fdc-authority-smoke`, `vm-fdc-dma-boundary-smoke`, `vm-fdc-read-track-dos-smoke`. | Current ownership is conforming.  Unallocated readiness/motor/rotation and board-phase inputs transfer once to the receiver. |

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
