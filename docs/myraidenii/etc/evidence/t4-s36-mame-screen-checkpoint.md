# M1 T4 S36 — Neutral MAME Screen Checkpoint

## Boundary

This record uses MAME's public Lua screen API as an external black-box only.
It retains a visible size and a non-reversible byte-stream checksum; it does
not retain pixels, screenshots, ROM bytes, registers, RAM, traces or MAME
implementation details.

## Probe

A transient Lua file outside tracked source registered a frame-done callback.
At emulated frame 600 it called `screen:pixels()`, calculated Adler-32 over the
returned byte string, printed screen tag/width/height/checksum and scheduled
`machine:exit()`.  The script was neither an input to MyRaidenII nor a source
dependency.

Both runs used current MAME `0.289 (mame0289)`, S35's two external read-only
ROM roots, `-video none -sound none -nothrottle -skip_gameinfo -nowriteconfig`
and `-nonvram_save`.

| Run | Frame | Screen | Width | Height | Adler-32 |
| --- | ---: | --- | ---: | ---: | --- |
| 1 | 600 | `:screen` | 320 | 240 | `da88bdb4` |
| 2 | 600 | `:screen` | 320 | 240 | `da88bdb4` |

The identical results establish a bounded, repeatable black-box checkpoint.

## Disposition

The selected set produces a 320×240 visible screen at this checkpoint.  That
agrees with the existing declared native-screen target, but does not establish
graphics ROM layout, decryption, tile/sprite order, priority, scroll,
brightness, palette wiring or a MyRaidenII native-frame contract.  A later
Video implementation must gain its own complete behavior contract and cannot
substitute this checksum for decoder evidence.

The temporary Lua script and any MAME-generated transient output are removed
at S36 closure.  No MyRaidenII source, test, manifest, asset archive or
executable changed.
