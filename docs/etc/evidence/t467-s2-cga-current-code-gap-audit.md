# T467 S2 IBM CGA Current-Code Gap Audit

## Method and vocabulary

This audit consumes exactly the 17 frozen IDs in [Checklist 1](../research/t467-s1-cga-function-timing-checklist.md).
Paths were traced from profile composition to the one Core VADP owner and then
to its copied display snapshot.  `Board L3 capability` means that the existing
construction input can carry source-qualified board data into that owner; it
does **not** require the presently selected profile value to be qualified.
`Current data L2` is consequently a property of the selected value, not a
downgrade of that capability.  `L2` below is retained only where no complete
source-qualified input/consumer contract currently exists.

## Checklist 2: source-to-owner disposition

| ID | Current owner and path | Existing proof | Gap or retained receiver |
| --- | --- | --- | --- |
| CGA-R1 | `vadp.c` owns 3D4/3D5 selection, masks and CRTC state; `core_machine_vadp_register_cga_ports()` registers the sole route. | `core-machine-vadp-text-smoke`; port routes in `pc_at_profile.c`. | S3 must compare every selected 6845 read/write class and reset value with the rendered Motorola source. |
| CGA-R2 | `vadp.c:core_machine_vadp_write_mode()` owns 3D8 state. | CGA graphics port smokes. | It currently admits only the historical 640 mode value `1Ah` and does not model output-disable in capture; S3/S4 must close the complete six-bit grammar. |
| CGA-R3 | `vadp.c:core_machine_vadp_write_color()` owns 3D9; snapshot palette derives from it. | 320/640 CGA port smokes. | S4 must prove every selected border/palette effect rather than only retained graphics cases. |
| CGA-R4 | `core_machine_vadp_status()` is the sole 3DA reader and uses VADP raster phase. | `core-machine-vadp-text-status-smoke`. | Logical bit meaning exists; phase is only as qualified as CGA-T1/T2's board input. S5 owns the exact source-qualified conversion. |
| CGA-R5 | Generic CGA has no 3DB/3DC registration or R16/R17 state; Compaq light-pen state is adapter-specific and cannot be reused. | Generic/Compaq port-route tests distinguish the personalities. | Manual-L3 register/interface gap; S3 must either implement selected IBM-CGA latch input through VADP or transfer it with a source-qualified profile input contract. |
| CGA-F1 | VADP owns B8000 text capture, start/cursor words and copied text snapshot. | `core-machine-vadp-text-smoke`; firmware/default-profile smokes. | S4/S6 must prove 40/80 attribute and mode-select combinations; no second text state exists. |
| CGA-F2 | VADP owns B8000 16-KiB reads and 320x200 even/odd-bank decoding into the copied snapshot. | `core-machine-cga-graphics-port-smoke`; VM CGA graphics smokes. | S4 must reconcile all source palette/mode combinations and output disable. |
| CGA-F3 | VADP owns 640x200 even/odd-bank decoding into the copied snapshot. | `core-machine-cga-640-port-smoke`; VM 640 graphics smoke. | S4 must replace the single-value mode admission with the sourced grammar and prove it. |
| CGA-F4 | No 160x100 snapshot kind or sourced programming tuple exists. | None; Checklist 1 confines the source fact to existence/boundary. | L2: neither source-complete rule nor Board-L3 input/consumer exists. Keep it explicit; do not infer a renderer mode. |
| CGA-F5 | `core_machine_vadp_configure_cga_memory()` registers the sole B8000h 16-KiB provider; VADP owns its backing bytes. | CGA memory/graphics port smokes. | S4/S6 must complete profile and guest-path aperture proof; no ownership gap is found. |
| CGA-F6 | CRTC state drives text geometry and VADP's logical raster period. | Text/status smokes exercise CRTC writes and snapshot capture. | The CRTC relation is Manual L3; its conversion into Core elapsed ticks is CGA-T2's Board-L3 capability/current-data distinction, owned by S5. |
| CGA-F7 | 3D8 writes remain in VADP and snapshots are captured only through it. | Graphics port smokes cover mode writes. | Output-enable is not yet capture suppression and programming-order proof is absent; S3/S4 must repair it without a second mode path. |
| CGA-T1 | `core_machine_configure_display()` copies `core_machine_display_config.text_timing` once into VADP; session composition copies `profile->cga_text_timing`; `machine_scheduler.c` advances VADP from Core time. | Default-profile smoke fixes the current 48/8/8 values; text-status smoke covers VADP phase. | **Board L3 capability exists now.** The selected 48/8/8 values have no recorded source provenance and remain Current data L2; S5 supplies qualified values/provenance, not another setter. |
| CGA-T2 | VADP accepts the copied timing declaration and retains it as its only timing state. | Display configuration and text-status paths. | **Board L3 capability exists now.** No source-qualified mapping from Core elapsed ticks to CRTC character/raster units or ISA wait formula is selected. S5 may use a qualified construction plan; otherwise only that missing mapping remains L2. |
| CGA-T3 | No selected IBM-CGA light-pen input, cancellation or consumer path is present. | The isolated Compaq latch proves this is not a shared input route. | L2 until an admitted profile supplies the complete source-qualified external input lifecycle; S3 is the earliest receiver. |
| CGA-T4 | VADP publishes copied digital snapshots; VM/platform presents them. | Display capture/presentation tests and VADP owner review. | L2 for electrical/composite/monitor waveform and host presentation cadence: no source-complete consumer contract is present, and no renderer state may be added here. |
| CGA-T5 | Guest port/memory writes enter VADP; VADP owns mutable state and publishes copied snapshots; VM consumes them. | `machine_display.c`, VADP capture and all listed CGA smoke paths. | Accepted architecture boundary. S3--S6 must retain it; no duplicate state or renderer receiver is allowed. |

## Result and next implementation batch

The source and code universe is complete: all 17 frozen rows have one current
owner/proof disposition.  CGA-T1 and CGA-T2 are **Board-L3-capable now**;
their default profile values are merely unqualified current data.  The actual
Manual-L3 repair batch is finite: S3 (CRTC/ports/reset/light-pen disposition),
S4 (mode/output, memory/layout/palette), S5 (qualified board timing plan) and
S6 (profile/firmware/guest proof).  CGA-F4, the absent CGA light-pen lifecycle
and physical monitor output remain explicit L2 because no complete external
input and Core consumer path exists.  None may be promoted by an arbitrary
ratio or a presentation-side mirror.
