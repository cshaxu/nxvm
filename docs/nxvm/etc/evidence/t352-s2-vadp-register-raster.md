# T352 S2: Selected VADP Register And Raster Lifecycle

## Audited Mechanism

The selected digital display register/raster mechanism has one mutable owner:
`src/core/machine/vadp.c`.  Its callback groups are CRTC (`3D4h/3D5h`), CGA
mode/color/status (`3D8h`--`3DAh`), EGA graphics (`3CEh/3CFh`), attribute
(`3C0h/3C1h`), and sequencer (`3C4h/3C5h`).  `machine.c` is the sole caller
that advances VADP at deterministic due ticks; VM and platform code consume a
copy of a captured snapshot and do not mutate this state.

## Validation And Publication Sweep

| Mechanism | Validation/preparation | Publication/consumer | Result |
| --- | --- | --- | --- |
| CRTC | `core_machine_vadp_supported_crtc_index` guards every dynamic subscript; compile-time assertions guard every direct index; masks are selected before store | Cursor/start/offset readers use the same bank; dirty generation changes only on an effective write | Retained: T314 bound and focused port proof cover selected `0Ah`--`0Fh`/`13h`; no second offset state. |
| CGA mode/color | Selected high-resolution mode rejects unsupported `1Ah` variants before state publication; effective writes mark dirty | CGA snapshot uses the registered state and palette | Retained for S3, whose aperture/mode matrix owns selected mode semantics. |
| EGA indexed registers | Configuration gate, supported-index predicate, and per-index masks precede each data write | Planar activation/aperture/dirty state consumes the same masked registers | Retained for S4, whose selected planar modes own pixel/aperture proof. |
| Attribute flip-flop | Attribute port alternates index/data phases; reading status resets the phase only when EGA controllers are configured | Attribute display-enable and masked data feed selected EGA activation/palette | Existing controller smoke proves status reset, masks, and readback; no stale writer found. |
| Raster/status | Valid timing requires nonzero active/retrace and bounded unsigned period; reset begins in active-display phase | status is pure phase observation except the selected flip-flop reset | Existing VADP status smoke proves vertical/display/horizontal phases, repeated reads, wrap, and reset configuration. |
| Reset/finalize | Reset preserves configured topology, clears transient/register/capture/planar contents, reapplies selected config, and resets dirty generation; finalize releases only planar storage | Timeline reset reschedules the one peripheral callback; no callback captures host state | Existing controller/text/timeline proof retains reset and due-tick order. |
| Snapshot/copy | VADP captures into a value snapshot and tracks local dirty/captured state | VM session copies it into a presentation mailbox; platform reads the mailbox | No consumer writes VADP or guest video RAM; S5 owns cross-mode stale-copy composition. |

## Focused Proof Mapping

| Proof owner | Selected evidence retained by S2 |
| --- | --- |
| `core-machine-vadp-text-status-smoke` | CRTC masks/readback, cursor/page wrap, status phase sequence, deterministic local advance, and text reset state. |
| `core-machine-vadp-text-smoke` | display/retrace/blank traversal, text snapshot dirty/cursor publication, mode/color readback, and unsupported high-resolution mode retention. |
| `core-machine-ega-controller-port-smoke` | graphics/sequencer/attribute mask and index handling, status-reset flip-flop, active aperture selection, dirty observer, and controller reset. |
| `current.core-machine-ega-crtc-boundary-port-smoke` | `13h` round-trip, adjacent state isolation, and selected display-kind transition without out-of-bounds CRTC access. |
| `core-machine-input-display-s5-smoke` | timeline order places VADP after KBC under the retained due-event owner; reset replaces pending callbacks. |
| display authority and VM system/DOS tests | frozen configuration and copied display-consumer boundary. |

## Outcome And Transfers

No S2 mechanism defect reproduced.  The audit found neither a duplicate
register/raster writer nor a validation/publication ordering split.  The
selected CRTC set is intentionally narrow; expanding arbitrary 6845/EGA
register semantics is not a missing S2 repair and remains allocated by the
S1 ledger.

S3 owns selected CGA mode/aperture/pixel behavior; S4 owns selected EGA
planar/controller-to-pixel behavior; S5 owns mode-switch/reset/copy
composition.  General wait states, dot clocks, scanline contention, and host
refresh remain the later L3 bus-timing or platform receivers.
