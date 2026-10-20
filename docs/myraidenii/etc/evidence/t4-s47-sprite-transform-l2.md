# T4 S47 Raiden II Sprite Transform L2 Model

## Decision

The owner directed MyRaidenII to use mature external implementations as L2
evidence rather than repeatedly block on unavailable physical L3 material. The
selected Raiden II sprite transform therefore uses one narrow, reviewed
BSD-3-Clause exception to the normal no-table rule.

## Provenance

The local read-only checkout `O:/repos.external/mame-raiden2-l2` is pinned at
`ab2d1ca215c6e14dbe83fd2d0a70a4862fd16253`. The reviewed units are:

| Unit | License and holders | Adopted finite fact |
| --- | --- | --- |
| `src/mame/seibu/r2crypt.cpp` | BSD-3-Clause; Andreas Naive, Olivier Galibert | The 512-entry rotation, 256-entry five-bit XOR and 256-entry eleven-bit XOR constant sets, word-index selection, permutation and fixed masks for Raiden II sprite words. |
| `src/mame/seibu/seibu_helper.cpp` | BSD-3-Clause; Ville Linde, Nicola Salmoria | The bounded partial-carry rule: carry crosses a bit only when that carry-mask bit is set, with top carry folded into bit zero. |

The implementation is written in project C11 vocabulary and only retains the
finite constants needed for the selected set. It does not link, include or run
MAME, retain MAME driver code, copy ROM bytes, or assert the original Seibu COP
hardware algorithm. The source itself describes its table relationship to
uploaded hardware tables as uncertain; this model remains L2.

## Local Ownership

`video.c` owns the private lane-interleaved graphics allocation and invokes one
Graphics-private decrypt operation exactly once after all four sprite lanes are
copied. `graphics.c` owns the word transform and the single packed-nibble
sprite-pixel reader. Public callers see only the bounded copied-pixel result;
no caller owns a decoded stream or can mutate Video's allocation.

The model uses little-endian 32-bit words, then packed low-nibble-first pixels
for a 16 by 16 four-bit tile. Character and background decode paths do not
change.

## Limits And Receivers

This does not qualify object visibility, tile extensions/banking, priority,
blend, sprite flags, rotation, selected-game frame output, sound, Product or
real-ROM execution. Those belong to later Video, Machine and Product packets.
