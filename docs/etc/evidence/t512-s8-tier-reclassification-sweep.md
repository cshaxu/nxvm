# T512 S8 CPU timing-tier reclassification sweep

## Decision

The owner approved correcting every discovered false Manual-L3 classification.
Manual-L3 requires an exact numeric result or executable formula. A manual
range, an external-emulator model, or a delegated coprocessor duration is L2.

## Five-profile sweep

| profile | result | disposition |
| --- | --- | --- |
| 8086 / 8088 | Group-3 ranges already use the named `L2:G3` model; ESC records only the manual's exact CPU issue cost | no correction |
| 80186 | documented ranges already use `L2:midpoint`; ESC records the manual's exact CPU issue cost | no correction |
| 80286 | Appendix-B rows recorded by the result artifact are exact/formula CPU rows; ESC records the exact CPU-side row | no correction |
| 80386 | `I386-ESC` recorded a 12--26 MCP-clock 80387 data-sheet interval as L3; `I386-WAIT` recorded its manual 7-minimum row as exact | both corrected to L2.  The same global sweep found and repaired the shared FPU-owner false completion; its source-qualified Core deadline path is recorded in `t512-s8-fpu-wait-global-sweep.md`. |

`WAIT` and `HLT` were initially found by a broad source-text search because
they shared an old manifest row with ESC. Visual inspection of the original
WAIT page shows `7 minimum`, so WAIT is L2; HLT's five clocks are exact and
remain Manual-L3. Splitting that manifest row prevents either qualification
from leaking into the other instruction's provenance.

The correction preserves the Core-owned ESC/FPU command handoff and does not
invent a CPU tick count for coprocessor completion.
