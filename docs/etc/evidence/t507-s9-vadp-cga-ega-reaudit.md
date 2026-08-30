# T507 S9 VADP CGA/EGA Re-Audit

`M5:T507:S9:VADP-CGA-EGA:P2`

## Source And Method

The selected primary source is IBM, *Technical Reference: Options and
Adapters, Volume 2*, revised April 1984, SHA-256
`B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
Its owner-managed scan is archived as
`assets/manuals/controllers/ibm/IBM_Technical_Reference_Options_and_Adapters_Volume_2_Apr1984.pdf`.
Rendered CGA pp. 1 and 15--25 and EGA pp. 1, 12--68 were read visually; OCR
located pages only.  The existing complete CGA List 1
[`t467-s1-cga-function-timing-checklist.md`](../research/t467-s1-cga-function-timing-checklist.md)
and EGA List 1
[`t450-s15-vadp-function-timing-checklist.md`](../research/t450-s15-vadp-function-timing-checklist.md)
remain the frozen source universe.

Read-only local comparison used 86Box `src/video/vid_cga.c` and
`src/video/vid_ega.c`, PCjs `machines/pcx86/modules/v2/video.js`, and Bochs
2.6 `iodev/vga.cc`.  86Box confirms that Attribute Controller index bit 5
selects a single blank/text/graphics output path.  PCjs confirms the 3C0h
index/data flip-flop and Status-1 phase reset.  Bochs is later VGA only.
MAME and QEMU video sources are not in the local corpus.  These references
corroborate the owner shape but do not add IBM register or timing facts; no
external code is imported.

## List 1 Reconciliation

| Frozen rows | Source disposition | Selected owner and receiver |
| --- | --- | --- |
| CGA-R1--R5, F1--F7 | Manual L3 as already frozen: CRTC, 3D8h/3D9h/3DAh, light pen, text/320/640 layout and B8000h storage. | `vadp.c` CGA port/VRAM/CRTC state; copied snapshot. |
| CGA-T1--T5 | CRTC/display logical relation is Manual L3; selected board cadence, ISA waits, light-pen lifecycle and monitor rendering retain their documented L2 boundaries. | VADP clock receiver and copied VM presentation only. |
| VADP-R1--R5, F1--F5 | Manual L3 for IBM EGA register grammar, planar maps, recognized geometry and attribute state; named board/monitor/clock conversions retain the frozen L2 boundaries. | `vadp.c` EGA sequencer, graphics, attribute, CRTC, plane storage and snapshot. |
| VADP-T1--T5 | Manual L3 adapter topology; unselected card decode, monitor and ISA arbitration remain L2. | Frozen profile construction feeds VADP once; VM has no guest-video state. |

The attribute row is decisive here: IBM EGA pp. 56--62 defines the 3C0h
address/data phase and output state; Status-1 reads reset the phase.  The
manual does not authorize a text-specific bypass.  The existing CRTC and
planar geometry rows remain unchanged; no raster/deadline claim is added.

## List 2 Owner/Consumer Audit And Repair

| ID | Current route and finding | Disposition |
| --- | --- | --- |
| G1 | Guest CGA/EGA port writes, CRTC state, CGA B8000h and EGA aperture writes terminate in `vadp.c`; the memory observer only marks that owner dirty. | Retained one owner. |
| G2 | VADP alone derives text, CGA and selected EGA planar snapshots. `machine_display.c` exposes a completed copy, while `vm_session_publish_display()` copies it once to the presentation mailbox. | Retained one data flow; VM has no register, VRAM, mode or frame truth. |
| G3 | Attribute index bit 5 already updated the sole VADP `attribute_display_enabled` state and suppressed recognized planar frames.  Generic EGA text fallback checked only the CGA 3D8h output bit, so the same disabled EGA output could publish text. | **Repaired:** text capture reuses the existing EGA output predicate, producing the same blank text state as the planar blank path. |
| G4 | Status-1 read resets the one attribute phase; the selected port smoke covers this ordering before and after the repaired output gate. | Retained Manual L3 path. |
| G5 | VM presentation copies only VADP's immutable capture value into its mailbox.  No direct VM test or production route reads VADP private state. | Retained direct-consumer boundary. |
| G6 | Unrecognized EGA geometry, physical raster/dot-clock conversion, board arbitration and monitor electrical behavior are still the frozen L2 boundaries. | No fabricated timing or renderer path. |

## Proof And Minimality

`core_machine_ega_planar_port_smoke` now writes visible text through the
existing memory route, disables Attribute output through the documented
Status-1/3C0h sequence, and proves that capture changes to blank text before
re-enabling the same state for the existing planar proof.  The repair is one
additional condition around the existing capture loop; it adds no state,
port, callback, video mode, VM branch, renderer cache or deadline.  The
focused owner test passes.  The complete repository-only unit replay passes
312/312 in 16.28 seconds.

## P2 Actual-Diff Review

P1 `e704100f` changes two production lines and adds nineteen owner-local test
lines.  It reuses the existing `core_machine_vadp_ega_output_active()`
predicate, which already governs the recognized planar blank route.  The
review finds no new state owner, public ABI, device deadline, profile input,
VM special case, renderer cache, second snapshot route or unsourced timing.
The evidence/status delta is limited to the S9 source, List 1/List 2,
verification and corrected packet exit wording.  The unrelated working-tree
proposal change was neither staged nor reviewed as S9 work.
