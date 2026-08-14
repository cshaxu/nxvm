# T352 S1: Selected PC/AT Digital-Video State Ledger

## Selected Contract And Sources

T352 uses the IBM Color/Graphics Adapter and Enhanced Graphics Adapter
technical-reference register/mode tables as the hardware authority.  The
project retains their selected digital subset only: text modes, digital CGA
`320x200x4` and `640x200x2`, EGA planar `320x200x16-direct`, and EGA mode
`10h` / `640x350x16-direct`.  The IBM EGA register tables are available in
[IBM Technical Reference Options and Adapters, Volume 2](https://bitsavers.org/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_2_Apr84.pdf).
This is documentation research only; no external source or firmware is
imported.

## State And Ownership Matrix

| Selected state family | Sole owner and mutation boundary | Current proof | S disposition |
| --- | --- | --- | --- |
| CGA mode/control/color (`3D8h`, `3D9h`) and text geometry | `vadp.c` port callbacks and `core_machine_vadp_configure_text`; ROM writes real ports | VADP text, CGA graphics/high-res port, ROM/DOS display tests | S3: complete selected mode/control, aperture, palette, and reset matrix. |
| CRTC index/data (`3D4h`, `3D5h`), cursor/start/offset | one bounded `crtc[20]` bank with index predicate/masks | VADP text/status and EGA CRTC-boundary smoke | S2: reconcile selected CRTC register semantics with text/CGA/EGA raster consumers; preserve the T314 bound gate. |
| Raster/status (`3DAh`) | VADP `raster_phase`, configured timing, and timeline callback | VADP text/status and T346 input-display timeline smoke | S2: prove selected display-enable/retrace phase/reset behavior and its relation to the selected state matrix; no host cadence claim. |
| CGA B8000 aperture and packed odd/even scanlines | VADP snapshot read through checked RAM; VADP owns palette and dirty capture state | CGA graphics/high-res port, VM CGA system/DOS tests | S3: reconcile selected memory mapping, page/start interaction, copied indexed frame, and unsupported mode boundary. |
| EGA sequencer (`3C4h/3C5h`) and planar registration | VADP configuration, memory provider/observer transaction | EGA sequencer, registration-transaction, planar port tests | S4: reconcile selected reset/map-mask/memory-mode and aperture activation with rendering; preserve registration atomicity. |
| EGA graphics/attribute controllers (`3CEh/3CFh`, `3C0h/3C1h`) | VADP indexed callbacks, masks, attribute flip-flop, and planar latches | EGA controller/planar port tests | S4: reconcile selected write/read/palette/enable state with direct modes; unsupported controller behavior is an explicit transfer, not an inferred VGA claim. |
| EGA A0000 aperture, four planes, latches, display start/offset | VADP provider and copied planar snapshot | EGA planar, mode-10, CRTC-boundary, ROM/DOS tests | S4: reconcile selected direct 320x200 and 640x350 geometry, stride/start wrap, reset, and copied pixels. |
| Copied display snapshot to VM/platform | `core_machine_capture_display_snapshot` then VM copy into presentation mailbox | display authority, ROM video, platform/window/Console system tests | S5: compose cross-mode capture generation, reset isolation, and consumer non-mutation; no renderer or host pacing expansion. |
| Deterministic advance/order | T346 machine timeline, VADP last in due-tick order | T346 S5 timeline evidence | S5 retains the order proof and composes only selected display state; general bus wait states transfer to L3 bus timing. |
| Firmware/BDA/INT 10h selected mode writer | default-profile firmware/QDCGA through checked core bindings and guest ports | ROM EGA mode-10, CGA graphics/high-res, DOS video tests | S3/S4 consume only selected mode writers; BIOS service breadth remains a firmware/Windows-readiness transfer. |

## Selected Hardware Matrix

| Family | Selected behavior | Explicit boundary |
| --- | --- | --- |
| Text | 40/80-column CGA text, CRTC start/cursor shape/visibility, copied text frame | Additional geometry/light pen stay in `TODO(Medium) Complete digital CGA`. |
| CGA graphics | Digital `320x200x4` and mode `06h` `640x200x2`, B8000, palette/mode-control selected bits | Composite/NTSC fidelity stays `TODO(Low)`; other CGA modes require an exact selected contract. |
| EGA | Configured planar aperture, selected sequencer/graphics/attribute controls, direct `320x200x16` and `640x350x16` mode 10h | General EGA BIOS mode table, text compatibility breadth, VGA/VBE/SVGA, and unselected CRTC/controller semantics are not claimed. |
| Raster/timing | Deterministic display-enable/vertical-retrace model driven by guest ticks | This is L3 ordering, not physical dot clocks, scanline contention, host refresh, or cycle-exact video. |

## Writer, Reader, And Lifecycle Sweep

`vadp.c` owns mutable display state, port callbacks, apertures, snapshot
generation, reset, and finalization.  `machine.c` owns timeline scheduling and
advances VADP after KBC at each due tick.  The default profile supplies frozen
topology/configuration; QDCGA/ROM write the admitted guest-visible ports and
BDA state; VM session copies snapshots to a presentation mailbox; platform
consumers read only the copy.  No platform module writes VADP, guest video RAM,
or the guest clock.

S2 must sweep all CRTC/status/raster writers/readers.  S3 and S4 must each
sweep their aperture, mode, ROM, snapshot, reset, and consumer readers before
changing code.  S5 must prove that mode switching, reset, and timeline
publication cannot leave a stale copied frame or a second state owner.

## Transfers And Planned Receivers

| Deferred boundary | Receiver and admission condition |
| --- | --- |
| Additional selected digital CGA modes/CRTC geometry/light pen | Existing `TODO(Medium) Complete digital CGA`; require an exact IBM mode/register matrix and port/memory/frame/DOS proof. |
| Composite artifact color/phase/colorburst | Existing `TODO(Low) CGA composite-video fidelity`; require renderer/profile contract without changing VADP digital state. |
| Broad EGA compatibility, VGA/VBE/SVGA | Later named adapter/profile task with a complete hardware matrix; never inferred from the direct selected paths. |
| Host refresh/window pacing/GPU work | Platform product task after a copied-frame consumer contract; no host time becomes guest time. |
| General memory/I/O wait states and scanline bus contention | Ordered L3 bus-timing convergence candidate; require bus ownership and device timing evidence. |
| Windows application/setup behavior | Windows 3.x readiness map after selected device and bus candidates; not a T352 validation substitute. |

## S2--S5 Closure Requirements

Each implementation S must use a focused owner smoke plus retained system/DOS
proof, named source/manual row, actual code review, current-gate run, and a
similar-issue sweep.  It may repair an identified VADP mechanism only after
mapping its writers, readers, validation/publication ordering, reset/finalize
path, and copied-consumer impact.  A no-op audit outcome is valid only when
the whole selected row has direct evidence; it cannot be inferred from a
nearby graphics mode.
