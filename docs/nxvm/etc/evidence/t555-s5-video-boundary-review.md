# T555 S5 Video Boundary Review

## Decision State

This is the executor's pre-implementation review against accepted source
`44340d5c1`, active S5 and the [source/receiver ledger](t555-s5-video-source-ledger.md).
It is not an independent API authority or acceptance record. The owner
explicitly answered "Approve all three and continue S5" to the snapshot,
IRQ/deadline/board and project-firmware review request. CURRENT records that
approval and expanded file boundary. Internal V07/V11 arithmetic, V21
Environment and V26/V27 driver delivery have focused dual-width receiving
proof. The approved 50-row/dual-font copied boundary and Core converter are
implemented; the approved IRQ/deadline/wiring boundary now has direct and
real Board/PIC receiving proof. Live chip font/geometry, complete raster/MUX
and project-firmware repairs remain in progress. The earlier repairs use the
standing internal authority.
Shared-six, MyNES and owner media/INIs remain read-only.

The complete current-path audit has identified mechanism classes below.
Original-page reconciliation and exact receiving vectors continue; a row
cannot close merely because its current test passes. This review preserves
all 28 ledger units rather than selecting only the publication defect.

## Confirmed Boundary Needs

### Copied Text Facts, Not Another Renderer

Lib already has `kvm_text_cell.glyph_bank` and both `font` and
`secondary_font` in `kvm_window_text_frame`; its text capacity is 80x50.
No Shared API addition is needed to use those existing 8x16 capabilities.
Core's `x86_video_snapshot`, however, carries one font, no font-selection
fact and only 25 text rows. The current converter hardcodes every cell to
font bank zero. EGA's documented 80x43 mode and programmable dual font
selection therefore cannot traverse that copied boundary.

Proposed existing-value-ABI extension for review:

```c
/* In existing core/chips/video/video_values_interface.h. */
#define X86_VIDEO_MAX_ROWS 50u

/* New copied members in the existing x86_video_snapshot. */
lib_bool text_font_selection;
lib_u8 secondary_text_glyphs[X86_VIDEO_TEXT_GLYPH_BYTES];
```

The chip resolves the font-selection fact from programmed sequencer state;
Machine maps attribute bit 3 to the existing KVM bank only when enabled.
Primary/secondary bytes are copied values, not another VRAM owner. CGA's
selected 25-row limit remains a distinct source fact; increasing export
capacity must not silently change CGA geometry. No live register pointer,
new renderer, source-specific callback or product state is exported.

Originals show an important difference: IBM EGA Character Map Select uses
8-KiB table locations, whereas the Compaq guide specifies four 16-KiB font
banks. They must not share an invented identical offset formula. CECG
printed 4-12/PDF 106 and 4-13/PDF 107 were visually checked; they also prove
sequencer Memory Mode bit 0 is meaningful. The current mask `0E` removes it.
Current public glyph export remains 8x16. The subsequent source/consumer review
below confirms that exact 17--32-scanline export requires a separately approved
Shared capacity extension; the three existing approvals do not authorize it.

### Additional Shared Font-Capacity Review -- Approved For Separate S6

CECG original printed 2-19/PDF 41 was rendered and visually checked. Each
character occupies 32 raster bytes; bits 15:14 select a bank, bit 13 is forced
zero, bits 12:5 select a character and bits 4:0 select its raster. Thus Compaq's
16-KiB bank spacing does not imply a 64-byte glyph stride: the character stride
is 32 bytes, and only the lower 8 KiB of each bank carries its 256 characters.
IBM printed 22/PDF 105 instead selects banks at 8-KiB intervals. Do not combine
bank spacing and character stride into one guessed constant.

Lib's `kvm_window_text_frame` has two 256x16 arrays; its validator rejects
`font_height > 16`, and its renderer indexes characters with a fixed 16-byte
stride. Core currently scales cursor coordinates into that fixed font and
exports no glyph bytes beyond row 15. For example, a 24-row glyph with only
row 20 populated cannot traverse the current boundary exactly. Clipping or
resampling would be a presentation-model decision, not complete glyph support.

Proposed narrow review:

- Keep `KVM_WINDOW_FONT_HEIGHT = 16` and the existing zero-height default.
- Add `KVM_WINDOW_FONT_MAX_HEIGHT = 32` as the copied-font slot capacity;
  size both existing arrays and index glyph slots by that constant. Accept
  explicit heights 1--32. No new rendering path, object or function is needed.
