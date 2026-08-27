# T493 S3 IBM 5160 CGA Current Code-Gap List 2

`M5:T493:S3:CGA-LIST-2:OK`

This maps every C1--C33 row of the accepted
[List 1](t493-s2-cga-function-timing-list-1.md) to its present implementation
and proof. `Present` means the retained VADP owner already implements the
source-qualified behavior. `Boundary` means the source itself lacks the value
or mapping needed for a correct additional model. Only `S4` rows are the
complete immediate-repair batch.

| List 1 ID | Current owner/path and proof | Disposition |
| --- | --- | --- |
| C1 | `core_machine_vadp_write_crtc_index` writes the low five bits of `3D4h`; `core_machine_vadp_supported_crtc_index` limits CGA to R0--R17. `core-machine-vadp-text-status-smoke` exercises index/readability boundaries. | Present. |
| C2 | `core_machine_vadp_write_crtc_data` is the sole `3D5h` write path. | Present. |
| C3 | `crtc[0]` is a VADP-owned write-only CGA register, used by logical raster state where configured. | Present; S4 must not create a second CRTC bank. |
| C4 | `crtc[1]` is VADP-owned and supplies text columns/logical raster state. | Present. |
| C5 | `crtc[2]` is the VADP-owned horizontal-sync-position register. | Present. |
| C6 | `crtc[3]` is the VADP-owned horizontal-sync-width register. | Present. |
| C7 | `crtc[4]` is VADP-owned and participates in logical raster state. | Present. |
| C8 | `crtc[5]` is the VADP-owned vertical-total-adjust register. | Present. |
| C9 | `crtc[6]` is VADP-owned and supplies text rows/logical raster state. | Present. |
| C10 | `crtc[7]` is VADP-owned and supplies logical vertical-retrace start. | Present. |
| C11 | `crtc[8]` is the VADP-owned interlace-mode register. No physical interlace waveform is claimed. | Present/Boundary. |
| C12 | `crtc[9]` is VADP-owned and supplies scan-lines-per-row in the logical raster. | Present. |
| C13 | `crtc[10]` is VADP-owned and supplies copied-text cursor start/visibility. | Present. |
| C14 | `crtc[11]` is VADP-owned and supplies copied-text cursor end. | Present. |
| C15 | `core_machine_vadp_crtc_word` consumes R12/R13 for copied-text start/wrap. IBM's selected pages name the register but do not give a graphics-display-start conversion; graphics capture therefore does not infer one. | Present for source-defined text path; physical/graphics mapping Boundary. |
| C16 | R14/R15 are the only readable/writeable CGA CRTC entries and feed copied-text cursor position. | Present. |
| C17 | R16/R17 remain read-only through the port predicate; no source-qualified external light-pen position producer exists. | Present/Boundary. |
| C18 | `core_machine_vadp_write_mode` owns the six-bit mode state; graphics/text capture selects solely from it and output-disable blanks copied output. Text attributes preserve the blink/intensity bit for presentation; no unsourced cadence is imposed. | Present. |
| C19 | `core_machine_vadp_write_color` owns the six-bit color state; `core_machine_vadp_graphics_palette` consumes the documented medium-resolution selection. | Present. |
| C20 | `core_machine_vadp_read_status` is the sole `3DAh` reader. Its logical raster returns bit 0/3 phase; generic CGA bit 1 is missing with C21, and bit 2 has no admitted external switch producer. | S4 for bit 1; Boundary for bit 2 and physical phase. |
| C21 | `3DBh`/`3DCh` are registered, but `core_machine_vadp_write_cga_lightpen` currently changes only the Compaq CECG latch. Standard CGA gets no preset/clear state. | **S4: add one CGA-owned latch used by `3DAh` bit 1 and reset it with VADP.** |
| C22 | Guest software writes the documented order through the one VADP port path; IBM states a programming sequence, not an undocumented write-delay/rejection mechanism. | Present. |
| C23 | `core_machine_vadp_configure_cga_memory` registers one `B8000h`/16 KiB VADP provider; `core_machine_vadp_cga_read/write` own guest access and capture reads through Core memory. | Present. |
| C24 | The provider correctly owns the buffer, but the source gives no numeric horizontal-retrace schedule. Current source-qualified behavior cannot reject or corrupt access on an invented interval. | Boundary, not an S4 estimate. |
| C25 | `core_machine_vadp_capture_text_snapshot` copies character/attribute pairs, 40/80 columns, 25 rows, CRTC start and cursor through VADP. `core-machine-vadp-text-smoke` proves text/cursor/wrap. | Present. |
| C26 | VADP copies raw attributes, including bit 7, across the VADP-to-VM copied frame. The platform consumer may present it; no source-qualified blink cadence exists here. | Present/Boundary. |
| C27 | `core_machine_vadp_capture_graphics_snapshot` decodes four two-bit PELs per byte into 320 by 200 copied pixels. `core-machine-cga-graphics-port-smoke` and VM CGA smokes cover the route. | Present. |
| C28 | That capture uses `80 * (y / 2)` plus `0x2000` for odd lines, exactly the two 8 KiB banks. | Present. |
| C29 | `core_machine_vadp_graphics_palette` uses the palette-select/intensity/background state for the documented two sets. | Present. |
| C30 | `core_machine_vadp_capture_high_res_graphics_snapshot` decodes the documented 640 by 200 bit map and bank map, but `core_machine_vadp_high_res_palette` currently derives pixel 1 from `3D9h`; IBM says this mode is black-and-white only. | **S4: make the retained high-resolution VADP palette binary, with a focused regression.** |
| C31 | `core_machine_vadp_advance` has a deterministic logical raster input. Its XT values are declared non-physical, and no source-qualified CGA clock formula is available. | Boundary, not an S4 estimate. |
| C32 | `core_machine_vadp_capture_snapshot` is the one VADP snapshot producer; `src/vm/composition/session/display.c` copies it into a presentation frame. `vm-display-composition-s5-smoke` proves consumer copy semantics. | Present, one-way owner path. |
| C33 | `core_machine_vadp_reset` resets VADP-owned transient state while retaining configured board state. IBM selected pages do not state a power-on register image; existing deterministic project reset must not be relabelled as IBM power-on behavior. | Present/Boundary. |

## Complete S4 batch

S4 changes only `vadp.[ch]` and its focused CGA machine smoke(s): one
standard-CGA light-pen latch, the status/preset/clear relation, reset coverage,
and the binary high-resolution palette. It does not add a VADP wrapper, a
renderer state mirror, an external input setter, a CRTC bank, an estimated
raster rate or a profile-side graphics path. C1--C20 and C22--C29, C31--C33
retain their named paths; C21 and C30 are the entire owner-local correction.
