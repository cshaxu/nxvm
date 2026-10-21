# M1 T7 S10 — Sprite Blend Contract Review

## Local ownership audit

The Board owns one 16-bit `0x6ce--0x6cf` sprite-flags latch and writes it only
through `myraidenii_core_video_sprite_flags_write()`.  Video resets and stores
that value, but the native RGB32 compositor has no reader or alternate blend
path.  The existing unit verifies byte order and reset only.  Thus there is
one clean owner and no hidden duplicate implementation to merge.

## External reference result

The lawful external research copy
`O:\repos.external\mame-raiden2-l2\raiden2.cpp` was inspected on 2026-10-20
with SHA-256
`fdde57f6e9024657f2a9830864523305139562229750b87c967e597324cdb26a`.
It identifies a conditional half-alpha blend operation for a reference-specific
set of palette entries.  That is an **L2** behavioral observation, not a
hardware fact or an L3 source.  More importantly, the condition is controlled
by a large reference-owned colour-membership table; the current inspection does
not establish a Board sprite-flag bit, palette-index rule, reset rule, or any
non-ROM oracle that selects those entries on the target board.

The table and external implementation are not copied, transliterated, linked
or retained by this repository.  A generic half-alpha operation without its
selection contract would be L1 guesswork and would render the same sprites
differently from the reference in unbounded ways.

## Disposition

No finite implementation contract exists.  The existing TODO receiver remains
correctly deferred: a later packet needs a direct board/manual rule (L3), or a
bounded independently observable L2 contract that provides the active
condition, inputs, output rounding, reset/failure behavior and a synthetic
non-ROM oracle.  S10 makes no source/API/test/asset change and does not claim
sprite fidelity or selected-set gameplay qualification.