- Keep Core's construction-time normalized 8x16 font configuration intact.
  Extend only snapshot font slots to 32 rows and add a copied
  `text_glyph_height` fact. CGA reports its normalized asset height 16;
  programmable EGA reports its actual character height. Machine copies/pads
  at that one boundary, without guessing card identity or inventing font data.
- Extend existing Lib render/copy tests for zero/default, 8/14/16/24/32,
  secondary banks, last-glyph/last-row and rejecting 33; migrate the two
  fixed-stride font indexes in the existing Emulator publication test.
  Preserve current geometry/default-height regressions.

Estimated capacity-only source C/H: Lib two existing files +8..16/-5..10;
Core values/capture/converter and their existing consumers +25..55/-12..30.
Existing Lib/Emulator/Core tests +80..140/-15..35. These are estimates, not
the measured complete S5 diff. Update their owner manifests only after approval.

No Emulator production/API or MyNES source change is required. Shared changes
would require all ten receiving App artifacts and complete units on both widths,
not just the eight PC artifacts. The inspected SoftPC producer still writes
256x16 fonts directly into these arrays; that sibling is read-only here and
would need its own producer adaptation when adopting the enlarged fixed slot
ABI. It cannot be declared byte-compatible without that change.

The owner explicitly approved this exact change on 2026-10-12 and directed
bounded S5 closure followed by a separate S6. Shared-six and MyNES remain
unchanged until S6 admission. S6 requires public-six quality review, full
dual-width unit/integration tests, ten rebuilt receiving EXEs, target-scoped
commit/push and subsequent owner acceptance. It qualifies the copied capacity
and rendering contract, not complete live EGA/CECG production. Remaining V17
hardware producers and other video classes stay required in S7; T remains open.

### Additional Existing Capture-Result Review -- Approved

Current-state source review finds one failure-contract gap that cannot be
cleanly repaired by another bool wrapper or persistent error flag:

```text
Video capture false
  -> Board capture UNSUPPORTED
  -> Machine callback false
  -> Machine publish IO_ERROR
```

False conflates currently unrepresentable/unprepared copied output with a
real failed backing read. The current mode dispatcher also chooses text when
its graphics shape classifier fails, even though Attribute Mode still selects
graphics. The existing Mode10 test's final assertion expects that false Text
fallback; its current passing run reproduces an incorrect oracle rather than
proving the admitted V19 eligibility contract.

Proposed exact review, with no new function/type/state/parallel path:

- Change existing `x86_video_capture_snapshot_from`,
  `core_machine_vadp_capture_snapshot` and
  `core_machine_display_capture_snapshot_from` returns to existing `lib_status`.
- Change the existing `core_machine_display_snapshot_provider` callback return
  to the same status. Board's already-status-returning capture passes through
  rather than converting failure into UNSUPPORTED; Machine stops compressing
  it to IO_ERROR. Private producer helpers propagate their real statuses.
- Remove `x86_video_capture_text_snapshot_from` from the public interface and
  keep its implementation private. Its only outside callers are three same-
  owner test files; they must exercise the sole public mode dispatcher rather
  than force a text capture around current chip mode.
- Dispatch from programmed mode facts, then validate the corresponding copied
  shape; classifier failure must not invent a different hardware mode.
- OK means a complete valid copied frame. UNSUPPORTED means there is no
  currently representable coherent frame under the admitted output bounds.
  Invalid caller/provider setup and real backing-read errors retain their
  existing distinct status. Guest register bit patterns are not mislabelled
  as invalid host arguments.
- Machine treats the narrowly defined unrepresentable-frame result as no new
  publication, preserving its pending cache and acknowledged generation. It
  propagates actual failures through the existing driver failure route. Both
  non-OK producers leave caller output and capture cache unchanged. This does
  not authorize leaving required selected text/font modes unimplemented.

The owner explicitly approved this existing return-contract convergence and
withdrawal of the test-only text entry, then authorized subsequent Core-only
interface repairs without individual approval. Shared changes remain separate.
The existing callback/types change their public source/semantic contract,
despite using the same scalar status vocabulary and adding no functions. Do not keep bool
compatibility aliases or export a second status-capture entry point.

Read-only inventory locates nine Core source/header files and nineteen directly
referencing test files; the existing provider-slot receiver additionally covers
the one callback binding. Expected C/H change is production +130..180/-110..160,
tests +200..300/-160..240, mostly necessary return/condition migration. The
private producer ordering, failure injection, unsupported-graphics retention,
pending delivery and actual worker receivers must be requalified together.
Shared-six, MyNES, native APIs, threads/queues and third-party ROMs are outside
this result-contract repair. The separately approved 32-row glyph-capacity
change is reserved for S6 admission after qualified S5 closure; it is not
part of the Core/firmware and capture-result repair.

