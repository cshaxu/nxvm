# M5 T466 Core VADP Phase Contract

## Active Record

T466 owns the selected VADP CRTC/raster/status/aperture phase contract. Its
source and code-ledger reconciliation completes before any implementation batch.

## S1 Accepted Evidence

`d6615ea2` reconciles all fifteen VADP source/code rows. Rendered IBM EGA
pages are the only IBM-EGA authority; 86Box, Bochs, PCjs, MAME and QEMU are
recorded with their actual limits (card variants, later VGA models or no
selected IBM-EGA device). Every row now names Manual L3, bounded Other/board
L3, or fallback to L2 and assigns any current code gap to the existing single
Core VADP path. No source, asset, display consumer, scheduler, parser or ABI
was added.

## S2 Accepted Evidence

The IBM EGA external-register subset now remains in the existing `vadp.c`
owner: Miscellaneous Output resets to zero and selects mono/color CRTC and
Status-1 routing; both Feature Control write addresses share one field; Input
Status 0 reports only the Core-known CRT display state. The new
`core-machine-ega-external-port-smoke` proves reset, port selection, feature
state and Status-1 attribute-phase cancellation. The seven affected CECG and
VADP smokes pass with the same address values but distinct personality
semantics. Switch sense, feature pins and board decode remain L2; no renderer,
scheduler, display-memory path, public ABI or source import was added.

## S3 Accepted Evidence

The existing `ega_planar_active` predicate is the sole operational sequencer
gate: either Reset bit clear makes the provider high impedance, so the frozen
memory resolver falls back to ordinary RAM and EGA capture is unavailable;
both bits set restore the original planar provider and retained plane contents.
`core-machine-ega-planar-port-smoke` proves both reset forms, fallback and
recovery; sequencer, mode-10, external-port and VADP status smokes pass.
Clocking Mode propagation, asynchronous-reset DRAM loss and Core tick/board
conversion remain explicit L2. No memory route, scheduler, API or state mirror
was added.

## S4 Accepted Evidence

The existing VADP CRTC bank now spans IBM EGA 00h--18h. Its one
personality-aware access path retains the CGA rules, applies the IBM EGA masks,
allows readback only for 0Ch--0Fh and returns the explicit L2 zero light-pen
input for 10h/11h; all other EGA write-only reads no longer echo stored state.
The extended CRTC smoke proves every mask, access class and reset state; the
planar, Mode 10, CECG and CGA status regressions pass. Raster tick conversion,
vertical IRQ delivery and physical inputs remain L2. No scheduler, renderer,
memory route, public ABI or compatibility readback path was added.

## S5 Accepted Evidence

The sole VADP planar provider now implements IBM EGA Graphics Controller
00h--08h modes 0--2: rotate/set-reset/logical/bit-mask writes, latch-copy and
processor-bit expansion writes, and read-map or Color Compare reads.  Test
Condition is the same provider's high-impedance gate.  Graphics Controller
data reads now return zero because the IBM registers are write-only; private
state no longer leaks through false readback.  The detailed source ledger
retains L2 for unassigned read maps, Odd/Even/Shift Register, invalid mode 3,
serializer clocks and board arbitration.  Focused planar/controller, EGA
CRTC/mode-10, CECG and VM-system smokes pass; no renderer, memory path, API or
compatibility route was added.

## S6 Accepted Evidence

The existing VADP Attribute-port phase machine now accepts only 00h--13h,
keeps its existing palette and plane consumer, applies the IBM masks and
returns zero from 3C1h rather than mirroring write-only state.  IBM's
Horizontal Pel Panning page itself repeats index 12h; the read-only 86Box,
QEMU and Bochs comparison agrees on 13h, so that one index is explicitly
Other L3 rather than mislabelled Manual L3.  The 14h--1Fh range, external
status/color pins, monitor effects and panning/raster conversion remain L2.
Focused EGA controller/planar/CRTC/mode/external, CECG and VM-system smokes
pass.  No renderer, route, ABI or compatibility state was added.

## S7 Accepted Evidence

Rendered IBM pages establish only the 64/128/256-KiB capacity facts and four
aperture locations.  The manual does not define the exact CPU address formula
for the 128-KiB aperture's extended banks; 86Box's comparable branch also
marks that mapping for real-hardware confirmation.  The audit therefore
corrects the former vague code-gap claim to explicit L2 for the unselected card
capacity and extended-bank mapping, while retaining Manual L3 for the facts
the source actually states.  The future receiver remains the one VADP
construction/provider route; no guessed mapping, card profile, memory path or
state was added.

## S8 Accepted Evidence

Rendered IBM pages 5--10 distinguish the retained CGA-compatible modes from
EGA D (`320x200x16`), E (`640x200x16`), 10h (`640x350` four-plane graphics)
and F (`640x350` chained monochrome) layouts. The finite F2 ledger now records
those Manual L3 facts and the precise current gap: VADP's Offset-13h-only
classification cannot distinguish E from 10h, while the generated-firmware
mode-10 VM replay advertises `5010h` but never observes the expected VADP
frame. Reverting the rejected CRTC-geometry experiment reproduces that
failure, so it is baseline evidence rather than a new regression. The next S
owns one repair across the existing generated-firmware port route and the sole
VADP snapshot path; BDA coupling, a second renderer and a second VRAM route
are excluded. F chained odd/even and alpha character generation remain
separate later batches.
