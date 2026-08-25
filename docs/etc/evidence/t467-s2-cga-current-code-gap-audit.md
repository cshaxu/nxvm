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
| CGA-R1 | `vadp.c` owns 3D4/3D5 selection, masks and CRTC state; `core_machine_vadp_register_cga_ports()` registers the sole route. | Text, CGA graphics/640 and port-route smokes. | Closed in S3: five-bit index, R0--R17 access classes and masks are one VADP grammar. |
| CGA-R2 | `vadp.c:core_machine_vadp_write_mode()` owns 3D8 state. | CGA graphics/640 port smokes. | Closed in S3/S4: one six-bit grammar replaces the `1Ah` special case and output-disable gates snapshots. |
| CGA-R3 | `vadp.c:core_machine_vadp_write_color()` owns 3D9; snapshot palette derives from it. | Graphics-port smoke covers both palette sets. | Closed in S4: the existing sole palette calculation consumes the intensity and palette-set bits. |
| CGA-R4 | `core_machine_vadp_status()` is the sole 3DA reader and uses VADP raster phase. | Text-status and Model-339 clock-contract smokes. | Logical status is Manual L3; elapsed phase uses the CGA-T1/T2 Board-L3-capable receiver and selected Other-L3 cadence only. |
| CGA-R5 | VADP registers 3DB/3DC as the selected generic CGA control addresses; Compaq light-pen state remains adapter-specific. | Generic/Compaq port-route and CGA port smokes. | Closed in S3 for register grammar: controls are address-accepted no-ops because no source-qualified IBM-CGA input lifecycle exists. That lifecycle remains CGA-T3 L2. |
| CGA-F1 | VADP owns B8000 text capture, start/cursor words and copied text snapshot. | Text and S6 system proofs. | Closed: reset and mode-03 firmware reach the same VADP owner; no text mirror exists. |
| CGA-F2 | VADP owns B8000 16-KiB reads and 320x200 even/odd-bank decoding into the copied snapshot. | Graphics-port, system and DOS smokes. | Closed in S4/S6: selected palette and output gating share this one capture route. |
| CGA-F3 | VADP owns 640x200 even/odd-bank decoding into the copied snapshot. | 640 port and system smokes. | Closed in S3/S4/S6: the six-bit grammar admits valid high-resolution combinations through the existing decoder. |
| CGA-F4 | No 160x100 snapshot kind or sourced programming tuple exists. | None; Checklist 1 confines the source fact to existence/boundary. | L2: neither source-complete rule nor Board-L3 input/consumer exists. Keep it explicit; do not infer a renderer mode. |
| CGA-F5 | `core_machine_vadp_configure_cga_memory()` registers the sole B8000h 16-KiB provider; VADP owns its backing bytes. | Memory/graphics-port and S6 system/DOS smokes. | Closed: profile, firmware and guest writes all reach this one provider; no ownership gap exists. |
| CGA-F6 | CRTC state drives text geometry and VADP's logical raster period. | Text/status and Model-339 clock-contract smokes. | Manual L3 logical relation; selected Model-339 elapsed conversion is Other L3 through the one Board-L3-capable receiver, while default data remains L2. |
| CGA-F7 | 3D8 writes remain in VADP and snapshots are captured only through it. | Graphics-port, system and DOS smokes. | Closed in S4: output-disable suppresses the copied output without another mode or frame path. |
| CGA-T1 | `core_machine_configure_display()` copies `core_machine_display_config.text_timing` once into VADP; session composition copies `profile->cga_text_timing`; `machine_scheduler.c` advances VADP from Core time. | Model-339 clock-contract and text-status smokes. | **Board-L3-capable receiver exists.** Model-339's qualified 86Box-derived ratio is Other L3; selected default 48/8/8 is Current data L2. |
| CGA-T2 | VADP accepts the copied timing declaration and retains it as its only timing state. | Display configuration, Model-339 clock-contract and text-status paths. | **Board-L3-capable receiver exists.** No IBM-selected Core-tick/ISA-wait formula exists; Model-339's bounded Other-L3 cadence does not promote default data or waits. |
| CGA-T3 | No selected IBM-CGA light-pen input, cancellation or consumer path is present. | The isolated Compaq latch proves this is not a shared input route. | L2 until an admitted profile supplies the complete source-qualified external input lifecycle; S3 is the earliest receiver. |
| CGA-T4 | VADP publishes copied digital snapshots; VM/platform presents them. | Display capture/presentation tests and VADP owner review. | L2 for electrical/composite/monitor waveform and host presentation cadence: no source-complete consumer contract is present, and no renderer state may be added here. |
| CGA-T5 | Guest port/memory writes enter VADP; VADP owns mutable state and publishes copied snapshots; VM consumes them. | `machine_display.c`, VADP capture and all listed CGA smoke paths. | Accepted architecture boundary. S3--S6 must retain it; no duplicate state or renderer receiver is allowed. |

## Result and next implementation batch

The source and code universe is complete: all 17 frozen rows have one final
owner/proof disposition. CGA-T1 and CGA-T2 are **Board-L3-capable now**;
Model-339 carries bounded Other-L3 cadence data while default profile values
remain unqualified Current data L2. S3--S6 closed every eligible manual or
Other-L3 gap through the existing VADP route. CGA-F4, the unmodelled ISA-wait
formula in CGA-T2, the absent CGA-T3 light-pen lifecycle and CGA-T4 physical
monitor output remain exact L2 because no complete source-qualified input and
Core consumer exists. None is promoted by an arbitrary ratio or a
presentation-side mirror.
