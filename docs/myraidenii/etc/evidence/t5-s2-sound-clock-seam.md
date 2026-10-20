# M1 T5 S2 — Board Sound Clock Seam

One private `myraidenii_core_sound` object now belongs to the MyRaidenII Board.
The sole existing Board attachment callback advances it before Video and COP
work; Board reset clears it. No public Board operation, Shared interface,
worker, host clock or PCM route changes.

The bridge converts the Board's existing 16 MHz axis to the selected sound
Z80's L2 `28.636363 MHz / 8` rate with a retained integer remainder:

```text
z80_ticks += floor((board_ticks * 28,636,363 + phase) / 128,000,000)
phase      =      (board_ticks * 28,636,363 + phase) % 128,000,000
```

The multiplication is split into quotient/remainder terms before evaluating
the bounded phase product, and detects delivered-tick overflow. If the latter
ever occurs, Board reports the existing Core fault rather than wrapping guest
time. Current normal execution has no sound CPU yet: S2 deliberately does not
read, acknowledge, overwrite or complete any Board command/interrupt latch.

The new local `sound_smoke` proves eight equal Board slices deliver exactly one
full Z80-second numerator, verifies the retained fractional phase immediately
before that boundary, reset behavior and invalid arguments. Existing Board
transport smoke continues to own main/sound latch and interrupt semantics.
Both cached x64/x86 builds and all six MyRaidenII unit rows pass. This is the
single time seam consumed by later Z80 and chip work; it is not an audio stub
or a claim of audible output.