### Vertical Interrupt And Deadline

The CECG guide printed 4-34/PDF 128 was visually checked. CRTC 11h controls
vertical interrupt enable/clear; the interrupt is generated at vertical
blanking start and maps to IRQ9 on the selected 286/386 boards. IBM EGA
printed 72 supplies the IRQ2 source and its status/control relationship.

Current chip advance only updates raster phase, and status bit 7 is not a
latched interrupt fact. Board VADP has no PIC source lease, and Board-Base's
deadline reducer does not inspect video. Storing CRTC 11h cannot qualify
the missing runtime path.

Proposed minimal public chip boundary for review:

```c
/* One new copied output fact in existing x86_video_bus_observation. */
lib_bool vertical_interrupt;

/* One new query, same units as x86_video_advance(). */
lib_status x86_video_ticks_until_event(const x86_video *video,
    lib_u64 *out_ticks);

/* One routing input in existing core_machine_display_config. */
lib_u8 vertical_irq; /* 0 = unconnected; selected EGA/CECG AT route = 9. */
```

The query reports only a real programmed model event; a halted/unconfigured
source returns no eligible event. It does not invent a physical oscillator
or promote a fallback phase to L3. Board-Base applies its existing clock-domain
conversion, reduces into its existing attachment deadline and binds the
existing PIC IRQ-source capability. Private VADP integration publishes that
one chip output on write/reset/advance. No new interrupt queue, scheduler,
worker or public PIC getter is required.

The borrowed PIC lease requires teardown/reset review: VADP currently dies
after PIC, so that ordering must be corrected before VADP may deassert a
borrowed source during destruction. PIC remains lease owner; video retains
only its real interrupt latch and raster phase.

### Default Guest Firmware Is A Real Receiving Prerequisite

Original IBM EGA Miscellaneous Output bit 1 enables processor RAM access;
Compaq uses the opposite meaning. The current generic EGA CPU gate ignores
the IBM bit. The default guest firmware in `boot.asm` and mode-03 `video.asm`
writes `3C2h = 01`, which disables that access under the documented IBM rule.
The mode-03 handler then programs Graphics Miscellaneous `09` (B0000h)
while its character routines still address B8000h. It does not program a
complete EGA text/font path. This is caller evidence, not a hypothesis from
a failed boot.

Correcting the chip while retaining these writes would expose a real default
App failure. Do not add a "default BIOS" exception to the chip or retain
ordinary RAM as a second substitute EGA text store. The whole repair must
include source-correct programming in the project-authored default firmware,
with guest-execution tests, or an explicitly reviewed selected-model boundary.
External IBM/Compaq ROM bytes, host interception and synthetic BIOS replies
are not permitted. The owner has now explicitly admitted that firmware repair
in CURRENT. The additional Shared glyph-height boundary above remains a
separate review.

## Current-Path Repair / Retention Matrix

These are proposed dispositions, not final qualifications. `Repair` denotes
a current implementation difference with a named owner. `Review` denotes a
required source/ABI/model decision before editing. `Retain` keeps a distinct
existing behavior and still needs its current direct/receiving proof.

