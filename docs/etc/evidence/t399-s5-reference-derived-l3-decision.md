# T399 Final Reference-Derived DeskPro L3 Decision

## Decision

**Reference-derived Model-L3 is not ready. Physical Model-L3 remains not ready.**

The frozen five-batch universe is exhausted. B2 is accepted as PCjs-only,
project-owned FDC/DMA2/IRQ6 logical ordering; B3 is accepted for the exact
DeskPro port-61 active-low NMI semantics. B1 is non-eligible: no CPU
retirement-to-DCLK/BWAIT signal contract exists. B4 is rejected: the exact
PCjs DeskPro configuration selects a generic IBM Type-5 HDC, not the Compaq WD
40 MB controller. B5 is rejected: PCjs explicitly uses IBM EGA and says Compaq
EGA support is incomplete; no exact CECG reference exists. B1/B4/B5 have the
pre-existing precise TODO receivers.

The full current gate ran 284/284 CTests successfully on 2026-08-17. This
proves the current runnable graph, not the missing physical or reference
contracts. No third-party source, ROM, media, configuration or trace was
imported; no new Core/VM interface or scheduler exists.

## Transfer

T399 is closed. The next Queue candidate is the four-profile CPU
instruction-correctness audit. Any future DeskPro L3 re-audit requires a new
admission with a qualifying exact reference or physical observation; it cannot
use generic AT/EGA/HDC behavior or derive hardware time from reference ticks.