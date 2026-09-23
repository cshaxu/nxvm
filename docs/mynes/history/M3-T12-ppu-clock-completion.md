# M3 T12: PPU And Clock Completion

## Admission

T12 consumes the second ordered M3 candidate under the owner's standing
automatic M3 execution approval. It follows T11's accepted vertical foundation
at `96a9f71`; no Git remote exists, so every P delivery is local. Its outcome is
the declared mapper-0 PPU and production slot-clock behavior with direct
hardware-state proofs, not a compatibility claim for commercial ROMs.

## S1 Contract

S1 first freezes and implements the remaining PPU register/latch and rendering
state contracts that can be proven independently: all register/mirror accesses,
address and scroll toggles, palette aliases, rendering clipping and frame
publication boundaries. It uses the registered PPU research records and the
[PPU completion proposal](M3-T12-ppu-clock-completion-proposal.md). It may change only
product-owned Core and `test/core`/`test/integration` files; Lib/Common and their
tests remain byte-identical.

The S1 outcome must be a production-path test matrix and corrected behavior,
with positive and side-effect-negative cases on both architectures. Exact
fetch/evaluation pipeline timing, all sprite status races, DMA timing and NMI
races remain later bounded S work inside T12. A contradiction in the registered
research or a required shared change stops the work for review.

## S1 Implementation Record

P1 makes `$2000` select both the PPU temporary nametable bits and the renderer's
base nametable. P2 removes the separate `scroll_x`/`scroll_y` camera fields:
the renderer now derives coarse X, coarse Y, fine Y and fine X from the PPU
temporary address plus fine-X latch. `$2005` and `$2006` share the same write
toggle, and the second `$2006` write replaces the low byte before copying `t`
to current address `v`.

The Core proof distinguishes all four `$2000` base selections under mapper-0
mirroring, then writes a coarse-Y/fine-Y scroll of 239 through `$2005` and
proves the next scanline selects the vertically adjacent nametable. It also
asserts the exact `t/x/w` values after `$2000`, both `$2005` writes and both
`$2006` writes. P3 adds palette alias, PPUDATA-32, OAMADDR-wrap and PPUSTATUS
toggle-reset receivers. Focused `mynes.core.ppu-smoke` passes on shared x64 and
x86 at `2e17adf`; the full 99-test shared suites and documentation governance
also pass on the current T12 baseline. This is an S1 implementation record, not
an acceptance checkpoint: the complete PPU register and rendering matrix remains
active.

P4 makes selected-sprite capacity explicit in the production pixel path. Eight
in-range primary-OAM entries consume the scanline pipelines even when their
pattern sample is transparent; a ninth candidate cannot become a visual
fallback. `mynes.core.ppu-smoke` constructs exactly that transparent-eight,
opaque-ninth case and passes on shared x64/x86 at `1c10e39`. This still does not
claim the later indexed overflow-evaluation or fetch-timing work.

P5 retains the selected primary-OAM indices at scanline evaluation time, and the
pixel sampler consumes only that fixed eight-entry selection. The production
state therefore has an explicit boundary between scanline selection and pixel
sampling rather than re-scanning all primary OAM for every dot. The retained
capacity fixture passes on shared x64/x86 at `1bc178d`; indexed overflow and
fetch phase accuracy remain active work.

## S1 Acceptance And S2 Admission

S1 is accepted through `1bc178d`: it replaces the separate scroll camera with
PPU `v/t/x/w` state, proves base-nametable and cross-nametable rendering, fixes
PPUADDR's low-byte replacement, and records the register side effects and
selected-sprite boundary through the production PPU API. The full 99-test x64
and x86 suites and documentation governance passed at the S1 baseline. The
remaining exact fetch/increment-copy timing, indexed overflow behavior, status
races, DMA halt/continuation and run-slice equivalence are not claimed here.