| Unit | Proposed disposition and owner |
| --- | --- |
| V01 | Retain four selected bindings; recheck App ROM/font preparation and actual receiving modes rather than calling both 5160/5170 EGA. |
| V02 | Repair CGA cursor-start mask/blink interpretation at Video after MC6845 page review; preserve write-only start and read-only light-pen rules. |
| V03 | Repair missing text blink/intensity selection at Video; retain composite-only output effects as an explicit electrical boundary, not a false RGB palette repair. |
| V04 | Retain address-activated light-pen latch commands; no selected physical strobe producer is invented. Status must reflect actual current phase/latch, not read count. |
| V05 | Retain proven CGA text start/wrap/geometry; repair cursor mode effects without changing cursor ownership. |
| V06 | Repair CGA graphics start-address consumption at Video; preserve packed-pixel and interleaved-bank forms. |
| V07 | Repair timing-input arithmetic overflow validation; reconcile selected CGA character-clock division. Existing uncalibrated fallback quanta remain explicitly L2, not physical timing. |
| V08 | Repair EGA inactive/read-only direction and generic RAM-enable handling. The default firmware prerequisite above must be resolved in the same receiving boundary. |
| V09 | Repair original sequencer masks including alpha bit 0 and supported clock/reset effects. Keep retained VGA masks/readback isolated rather than imposing EGA rules on it. |
| V10 | Repair the source-proven CECG exclusive/wrapped cursor-end encoding. Reconcile IBM EGA cursor start/end separately: printed 33/PDF 116 describes a start value one less than the desired raster and does not give the Compaq end table. Do not apply the Compaq rule to generic EGA. Retain same-owner register storage; physical latch/signal claims need separate source/model grading. |
| V11 | Repair bounds/query/read/write consistency and disabled CPU access independently of display reads; preserve one planar store. |
| V12 | Retain original Color Don't Care polarity, read-map behavior and observe-only latch preservation; reserved-map fallback remains labelled. |
| V13 | Retain modes 0--2 rotate/logical/latch/bit-mask mechanisms with current vectors; reserved mode 3 is not upgraded into a new VGA feature. |
| V14 | Repair independent odd/even CPU read/write selection and text plane reads. Display capture must not fail merely because CPU aperture access is disabled. |
| V15 | Repair write-only EGA/CECG readback contract and source-qualified attribute programming tests. Preserve VGA's distinct existing representation; do not erase stored-value assertions. |
| V16 | Repair palette/plane-enable effects through text and graphics snapshots, with selected RGBI/enhanced-color mapping. Restore diagnostic MUX bits rather than four-bit masking. |
| V17 | Review copied snapshot extension above; implement plane-2 font/map selection only with an explicit export/height contract. |
| V18 | Review chip output/deadline and board IRQ routing above; repair logical status, IRQ latch/clear and blink period. Monitor/electrical/unsourced physical cadence remains bounded. |
| V19 | Repair start/stride/panning/line-compare consumers at the single Video capture owner; do not classify a real mode solely by one hardcoded stride value. |
| V20 | Reconcile CECG Control Mode selected-monitor blanking and miscellaneous output; external connector/physical monitor signals are not inferred from copied frames. |
| V21 | Repair Environment independence/reserved bits and selected reset/configuration. Preserve the real Feature Control output state, not its false Environment alias. |
| V22 | Retain one CECG odd/even planar store; B0000h compatibility remains the accepted explicit L2 alias, not a newly asserted physical decode. |
| V23 | Retain candidate route/allocation atomicity; extend real IRQ lease lifetime coverage when that boundary is approved. |
| V24 | Repair semantic text freshness for palette/font/mode changes and failure acknowledgement; no failed capture may advance a published generation. |
| V25 | Retain conversion validation-before-mutation and cursor-to-glyph ownership; extend only approved copied text facts. |
| V26 | Repair cache-versus-pending delivery by reusing the existing flag; consume only after successful copy. Keep cached value validity in the copied frame. |
| V27 | Add actual bound first/reset/paused delivery proof and preserve owner-thread test reads. Do not force driver capture from a concurrent observer. |
| V28 | Retain nonselected VGA regressions without calling them four-App selected-hardware qualification. |

## Estimated File Surface Before Implementation

Estimates are broad because the ABI/firmware review is not approved yet.
They count source/test C/H and guest assembly only; documentation, manifests
and EXEs are excluded. Final delivery must report measured Git additions and
deletions, not these estimates.

| Existing owner/files | Estimated additions / deletions | Responsibility |
| --- | --- | --- |
| `core/chips/video/video.c`, `video.h`, both interface headers | +550..900 / -200..350 | One register/memory/raster/capture mechanism; copied fields/output/deadline only if approved. |
| Board-Base VADP, display configuration, advance/deadline, board lifetime and AT materialization | +90..150 / -20..45 | Existing PIC lease and clock/deadline integration, candidate/reset/teardown. |
| Core Machine frame/display/machine/private state | +35..70 / -20..45 | Existing copied-frame converter and consume-on-success driver delivery, no second publication owner. |
| Default App firmware and guest-execution test prerequisites | Not frozen until admission and source-correct text/font initialization are agreed. | Fix the caller, never manufacture chip BIOS compatibility. |
| Existing Video/Core board/frame/App receiving tests and private fixtures | +1000..1700 / -250..500 | Independent masks/plane/capture/IRQ assertions and actual Core-to-Emulator delivery; remove assertions of incorrect old hardware behavior without deleting their stored-state proof. |

No new framework, manager, thread, Shared API or alternate framebuffer is
proposed. Private new state must correspond only to actual raster/blink/IRQ
facts. The original build trees, dual-width complete units, four-App receiving
integration and eight 0555 PC artifacts remain mandatory after the repair
boundary is approved and frozen. No partial implementation P is planned.
