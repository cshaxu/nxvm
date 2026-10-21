# M1 T7 S64 — Sprite Blend Contract Research Gate

## Scope and source grade

This is a source-policy-limited L2 research review.  It does not change the
compositor, attach a palette table, inspect protected media, or claim frame
fidelity.

The review uses the existing T4 L2 evidence and the current local pipeline.
Those records establish the sprite-flags latch and ordinary priority/transparent
pen behavior, but they expressly retain blend, shadow and flag decoding as
unresolved.

## Local ownership

Board owns byte-lane mapping for the two-byte sprite-flags destination.  Video
owns latch assembly and reset.  The sole compositor owns sprite ordering,
transparent-pen preservation and the existing four ordinary priority phases.
No second compositor or Product/Machine pixel path exists.

The latch is intentionally not consumed by frame composition.  That is the
correct current state because the following required semantics are absent:

- which flag patterns select ordinary, blend or shadow behavior;
- the source and destination colour transformation for each selected behavior;
- priority interactions with the existing tile phases; and
- reset, unsupported-value and failed-frame behavior for a flag-dependent
  composite.

The available L2 records identify that some special composition exists, but
do not establish those complete rules without importing external tables or
deriving behavior from protected output.

## Disposition

No finite Sprite blend receiver is admitted.  The existing TODO remains
correct: the current ordinary compositor is retained, and the flags latch is
not given a guessed effect.  A later Video receiver needs a complete bounded
L2 or L3 rule for flag classification, colour transformation, layer ordering,
reset/failure behavior and synthetic non-ROM pixel oracle before modifying the
sole compositor.

## Verification

The review reconciled current Board/Video ownership with T4 sprite-latch,
compositor and ordering records.  No external source code, palette, ROM or
pixel output was opened or retained.  No production/test/build/manifest/API or
asset changed, and no runtime, desktop or gameplay claim is made.
