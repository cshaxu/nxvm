# T375 S10: 6845 Logical-Raster Admission

`M5:T375:S10:CRTC-RASTER-ADMISSION:OK`

## Primary Contract

The [Motorola MC6845 data sheet](https://www.bitsavers.org/components/motorola/_dataSheets/6845.pdf)
states that the CRTC owns programmable horizontal and vertical timing, display
enable, refresh-memory addressing and row addressing. Its clock is normally
the character rate, derived externally from a dot-rate counter. The primary
register contract is therefore a logical character-clock and scan-line state
machine, not a CGA board-frequency conversion.

| Registers | Primary unit/effect | Current VADP disposition | S10 allocation |
| --- | --- | --- | --- |
| R0--R3 | Horizontal total, displayed characters, sync position and width; all in character-clock units. R0 is total character times minus one; R1 is displayed characters. | Unsupported and unreadable. The synthetic three-window timing does not contain scan lines. | CGA-only logical-raster mechanism: accept masks and derive horizontal display/blank state from delivered logical character clocks. |
| R4--R7 | Vertical total is character rows minus one, R5 adds scan lines, R6 is displayed rows, and R7 is vertical-sync position. | Unsupported and unreadable. | Same mechanism: derive rows, scan lines, displayed region and logical vertical-sync window. |
| R8 | Interlace and skew control. | Unsupported. | Preserve as an explicitly unsupported interlace/skew row until a selected CGA interlace/output contract exists. |
| R9 | Maximum raster address in scan-line units. | Unsupported. | Same mechanism: use non-interlaced `R9 + 1` scan lines per character row. |

The MC6845 also states that vertical sync is fixed at 16 scan-line times and
that the vertical-total-adjust adds programmed scan lines. Those facts can
describe logical state windows after a character-clock source exists; they do
not establish a Model-339 CPU-tick or oscillator conversion.

## Repository Sweep

`vadp.c` currently accepts only R10--R15 plus EGA-only index `13h`; thus
R0--R9 writes are discarded and reads return zero. `raster_phase` advances in
the synthetic profile value `{ active_display_ticks, horizontal_blank_ticks,
vertical_retrace_ticks }`, whose period is a one-dimensional window and cannot
represent the MC6845 scan-line hierarchy. The reset path initializes only
cursor R10/R11; the generated QDCGA firmware writes R10--R15 and never
programs R0--R9. Current VADP text snapshots use independently retained
`columns`/`rows`, not CRTC display geometry.

The IBM CGA reference establishes that its Motorola 6845 is the raster-scan
controller and that port `3DAh` status bit 0 is safe regeneration-buffer
access while bit 3 is vertical retrace. It does not supply the complete CGA
timing-generator division or a mapping from NXVM's 8 MHz retirement/source
ticks to the MC6845 character clock. The PC/AT 14.31818 MHz color-burst
oscillator fact likewise does not supply that mapping.

## Next Receiver

T375 S11 may implement one VADP-owned **CGA logical-raster** state mechanism,
limited to non-interlaced R0--R7/R9 storage, documented masks, logical
character/scan-line counters, reset/mode-change semantics and status/snapshot
proof. It must either receive a separately declared logical-character-clock
source or retain the existing synthetic phase only as a compatibility route;
it may not claim that route is a Model-339 physical cadence. R8, exact CGA
mode defaults, point-clock divisions, CPU/ISA contention, monitor retrace and
physical phase remain T375 timing evidence/implementation receivers.

No ROM, guest medium, firmware image, binary, external source code or
reference-emulator observation was imported.
