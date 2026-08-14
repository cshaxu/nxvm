# T352 S4: Selected EGA Planar Digital Path

## Scope And Outcome

The selected EGA implementation has one mutable owner:
`src/core/machine/vadp.c`.  It owns planar storage, latches, indexed
sequencer/graphics/attribute state, the active aperture, dirty tracking, and
the captured indexed frame.  `t_ram` owns checked physical-memory routing;
VM and platform consumers receive a copied display snapshot.

No production defect was reproduced in the selected contract.  This S adds a
direct reset-and-copy proof to the existing planar port owner.  Reset clears
the transient plane store and disarms planar access; this is the intended
reset state, not a second configuration path.  A normal guest graphics-mode
write re-arms the retained configured topology, after which the cleared
planes produce a fresh zero-valued EGA frame.  The previously captured output
values remain unchanged.

## State, Writer, And Reader Sweep

| State or boundary | Writer/preparation | Reader/publication | Selected disposition |
| --- | --- | --- | --- |
| Registration and topology | `core_machine_vadp_configure_ega_sequencer` allocates the four-plane store and atomically registers the provider/observer; controller configuration normalizes and installs its one indexed image | T334 transaction smoke proves allocation/provider/observer failure leaves no half registration and a retry succeeds | One VADP owner; no generic memory/provider change. |
| Planar activation | sequencer reset/map mask, graphics `5`/`6`, attribute `10h`, display-enable, and `ega_planar_armed` form the activation predicate | provider query/read/write reject inactive or out-of-aperture requests before planar publication | A graphics-mode data write explicitly re-arms after reset; no hidden activation path. |
| Aperture | graphics map-select chooses A0000 128 KiB, A0000 64 KiB, B0000 32 KiB, or B8000 32 KiB | `core_machine_vadp_ega_aperture_contains` and provider bounds use the same selected range | Controller and planar port tests prove selected aperture and inactive-address nonpublication. |
| Latches and writes | planar read loads all four latches; write applies rotate, set/reset, logical operation, sequencer map mask, and graphics bit mask | selected-plane read and captured pixels consume the same plane store | Planar port smoke proves map/read, latches, set/reset, map-mask, and bit-mask result. |
| 320x200x16 capture | selected active planar predicate, palette registers, and four planes | direct capture decodes eight pixels per planar byte with 40 bytes per row into a copied indexed frame | Planar port, VM boot, and DOS owners prove geometry, palette, and guest port writers. |
| 640x350x16 mode 10h | guarded CRTC offset/start state and the same plane store | direct capture uses 80 bytes per row and word CRTC start multiplied by two with 64 KiB plane wrap | Mode-10 contract and VM boot prove first/last rows, mode geometry, start, and palette. |
| Reset/finalize/copy | reset preserves configuration topology, clears planar storage/latches/capture state, then leaves planar access disarmed; finalize releases only private planar storage | re-armed capture creates a new zero frame; VM copies capture output into presentation ownership | S4 planar port proof reads cleared plane storage after an explicit guest re-arm and preserves sampled pre-reset frame values; S5 retains cross-mode/mailbox composition. |

## Layout Decision

The 320x200 and mode-10 640x350 selected paths deliberately do not share a
synthetic row/start formula.  They have different selected geometry and row
stride; mode 10 additionally has an admitted CRTC word-start contract.  That
is a real EGA layout difference, not construction drift.  General EGA mode
tables, text compatibility, arbitrary CRTC address semantics, VGA/VBE, and
host rendering remain outside T352 and are retained as exact future breadth.

## Direct Proof

| Proof owner | Selected evidence |
| --- | --- |
| `current.core-machine-ega-sequencer-port-smoke` | Sequencer masks, planar provider registration, A0000 range, dirty write observation, and non-aperture behavior. |
| `current.core-machine-ega-registration-transaction-smoke` | Allocation, provider-capacity, and observer-capacity rejection is atomic; retry has one provider and observer. |
| `current.core-machine-ega-controller-port-smoke` | Graphics/attribute masks, status flip-flop, all selected map windows, dirty observation, and controller reset. |
| `current.core-machine-ega-planar-port-smoke` | Four-plane map/read, latch/set-reset/mask write behavior, aperture switch, 320 frame/palette, reset clear after guest re-arm, and prior captured value isolation. |
| `current.core-machine-ega-crtc-boundary-port-smoke` and `current.core-machine-ega-mode10-contract-smoke` | Bounded offset/adjacent state and 640x350 start/stride/palette contract. |
| EGA VM/ROM/DOS owners | Guest port and ROM writers reach the same selected copied 320 or 640 indexed frame without a host mutation route. |

## Transfer

S5 owns cross-mode sequencing, timeline order, reset-to-presentation mailbox
composition, and the task artifact.  Broader EGA/VGA register semantics,
unselected display modes, host pacing, and dot-clock/bus contention remain
later device/L3 work; they are not silently represented by this selected
planar proof.