S2 is admitted under the standing M3 execution authority to own that finite
timing batch. It must add production-clock phase receivers for each behavior,
correct any contradiction in the slot model, and preserve S1's frame/register
proof. Native presentation, App input and Lib/Common changes remain excluded.

## S2 Implementation Record

P1 replaces the monolithic OAM transfer loop with machine-owned DMA phases:
halt read, optional alignment read, source read and `$2004` write. The current
page, byte index, data latch and phase survive a run-slice limit. Direct full
service preserves the 513/514-slot contract, while `core_machine_run` consumes
only its remaining budget and resumes DMA before guest instruction execution.
`mynes.core.dma-smoke` proves a 20-slot interruption after nine completed byte
pairs, then resumes to completion and executes the next guest instruction on
both shared architectures at `92138de`.

P2 proves a budget-bounded DMA still advances the PPU through its production
slots. P3 compares a 513-slot direct transfer with six exact run budgets and
proves identical OAM, PPU position, total slots, cycles and PC at `9d2b9e1`.
P4 replaces run-time RGB emphasis calculation with the immutable 512-entry
Core palette table used by both frame routes; the output fixture checks the
combined three-bit emphasis value directly at `9a260e4`. Native surface
qualification remains with Presentation.

P5 adds `mynes.integration.motion-rom-smoke`, a second project-owned NROM
fixture. Its 6502 initialization writes both CHR tiles, palette/CIRAM and OAM
through production registers, parks inactive OAM entries, then reads controller
A through `$4016`. The pressed branch writes a new `$2005` horizontal scroll and
sprite X through `$2003/$2004`; Window pixels from complete published frames
prove both changes without test-side PPU mutation. It passes on shared x64/x86
at `50c84a9`.

P6 runs that same fixture through the text-only Console route as a direct
receiver. After controller A is pressed and complete frames are published, a
Console cell changes in glyph or foreground color, proving that Core's graphics
to-text conversion reflects the ROM-driven motion. The direct Console proof
passes on shared x64/x86 at `5ff1ff2`. These fixtures are deliberately small,
project-authored mapper-0 programs: they establish current production-path
contracts without implying compatibility with commercial ROMs or mapper
extensions. Native surfaces still belong to Presentation.

P7 implements the bounded vblank-status race documented by the registered PPU
frame-timing source: a `$2002` read at scanline 241, dot 0 returns a clear flag
and suppresses the following dot-1 vblank transition and NMI line for that
frame. The suppression is private PPU state and is consumed at dot 1, so an
ordinary status read retains its normal clear-after-read behavior. The direct
phase receiver passes in `mynes.core.ppu-smoke` on shared x64/x86 at this
checkpoint. Same-dot/later-dot NMI sampling remains a distinct CPU/PPU
integration member of S2.

P8 replaces the count-only ninth-sprite check with the registered indexed OAM
overflow walk. Core retains the first eight selected entries, then evaluates
later primary-OAM bytes with the hardware's coupled `n`/`m` miss increment; a
later tile/attribute/X byte can therefore be interpreted as a Y coordinate.
The phase fixture deterministically proves the ordinary ninth-sprite result,
an all-out-of-range diagonal walk, and a false positive caused by a later tile
byte. `mynes.core.ppu-smoke` passes on shared x64/x86 at this checkpoint. This
models the documented overflow address sequence only; secondary-OAM per-dot
clearing, `$2004` timing and fetch bus behavior remain open S2 members.

P9 installs the production `v` scroll transitions used by the PPU clock while
rendering is enabled: coarse X wraps/toggles at tile boundaries, fine/coarse Y
increments at the visible-line boundary, dot 257 copies horizontal `t` bits,
and pre-render dots 280--304 copy vertical `t` bits. The direct receiver
exercises each transition, including the combined X/Y boundary, on shared
x64/x86. The current direct pixel sampler remains separate from a complete
background shifter/fetch implementation, so P9 does not claim fetch-bus or
mid-tile scroll accuracy; those are explicit remaining S2 members.

