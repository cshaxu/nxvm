# M1 T7 S37 — Sprite-Protection Source Trigger

## Scope and evidence grade

S29/S30 already established the local Board ownership and directions of the
source-segment, source-offset, destination, X/Y/max-X, trigger and destination
output endpoints.  S37 connects only that existing finite endpoint set.

The reference fact is **L2**: the inspected external emulator identifies one
trigger that derives a source address from the already modelled segment/source
latches, tests one descriptor against the existing bounds, updates its flag
and conditionally emits a compact record at the current destination.  The
local implementation was written independently and imports no external source,
tables, ROM payload or hardware-accuracy claim.

## Local model

A completed mapped write to the existing `0x6de--0x6df` trigger latch sets one
private Board pending bit.  It cannot execute the service inside that memory
provider callback because the CPU already owns that Core transaction.  The
existing Board deadline callback consumes the bit at its next safe point using
the existing Core DMA cycle.

The service reads one descriptor from `segment << 4 + source`, applies the
existing signed X/Y/max-X bounds, updates the descriptor's low visibility bit,
and writes `head-1`, `head-2`, left and top only when visible.  It advances the
existing destination by eight bytes only after all four writes succeed.  A
rejected descriptor preserves the destination and output latch.  A DMA failure
uses the pre-existing Core fault path and cannot report a completed advance.

This is deliberately not a generic COP queue, a renderer, a multi-object
walker, an API extension or a second timing path.

## Verification

The existing Board smoke covers the real mapped trigger path with:

- an accepted descriptor: flag update, four-word output and one eight-byte
  destination advance;
- a rejected descriptor: flag clear, output/destination preservation; and
- a source in the existing rejected control window: Core becomes `FAULTED` and
  the public advance call returns its existing `LIB_STATUS_INTERNAL_ERROR`.

The affected MyRaidenII unit label passed on both stable Ninja trees (12/12
per width).  The bounded 20,000,000-instruction owner-ROM vblank Coin/Start
probe also completed on both widths without a CPU/COP/control fault.  Both
widths agree exactly on its final non-payload summary:

```text
executed=20000000 ticks=324904898
frame-width=320 frame-height=240 nonblack=74865
frame-digest=5ca84b4d9994fb4c
video-vblanks=1125 video-crtc-writes=12668
video-sprite-flag-writes=8 video-sprite-latches=1045
```

The Probe stages owner media only under `O:\assets\raidenii\myraidenii-diagnostics`
and the directory was empty after both runs.  No gameplay, title screen,
sprite fidelity, PAL behavior, desktop/RDP behavior or release qualification
is claimed by this bounded result.
