# T374 S13: CGA 6845 Register Audit

## Authority And Matrix

IBM's [PCjr technical reference 6845 table](https://www.bitsavers.org/pdf/ibm/pc/pc_jr/PCjr_Technical_Reference_Nov83.pdf)
records the CGA-compatible register purposes, units, access direction and
standard text/graphics initialization values. The [Motorola MC6845 data
sheet](https://www.bitsavers.org/components/motorola/_dataSheets/6845.pdf)
identifies R12/R13 as the 14-bit display-start pair and R16/R17 as the
read-only light-pen pair. These are documentation references only; no source,
ROM or media is imported.

| Index | Function class | Current VADP disposition | T374 disposition |
| --- | --- | --- | --- |
| R0--R3 | Horizontal total/display/sync/width | Ignored. | Requires scan/blank/sync geometry; transfer to 5170 display timing. |
| R4--R9 | Vertical total/adjust/display/sync, interlace, max raster | Ignored. | Requires frame/raster geometry and timing; transfer to 5170 display timing. |
| R10--R11 | Cursor start/end raster | Stored with masks and consumed by text snapshot visibility. | Present selected cursor route; blink/scanline cadence remains timing. |
| R12--R13 | Display-start address | Stored with six-bit high mask and consumed by text snapshot wrapping. | Present selected text start route. |
| R14--R15 | Cursor address | Stored with six-bit high mask and consumed by text snapshot position. | Present selected cursor route. |
| R16--R17 | Light-pen captured address, read-only | Unsupported; no Model-339 light-pen input, latch signal, port, IRQ or firmware/BIOS consumer exists. | Explicitly unselected hardware-input path; do not synthesize zero values. |

VADP currently permits R10--R15 plus index `13h` (decimal 19) for an
EGA-specific offset. `13h` is outside the CGA 6845 R0--R17 table, yet is
currently accepted in the CGA-only profile because the shared VADP has no
profile-specific index gate. Reset supplies the retained text mode plus
cursor start/end defaults; generated firmware programs exactly the retained
cursor/start subset.

## Selection And Transfer

S14 is selected to reject EGA-only CRTC index `13h` on the CGA-only Model-339
route without changing EGA behavior. R0--R9 determine display geometry and synchronization;
storing them without making their documented effects observable would be a
false completion, while making them observable needs the later physical display
timing contract. R16/R17 need a selected light-pen input lifecycle and consumer
before they can latch an address. Therefore the next functional receiver is a
later selected-CGA geometry/state admission after the 5170 board/display timing
contract defines raster ownership; light pen remains unselected input debt.

## Sweep And Baseline

The audit swept VADP index support/masks/reset/word assembly/snapshot uses,
generated firmware writes, profile display selection and all CGA/Model-339
tests. Existing focused CGA/Model-339 replay remains recorded in S11/S12; this
documentation-only audit introduces no code or asset change.

This evidence does not claim full CGA, selected-device closure, timing closure
or Model-339 L3.
