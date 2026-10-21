# M1 T7 S13 — Raster Cadence Reconciliation

## Scope and grade

This is an **L2-only** cross-check.  The external FPGA project is an
independently inspected implementation, not a primary manual and not an
import source.  No external HDL, code, table, ROM or generated output was
copied, translated, linked or retained.

The owner-defined grading rule controls this record: an explicit fact in an
inspected primary-manual PDF would be L3; an external implementation or board
observation is L2; an unsupported local choice is L1.

## Inputs

The local owner is `src/myraidenii/core/video.c`:

```text
local Board axis: 16,000,000 ticks/s
line:             512 pixels x 2 Board ticks/pixel = 1,024 ticks
frame:            1,024 ticks/line x 282 lines = 288,768 ticks
visible/VBlank:   240 visible lines
```

Thus the local line duration is `1,024 / 16,000,000 = 64 us`, and its frame
duration is `288,768 / 16,000,000 = 18.048 ms` (about 55.41 Hz).

The inspected public external reference, `spacestate1/Arcade-Raiden2_MiSTer`
README at its main branch on 2026-10-20, reports independently:

```text
pixel clock:       8 MHz
line:              512 pixels
frame:             282 lines
visible:           240 lines
internal renderer: 4,096 cycles/line at 64 MHz
```

Its pixel-clock line duration is `512 / 8,000,000 = 64 us`; its frame duration
is `512 x 282 / 8,000,000 = 18.048 ms`.  Its separate internal-renderer
duration is `4,096 / 64,000,000 = 64 us`.  All three forms agree.

## Disposition

The current local raster cadence has the same L2 duration as the external
reference.  `4096` is not a replacement for the local `1024`: it counts a
different 64 MHz internal clock, whereas the local model owns the same elapsed
line duration on its 16 MHz Board axis.  Replacing the local value would
quadruple the modeled line duration and be a regression.

No source, test, public API, asset, build or release artifact changes are
justified.  This does not qualify physical timing, sprite rendering,
sprite-blend selection, COPX-D2 behavior, real-set startup or gameplay.

## Source boundary

The external project is GPL-3.0 and is used only as the allowed L2 behavior
reference.  The facts above are published clock/topology statements; no HDL or
its tests were used as an implementation template.  The canonical public page
is <https://github.com/spacestate1/Arcade-Raiden2_MiSTer>.
