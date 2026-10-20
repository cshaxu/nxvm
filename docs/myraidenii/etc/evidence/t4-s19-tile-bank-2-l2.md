# M1 T4 S19 — Tile-Bank-2 Endpoint

## Basis and boundary

The selected-set probe, after S18, first reached a one-byte Board read at
`0x470`.  The Raiden II reference address map identifies `0x470--0x471` as a
read/write tile-bank-2 endpoint.  This establishes only address ownership; it
does not establish tile selection, rendering, palette, or timing behavior.

- Reference: [Raiden II address map](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp)
- Internal prior disposition: `t3-s15-cop-register-inventory.md` assigns the
  tile-bank family to Board/Video rather than COP.

The model is therefore an explicit L2 Board register, not a physical-hardware
claim and not a translation of the reference implementation.

## Local model

`myraidenii_core_board` owns a reset-zero `lib_u16` at `0x470--0x471`.
Contained byte and word accesses preserve little-endian lanes; ordinary reads
and inspection return the stored lanes.  The endpoint has no renderer effect
and exposes no public API.  Neighbouring `0x472` remains under the existing
control-window refusal path.

## Verification

The existing Board smoke checks initial zero, independent byte lanes, complete
word overwrite, inspect, reset and neighbour refusal.  Both selected-set
probes pass `0x470` and identically stop at the next independent Board write:

```text
MYRAIDENII:STARTUP-PROBE:OK ... control-physical=1740 ... control-access=1
```

`1740` is `0x6cc`; it is not absorbed by this endpoint.  x64 and x86 Board
smokes both pass.  Tile rendering, palette behavior, desktop/RDP behavior and
a playable-game claim remain outside S19.