The P9 product-range regression also exposed a stale integration expectation:
the accepted T11 `OUTPUT_SET` operation was still asserted unsupported in the
debug workflow fixture despite its production implementation. The fixture now
submits a zeroed valid Console request, verifies the successful version-1
response, and separately verifies invalid output mode rejection. This is a
receiver correction for existing functionality, not an expansion of T12's
presentation scope.

P10 closes the R07 mutable-hook alternate path. `core_machine_set_cycle_hook`,
its public callback type, private callback state and bus-slot invocation are
removed. CPU, reset, interrupt, DMA and PPU timing now advance only through the
machine-owned bus slot; external IRQ/NMI levels remain explicit wiring inputs
and PPU NMI remains machine wiring. The interrupt receiver retains direct-line,
priority and entry-stack proofs but deliberately no longer injects an arbitrary
callback during a bus slot. Focused x64/x86 interrupt tests pass at this
checkpoint; full product-range regression remains required before acceptance.

P11 joins direct background sampling to the production scroll state. A visible
line snapshots current `v`, rather than reading temporary `t`; a visible-period
`$2000`/scroll write therefore waits for the next copy interval before changing
the sampled base. Because the current renderer is still a direct sampler, it
records the two-tile end-of-line prefetch and compensates that offset when
choosing the displayed pixel; a later full shifter/fetch pipeline replaces this
explicit bridge. The Core receiver proves `t`-versus-`v`, vertical progression
and wrapping. The project-owned motion ROM waits for two initialized frames and
again proves Window and Console motion on shared x64/x86. No complete fetch-bus
or mid-tile-write accuracy is claimed.

P12 fixes warm-reset PPU line state. The reset now clears vblank, the exported
NMI line, the one-dot vblank suppression marker and the shared write toggle,
while preserving palette/OAM memory as the declared warm-reset policy requires.
The PPU receiver seeds each member and verifies the preserved and cleared sets;
the existing machine reset receiver continues to prove the seven reset bus
transfers, warm elapsed-time preservation and power-reset reinitialization on
shared x64/x86.

P13 separates sprite evaluation from display by one scanline. Visible-line
dot 0 evaluates the next line into a private next-selection buffer, and dot
256 commits that selection for the following line's pixel sampler. The direct
receiver proves line zero has no active y=0 sprite while its next buffer has
sprite zero, then proves the dot-256 commit before line one samples it. Indexed
overflow remains attached to the evaluation phase. This is the adjacent-line
selection/fetch boundary; secondary-OAM clear/read timing and per-dot sprite
fetch buses remain open S2 work.

P14 gates sprite evaluation on the real rendering-enable condition: either
background or sprites must be enabled. The nine-sprite vector proves no overflow
is created while both layers are disabled, then proves overflow is produced by
the same OAM when background-only rendering is enabled. This retains the
documented behavior that sprite evaluation can run while the sprite layer is
hidden, without pretending to model every rendering-toggle corruption case.

## S2 Acceptance And S3 Admission

