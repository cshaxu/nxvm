# M1 T4 S22 — Sprite-Protection Register Group

## Basis and boundary

After S21, both selected-set probes stopped on a byte write at `0x6c0`.
The inherited Raiden II address map distinguishes reset-visible sprite-protection
endpoints at `0x6c0--0x6c3` from a write-only destination endpoint at
`0x6c6--0x6c7`; S21 independently owns the intervening constant no-op at
`0x6c4--0x6c5`.

- Reference: [Raiden II address map](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp)

This evidence establishes endpoint ownership and direction only.  It supplies
no protection algorithm, source interpretation, destination meaning, sprite
decryption, rendering behavior, or hardware timing.  No reference code, table
or ROM payload was imported.

## Local model

The Board has three reset-zero `lib_u16` latches: readable offset and source
segment pairs at `0x6c0--0x6c1` and `0x6c2--0x6c3`, plus a write-only
destination pair at `0x6c6--0x6c7`.  Contained byte and word accesses preserve
little-endian lanes.  Reads and inspection of the destination remain refused.

The already-existing Core memory contract splits a wider access across its
separately routed byte endpoints.  Therefore the `0x6c5` two-byte test retains
that contract: its no-op byte and destination byte are individually accepted.
There is no Board-local crossing-access framework or rollback path.

## Verification

The Board smoke covers readable byte/word lanes, reset, write-only destination
refusal, S21's independent no-op and the existing wider-access behavior.  The
former fallback-failure sentinel moved from newly-owned `0x6c0` to the still
unowned `0x400` control-window byte.

Both selected-set probes now advance through the group and reach the same next
terminal result:

```text
MYRAIDENII:STARTUP-PROBE:OK reason=7 detail=2147483648 fault-code=1788
```

The result is only the next bounded stopping point, not a playable-game claim.
x64 and x86 Board smokes pass.  No public API, Shared component, product
artifact, desktop/RDP behavior or protection implementation changed.
