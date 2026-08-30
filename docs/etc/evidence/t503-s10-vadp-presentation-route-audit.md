# T503 S10 VADP Presentation-Route Audit

## Method And Sources

This S re-read the selected CGA and EGA chapters in the visually inspected
IBM *Technical Reference Options and Adapters, Volume 2* (April 1984; CGA
printed pp. 18--19 and EGA printed pp. 1--55), then traced the live Core and
VM route.  The manual is normative.  Read-only local 86Box EGA sources
corroborate the selected display-enable result; local Bochs is a later generic
VGA implementation and PCjs a broader video model, so neither extends this
selected EGA claim.  MAME and QEMU sources are not present locally.  No
external code or binary is imported.

## Complete Selected Route Ledger

| Observable | Primary fact and disposition | One NXVM route | External comparison and result |
| --- | --- | --- | --- |
| CGA mode, CRTC and B8000 aperture | IBM CGA register and mode tables: Manual L3 for the selected text, `320x200x4` and `640x200x2` path. | Guest ports/memory -> `vadp.c` register/VRAM state -> snapshot. | Existing owner-local and VM CGA smokes cover port, odd/even scanline and copied snapshot. |
| CGA 640x200 foreground colour | CGA printed p. 18 states that `3D9h` bits 0--3 select blue/green/red/intensity foreground in 640x200: Manual L3. | `core_machine_vadp_high_res_palette()` now maps the low RGBI nibble through the existing palette table. | The prior hard-coded white pixel was wrong; `core-machine-cga-640-port-smoke` now proves `0Ch` produces `ff5555`. |
| EGA port and planar aperture state | IBM EGA printed pp. 12--55 define the selected external, sequencer, CRTC, graphics and attribute register families, planar mapping and register accesses: Manual L3 within the retained selected subset. | Guest ports/memory -> the same VADP register arrays, plane storage and memory provider -> planar snapshot. | 86Box `vid_ega.c` likewise keeps controller state and mapping together; no source is copied. |
| EGA geometry vs CPU mapping | IBM EGA graphics/CRTC chapters distinguish graphics memory from CRTC output geometry: Manual L3. | Mapping predicate and CRTC tuple classifier remain separate VADP predicates; recognized D/E/10 tuples alone form planar frames. | Unknown tuples deliberately return no source-qualified frame, not an invented fallback geometry. |
| EGA output disabled | IBM EGA Attribute Controller output-enable state is a guest-visible display gate: Manual L3. | Attribute address phase changes the sole VADP output state; a recognized planar geometry emits a same-geometry black snapshot. | 86Box stores Attribute bit 5 and selects `ega_render_blank` unless palette output is enabled; Bochs also gates its generic VGA display on Attribute video enable.  NXVM previously returned no snapshot, leaving a stale presentation; this S repairs it in VADP. |
| EGA text / unsupported frame boundary | The IBM manual distinguishes alpha and graphics display paths. The retained selected contract supports its existing text snapshot and only the named planar geometries: Manual L3 / explicit bounded surface. | VADP creates either text or selected planar snapshots; VM never interprets guest registers or VRAM. | No broad EGA text/BIOS-mode claim is added. |
| Reset and snapshot lifecycle | VADP reset retains frozen configuration while clearing mutable adapter state; copied capture is a Core observation boundary. | `machine_display.c` calls VADP; `vm_session_publish_display()` copies a completed snapshot to the presentation mailbox. | No VM/renderer mode, VRAM or guest-frame mirror exists. |
| Raster/time | IBM gives adapter clocks and CRTC timing facts, but current Core clock conversion is not derived from the selected physical board inputs. | VADP advances on the existing Core clock domain and publishes no guest-wakeup deadline. | Retained explicit L2 timing boundary; no guessed dot clock or host cadence is introduced. |

## Repairs And Minimality

The audit found two source-visible output defects in the existing single owner:

1. The CGA high-resolution palette discarded the manual-defined `3D9h` RGBI
   foreground selection and always emitted white.  Reusing the existing RGBI
   mapper changes one expression; it adds no state or rendering path.
2. An EGA output-disable write could leave the VM with the prior nonblank
   frame.  VADP now returns the source-known geometry as a zeroed snapshot and
   marks the existing dirty generation when Attribute display enable changes.
   VM's existing copy path therefore publishes the blank frame without any VM
   workaround or second truth.

The implementation delta is 39 added and 4 removed source/test lines (before
this evidence and state record).  It retains one VADP state owner, one memory
provider, one snapshot operation and one VM copied consumer.

## Proof And Residual Boundary

Focused owner/consumer replay covers CGA graphics and 640 ports, EGA planar
and mode-10, copied composition, ROM EGA mode setup, Model-339 CGA topology
and Model-40 CECG mapping.  The mode-10 contract now proves both a rendered
frame and a display-disable transition with `buffer_changed` and black pixels.
The required complete repository-only unit replay follows this record.

No selected CGA/EGA port, memory mapping, recognized frame geometry, output
gate, reset, snapshot or copied-presentation route remains untraced.  Physical
raster/dot-clock conversion and unselected EGA/VGA geometries remain the
already explicit bounded L2/later-admission surface; they are not a hidden
route defect in this S.