S2 is accepted through local commit `c889739`. Its direct production-path
receivers cover resumable 513/514-slot OAM DMA and partition equivalence, the
immutable shared palette, original mapper-0 Window and text-frame motion,
the dot-0 vblank suppression case, indexed overflow behavior, production
`v` increment/copy transitions, removal of the mutable clock-hook bypass,
warm-reset PPU line state, next-line sprite selection, and rendering-enable
gating. The current x64 registered range 44--100 (57 tests), x86 product range
63--100 (38 tests), and documentation governance gate pass at the acceptance
checkpoint. The unchanged Lib x64 type-layout self-test (#43) is outside that
product range and is not claimed as a new full-suite result.

S3 is automatically admitted under the owner's standing M3 execution approval.
It owns the still-open precision work rather than reclassifying it as accepted:
timed background nametable/attribute/pattern fetches and shifters replacing the
private direct-sampler prefetch compensator; secondary-OAM clear/read and
sprite-pattern fetch phases; and `$2002`/`$2000` NMI behavior at the remaining
same- and later-slot CPU/PPU boundaries. It may add only product-owned Core and
owned Core/integration tests. App input, native presentation, APU/DMC, PAL,
new mappers, Lib/Common changes, and commercial-ROM qualification remain out
of scope. Each state member needs a direct production-clock positive and
negative receiver before S3 can close.

## S3 Implementation Record

P1 introduces private timed background-fetch latches for nametable tile,
attribute quadrant and both pattern planes. On rendering scanlines the PPU fills
them at dots 1, 3, 5 and 7 from the current `v` address; the existing direct
pixel sampler is still the consumer while the paired shifters are introduced in
the following P. The Core receiver supplies distinct tile, attribute and pattern
bytes and asserts every latch at its fetching dot on both product architectures.
While adding that receiver, the attribute quadrant shift was corrected to use
the `v` coarse-X bit (`v & 2`) rather than a shifted address bit. No pixel path
has switched yet, so this checkpoint is an observable pipeline foundation, not
a claim of fetch/shifter completion.

P2 adds the paired 16-bit pattern and attribute shifters. Each eligible dot
shifts every register; dots congruent to one load the previous tile's fetched
pattern bytes and replicated two-bit attribute into the low halves before the
next nametable read. The direct sampler remains the output consumer for this
checkpoint, so the old image contract is preserved while the complete input
state for shifter-based pixels becomes production state. The Core vector checks
the load-plus-shift ordering and both product builds retain the original
pad-controlled motion-ROM proof.

P3 corrects P2's initially inverted shifter direction after rechecking the
registered PPU rendering source: fetched pattern and replicated attribute bytes
load the high halves at each eighth dot, while a later pixel consumer selects
from the low halves. The direct sampler remains active, so this correction is
confined to newly introduced private state. The direct PPU vector proves tile,
attribute and both pattern bytes followed by the high-half load and shift;
the original motion ROM still passes on x64 and x86.

P5 corrects the serial direction itself: high-half data shifts toward the low
half because fine-X samples the latter. This keeps the fetched byte in the
register for its eight-pixel lifetime instead of shifting it out on the first
dot. The direct vector now verifies the exact post-load-and-right-shift values;
the output route remains the direct sampler until the pre-render alignment
vector has proved which low-half bits belong to each visible dot.

P6 adds a self-owned NROM-128 integration oracle for that later output change.
It writes two deliberately different, eight-bit CHR rows through CPU/PPU MMIO,
places them at the first nametable boundary, and creates a fresh ROM for each
fine-X value from zero through seven.  The published Window pixels at the left
edge and across that boundary must equal the independently calculated serial
bit stream.  This is registered as `mynes.integration.ppu-timing-rom-smoke`.
It deliberately records the output contract while the direct sampler remains
the implementation: passing P6 is not a claim that shifter consumption or the
four-dot display tail has been implemented.

P6 adds that pre-render receiver. Two deliberately different tiles run through
dots 321--336; the vector proves that the second fetch has entered the high
half while the prior tile's remaining bits stay in the low half. This fixes the
starting state for scanline zero independently of the direct output path and
gives the later pixel-selection change a tile-boundary oracle.

P7 gives scanline selection an explicit secondary-OAM representation. Evaluation
initializes the 32-byte next buffer to `$ff`, copies each selected primary-OAM
entry in order, and dot 256 commits both the selected-index list and its copied
bytes together. The renderer still samples its retained primary indices in this
checkpoint; the new buffer is the owned state that later secondary-OAM clear,
read and pattern-fetch phases will consume. The receiver verifies copied Y,
tile, attribute and X data at the commit boundary.

P8 switches the sprite pixel sampler to committed secondary OAM while retaining
the parallel selected primary index only for sprite-zero identity. A direct
receiver changes primary OAM's tile byte after dot-256 commit and proves the
visible line still uses the committed tile/palette result. This removes the
visible-period primary-OAM reread from the product path; per-dot secondary-OAM
clear/read and sprite pattern-fetch timing remain open S3 work.

P9 adds the missing production PPU-to-CPU NMI receiver. A real CLI fetch starts
at scanline 241 dot 0, so its three PPU-dot bus slot crosses vblank dot 1; the
test then proves the PPU line reaches the Machine NMI latch and the next
interrupt service pushes `$8001` before entering the NMI vector at `$9000`.
This is distinct from the existing direct external-NMI and PPUSTATUS suppression
vectors. Same-slot CPU `$2002` sampling remains constrained by the current
bus-access ordering and stays open S3 work.

## S3 Acceptance And S4 Admission

S3 is accepted through local commit `92ab3fc`. It delivers timed background
fetch latches and correctly directed high-to-low shifter state, a pre-render
two-tile alignment receiver, committed secondary OAM consumed by the sprite
pixel path, and a production bus-slot PPU-to-CPU NMI-vector receiver. The
registered x64 range 44--100 (57 tests), x86 range 63--100 (38 tests), and
documentation governance gate pass at its acceptance checkpoint.

S4 is automatically admitted under the owner's standing M3 execution approval.
It receives only the unaccepted precision mechanisms: delayed PPU sample to
screen-X output including the four-dot line tail; consumption of background
shifters in that output path; dot-phased secondary-OAM clear/evaluation and
sprite pattern fetch; and the remaining same-slot `$2002`/NMI case. These are
not claimed by S3. App input, native presentation, Lib/Common changes and
commercial-ROM qualification remain out of scope.

## S4 Implementation Record

P1 corrects the production same-slot PPUSTATUS boundary. At scanline 241 dot 0,
the `$2002` CPU read now observes and clears the register before its own bus
slot advances the PPU through dot 1. The PPU's existing suppression latch then
prevents vblank/NMI during that slot. The production receiver proves the bus
read leaves vblank clear, the PPU and Machine NMI lines deasserted, and no CPU
interrupt service pending; ordinary bus reads retain their existing slot order.

P2 repairs the complementary PPUCTRL boundary. A `$2000` write that enables
NMI after its own bus slot enters vblank must immediately propagate the PPU
line to Machine, rather than waiting for a later transfer. The production
receiver begins at dot 0 with NMI disabled, writes `$2000=$80`, then proves
the raised PPU/Machine lines and normal `$FFFA/B` entry with the pre-write PC.

P3 adds the opposite PPUCTRL transition. A production `$2000=$00` write clears
the PPU contribution immediately; the receiver also holds an independent
external NMI line and proves that composition keeps the Machine line asserted
while only the PPU contribution is removed.

P4 moves secondary-OAM state into its documented broad dot phases without
claiming a byte-accurate primary-OAM evaluator yet. On every enabled visible or
pre-render scanline, dots 1--64 clear the 32-byte next secondary OAM buffer at
one byte per two dots; dot 65 then performs the existing ordered selection and
overflow computation for the following scanline. Dot 256 commits the selected
indices and copied OAM on both visible and pre-render scanlines, so scanline
zero receives the same pipeline as every later visible line. The direct Core
receiver seeds the buffer, proves its first and last byte are cleared by the
window, then proves selection and commit. Existing overflow receivers move to
the dot-65 phase, while sprite pixel and original motion-ROM receivers retain
their assertions. This checkpoint deliberately leaves per-primary-byte
evaluation and sprite pattern-fetch slots open.

P5 supplies that sprite-pattern-fetch stage. After dot-256 selection commit,
each 8-dot slot in dots 257--320 fetches the selected secondary-OAM sprite's
two pattern planes into private output buffers; visible pixels consume those
buffers rather than rereading CHR per pixel. The direct vector drives clear,
evaluation, commit and all eight fetch slots, proves the fetched planes, then
proves the next visible line retains its committed sprite after primary OAM is
changed. x64/x86 PPU and original motion-ROM receivers pass.
