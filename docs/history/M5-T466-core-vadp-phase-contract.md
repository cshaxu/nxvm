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
