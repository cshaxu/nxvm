# T480 S2 VADP Code-Gap Ledger

`M5:T480:S2:OWNER:OK`

## Existing Production Path

`guest/firmware port or mapped-memory transaction -> core_machine_vadp ->
core_machine_display_snapshot -> vm_session_publish_display -> platform frame`

`t_vadp_data` owns CRTC, sequencer, graphics, attribute, EGA latches, planar
VRAM, CGA VRAM and snapshot-change state in `src/core/machine/vadp.[ch]`.
`vm/composition/session/display.c` obtains only a copied snapshot through the
Core display-provider boundary and does not own a video mode, palette or VRAM.

## Source-To-Code Reconciliation

| S1 row | Current owner/path | Gap and receiver | Disposition |
| --- | --- | --- | --- |
| V1 | `t_vadp_data` contains the common CRTC, EGA register banks and one EGA planar allocation. | Retain this sole state. S3 may make common helpers explicit only if it deletes duplication. | Owner complete; S3 common audit. |
| V2 | `core_machine_vadp_read_attribute`/`write_attribute` retain the attribute phase and display-enable state; status reads reset the phase. | Verify VGA-compatible masks and palette-enable behavior in S3. | Common state exists; S3. |
| V3 | EGA sequencer register 4 and map mask exist; planar read/write owns latches and write routing. | Chain-4 is not modeled. Add it to the existing aperture handler/state, not a new VRAM representation. | S4 VGA-only. |
| V4 | EGA binds one fixed A0000h/64 KiB planar aperture through the Core memory provider. | Graphics register 6 exists, but the four IBM VGA aperture selections are not all materialized. | S3 common map selection; S4 chain-4 interaction. |
| V5 | `core_machine_vadp_ega_planar_read/write` owns latches, read modes, compare, rotate, logical operation, set/reset and write modes. | Audit exact VGA-compatible masks in S3; do not bypass it for 256-colour writes. | Common state exists; S3. |
| V6 | No DAC mask/address/component state or 3C6h--3C9h ports exist. | Add the finite DAC state to `t_vadp_data`, with one port owner and snapshot palette publication. | S4 VGA-only. |
| V7 | Snapshot kinds cover CGA and selected EGA indexed frames; VM already publishes indexed pixels and a palette copy. | Add one 320x200x256 VADP snapshot production branch sourced from existing VADP VRAM/DAC state. | S4 VGA-only. |
| V8 | CRTC start/offset feeds selected EGA frame capture, but current geometry is intentionally only the admitted EGA set. | Keep CPU aperture, generated geometry and output-disabled/text fallback as separate predicates; no QEMU-style display-fetch shortcut. | S3/S4, source-gated. |
| V9 | VADP has a local raster phase and profile-supplied text timing; VM's 16 ms publication cadence is host presentation only. | No source-qualified VGA timing plan or profile clock route is present. | L2/unsupported pending a named profile/card receiver; S4 must not guess. |
| V10 | Current profile descriptors expose `ega_present` and selected EGA ports only; Model 339 is CGA and Model 40 is Compaq EGA. | No current immutable profile/card declaration may select VGA. | S5 only; unselected. |
| V11 | No VBE/SVGA state or port route exists. | Keep absent. | Unsupported. |

## Minimal Receiver Partition

- S3 keeps and verifies the EGA-common CRTC, attribute, sequencer, graphics,
  latch and aperture mechanism in VADP. It may replace a duplicate only if the
  obsolete path is deleted in the same change.
- S4 adds only the IBM VGA-exclusive DAC, four-map decode, chain-4 and 256
  colour snapshot path to that same VADP state. The VM receives no VGA state.
- S5 decides profile/card binding. Until then no existing machine advertises
  VGA, and the Core capability remains unselected.

`M5:T480:S2:GAPS:OK`

`M5:T480:S2:PROFILE-UNSELECTED:OK`
