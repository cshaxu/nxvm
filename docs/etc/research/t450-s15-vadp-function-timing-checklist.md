# T450 S15 VADP Original Function And Timing Checklist

## Scope, Source Form And Sufficiency

**S19 verification:** direct rendered-page inspection confirms mixed form:
scanned pages with OCR body text and a cover without text layer.

`VADP` here means the IBM *Enhanced Graphics Adapter (EGA)* chapter in the
admitted *Technical Reference: Options and Adapters, Volume 2* (revised Apr.
1984, EGA chapter pp. 1--84).  The admitted PDF is an archival scan with OCR:
the extracted chapter is usable for headings, prose and tables but includes
recognition errors and revision-date mixtures.  S19 must verify the source
form and cited pages directly against rendered pages; this checklist does not
silently turn OCR text into a new authority.

The chapter is sufficient for IBM EGA programming, memory/display organization
and its documented display-rate relationships.  It is not a specification for
the selected Compaq CECG personality, an arbitrary CGA personality, host bus
arbitration, monitor implementation, or a host-renderer cadence.  Such facts
are L2 or blocked until their selected primary source is admitted.  No
mature-emulator reference is selected; S19 may name one only as labelled
reference-derived investigation for a precisely identified primary-source gap.

## T466 S1 Cross-Validation

The IBM EGA chapter remains the only authority for an IBM-EGA row. Direct
read-only comparison on 2026-08-25 used 86Box `4fef696`
(`src/video/vid_ega.c`), Bochs 2.6 (`iodev/vga.cc`), PCjs PCx86 v2 (no IBM-EGA
device implementation), current MAME (general video core, with no selected
IBM-EGA device evidence) and current QEMU (`hw/display/vga.c`). Bochs and QEMU
model later VGA-compatible state; 86Box has vendor/card variants; PCjs/MAME do
not establish a selected IBM-EGA adapter. None can add an IBM register, clock
or board fact. No reference code is imported.

| ID | Cross-check result | Final source tier |
| --- | --- | --- |
| VADP-R1 | 86Box retains external/status paths; Bochs/QEMU merge them into later VGA state. | Manual L3 IBM ports; selected board decode fallback to L2. |
| VADP-R2 | 86Box, Bochs and QEMU keep sequencer/reset state but choose different rendering and reset timing. | Manual L3 fields/order; board clock propagation fallback to L2. |
| VADP-R3 | 86Box derives CRTC raster state; later VGA models add registers/masking beyond IBM EGA. | Manual L3 IBM CRTC subset; selected raster clock fallback to L2. |
| VADP-R4 | 86Box/QEMU implement planar latch/write logic; QEMU additionally has VGA write-mode semantics. | Manual L3 IBM graphics subset; later variants fallback to L2. |
| VADP-R5 | 86Box and VGA implementations keep attribute flip-flop/palette state; status source differs by adapter. | Manual L3 IBM attribute behavior; external-status wiring fallback to L2. |
| VADP-F1 | Detailed models use planes/apertures, but capacity/decode are card-specific. | Manual L3 IBM plane/map facts; installed card memory fallback to L2. |
| VADP-F2 | 86Box renders EGA modes; later VGA and PCjs use different mode surfaces. | Manual L3 IBM modes; current CECG personality is Other/board L3 only when selected. |
| VADP-F3 | Every implementation owns a different scheduler or clock conversion. | Manual L3 chip values; Core tick conversion fallback to L2. |
| VADP-F4 | 86Box models display timing internally; Bochs/QEMU define no ISA wait-state authority. | Manual L3 allocation rule; board arbitration fallback to L2. |
| VADP-F5 | Reference models expose host renderers rather than monitor electrical interfaces. | Manual L3 signal names; monitor/host boundary fallback to L2. |
| VADP-T1 | Reference cards choose their own ROM/decode integration. | Manual L3 IBM topology; selected board integration fallback to L2. |
| VADP-T2 | Switch/jumper state is card configuration in 86Box; absent from generic VGA paths. | Manual L3 option facts; unselected profile fallback to L2. |
| VADP-T3 | Monitor choice is outside every emulator's hardware oracle. | fallback to L2. |
| VADP-T4 | References keep display scheduling distinct from host presentation, not one shared ISA timing rule. | Manual L3 adapter allocation; board service fallback to L2. |
| VADP-T5 | References have separate host consumer paths. | Current Core/VM ownership is a project boundary; unselected inputs fallback to L2. |

### VADP-R4 Graphics Controller Reconciliation

Rendered inspection of IBM EGA pp. 46--55 is the source for this finite
register grammar.  86Box corroborates the four-plane/latch shape; QEMU and
Bochs use later VGA behavior, notably a two-bit read-map selection and a valid
VGA write mode 3.  Those later details do not extend IBM EGA.

| Index | IBM EGA manual rule | Selected disposition |
| --- | --- | --- |
| 00h--02h | Set/Reset, Enable Set/Reset and Color Compare are four-bit write-only values. | Manual L3. |
| 03h | Data Rotate is rotate count 0--2 plus Replace/AND/OR/XOR. | Manual L3. |
| 04h | Read Map Select is a write-only three-bit value.  The selected four-plane model has no manual-defined recipient for values 4--7. | Manual L3 for maps 0--3; fallback to L2 returns zero for 4--7. |
| 05h | Modes 0--2 define source/read behavior; bit 2 makes graphics outputs high impedance; bit 3 selects map or color-compare reads; odd/even and shift control are specified but require board/serializer context.  Mode 3 is not valid. | Manual L3 for modes 0--2, Test Condition and Read Mode; fallback to L2 for odd/even/shift and invalid mode 3. |
| 06h--08h | Miscellaneous map, Color Don't Care and Bit Mask are write-only values with masks 0Fh, 0Fh and FFh. | Manual L3. |

Every Graphics Controller data read returns zero: the selected IBM manual
declares these registers write-only, and Core does not mirror private VADP
state through a false port readback.

### VADP-R5 Attribute Controller Reconciliation

Rendered inspection of IBM EGA pp. 56--62 establishes the write-only 3C0h
address/data flip-flop and registers 00h--13h: sixteen six-bit palette values,
four-bit Mode Control, six-bit Overscan, four-bit Color Plane Enable and
four-bit Horizontal Pel Panning.  Status-1 read resets the phase.  QEMU and
Bochs expose readback and index 14h for later VGA; neither is an IBM EGA rule.

| Index | IBM EGA manual rule | Selected disposition |
| --- | --- | --- |
| 00h--0Fh | Six-bit write-only palette values. | Manual L3. |
| 10h | Four-bit write-only Mode Control. | Manual L3. |
| 11h | Six-bit write-only Overscan Color. | Manual L3. |
| 12h | Four-bit write-only Color Plane Enable; bits 4--5 select status wiring. | Manual L3 for state/mask; external color pins remain fallback to L2. |
| 13h | The manual's Horizontal Pel Panning page mistakenly repeats 12h, which would collide with Color Plane Enable.  86Box, QEMU and Bochs agree on the distinct 13h four-bit register. | Other L3 for index/value/phase; raster shift remains fallback to L2. |
| 14h--1Fh | No selected IBM EGA register definition. | fallback to L2; ignore writes and return zero. |

The retained 3C1h read route returns zero rather than exposing a false mirror
of write-only Attribute state.

## Register And Programming Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| VADP-R1 | EGA pp. 12--17 | External ports provide Miscellaneous Output (3C2 write), Feature Control (3BA/3DA write), Input Status 0 (3C2 read) and Input Status 1 (3BA/3DA read), with mono/color address selection. | Hardware reset clears Miscellaneous Output; its address-select and RAM-enable state gates the listed interface. | Status 1 exposes display-enable and vertical-retrace state; Status 0 exposes CRT interrupt/switch/feature inputs. | Primary sufficient for IBM EGA external-register behavior: L3. |
| VADP-R2 | EGA pp. 17--24 | Sequencer index/data registers select Reset, Clocking Mode, Map Mask, Character Map Select and Memory Mode. | Async/sync reset halt the sequencer; both reset bits must permit operation, and synchronous reset precedes clocking-mode change when RAM contents must persist. | Character clock width, display-memory cycle allocation and serializer load are Clocking Mode functions. | Primary sufficient for these EGA register/formula rules: L3. |
| VADP-R3 | EGA pp. 24--45, register summary pp. 62--68 | CRTC index/data registers program horizontal/vertical totals, display end, blank/retrace, start address, cursor, underline and mode control. | CRTC register programming changes raster/cursor/retrace behavior; vertical interrupt enable/reset is a CRTC-controlled feature. | CRTC generates sync, display-buffer/refresh addressing and cursor timing from its programmed counts. | Primary sufficient for IBM EGA CRTC programming; actual selected clock and monitor phase remain L2. |
| VADP-R4 | EGA pp. 46--55, pp. 62--68 | Graphics Controller index/data registers supply set/reset, rotation/logical operation, read-map, mode, memory-map, color compare/don't-care and bit-mask behavior. | Register-controlled write/read modes select how processor data reaches bit planes; no separate display-memory writer is implied. | Latches, map selection and write mode relate a CPU memory cycle to plane updates. | Primary sufficient for IBM EGA graphics data path: L3. |
| VADP-R5 | EGA pp. 56--61, pp. 62--68 | Attribute Controller index/data programming supplies palette, overscan, color-plane enable, horizontal-pel panning, color select and mode control; index/data flip-flop is reset by Status 1 read. | Status 1 read cancels the address/data flip-flop state; display output follows current attribute state. | Palette/plane selection drives six color outputs and diagnostic Status 1 bits. | Primary sufficient for IBM EGA attribute behavior: L3. |

## Memory, Display, Signal And Timing Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| VADP-F1 | EGA pp. 1--4, 8--11 | Base display RAM is four 16-KiB planes, expandable to 128/256 KiB; supported maps include A0000 128/64 KiB, B0000 32 KiB and B8000 32 KiB.  Text and graphics assign planes as described. | RAM enable and Graphics Controller map select determine processor visibility; sequencer reset can lose DRAM data. | Sequencer inserts processor memory cycles between display cycles. | Primary sufficient for IBM EGA plane/aperture model: L3; selected installed-memory size is L2. |
| VADP-F2 | EGA pp. 5--11 | BIOS-supported CGA/MDA-compatible and EGA modes define documented resolution, color, buffer, page and plane arrangements, including mode 10h 640x350 planar graphics. | Mode/set-up and map-mask selections determine plane visibility; ROM BIOS assistance is not a Core state owner. | Mode tables distinguish 15.75-kHz and 21.85-kHz display operation. | Primary sufficient for IBM EGA mode facts; not sufficient for a Compaq CECG mode personality. |
| VADP-F3 | EGA pp. 3--7, 12--24 | Two 14- and 16-MHz sources provide dot rate; Miscellaneous Output selects channel/on-board/external clock; sequencer and CRTC generate character, memory and raster timing. | Sequencer reset gates timing generation; unused clock-select encoding is not a selected operational mode. | Manual gives 15.75/21.85-kHz horizontal and 60-Hz vertical display relationships, plus sequencer memory-cycle allocations. | Primary values/formulas are L3 for IBM EGA; conversion to Core ticks and board propagation is L2. |
| VADP-F4 | EGA pp. 3, 15--17, 19--20 | EGA permits processor display-memory access during active display through sequencer-inserted cycles; Status 1 display-enable/retrace permits software observation but is not required to avoid image glitches. | Sequencer reset halts outputs and changes memory preservation conditions. | High-resolution mode requires four of five memory cycles for CRT fetch; medium resolution can use two of five. | Primary sufficient for EGA contention rule; actual ISA/CPU arbitration is L2. |
| VADP-F5 | EGA pp. 1--3, 12--17, 76--86 | Adapter provides direct-drive, feature connector and light-pen interfaces; input status exposes switches/feature/light-pen/CRT state and output controls driver/polarity. | Disable Internal Video Drivers transfers direct-drive source to feature connector; physical inputs are external. | Sync polarity, vertical retrace, display enable and six color outputs define adapter-facing signals. | Primary sufficient for signal names/relations; host-display and selected-monitor electrical behavior are L2. |

## Selected-Adapter And Board Boundary Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- |
| VADP-T1 | EGA pp. 1--4, 12--17, 62--68 | IBM EGA owns its listed register groups, memory-map choices, 16-KiB ROM at C0000 and display memory; it is a card personality rather than a generic display contract. | Adapter reset/register programming affects its own state only; BIOS/ROM lifecycle remains firmware/board ownership. | Primary sufficient for IBM EGA topology; selected PC/AT decode/ROM integration requires board evidence. |
| VADP-T2 | EGA pp. 5--7, 79--86 | Switch/jumper configuration and attached monitor select supported IBM EGA display operation and memory options. | Input Status 0 switch sense observes configuration; no profile may infer switch state from a generic mode number. | Primary sufficient for IBM EGA options, but no selected repository IBM-EGA switch/monitor configuration is frozen: L2. |
| VADP-T3 | EGA pp. 5--7, 12--13 | IBM Color, IBM Monochrome and IBM Enhanced Color displays have distinct resolution, scan/polarity and mode relations. | Miscellaneous Output selects clock/polarity/driver behavior; external monitor is not reset by Core. | Selected monitor model and its precise electrical acceptance are absent: blocked pending primary monitor/adapter personality source. |
| VADP-T4 | EGA pp. 3, 19--20 | The adapter's sequencer has internal display/CPU memory-cycle allocation; it does not establish CPU/ISA wait states, bus grants, physical RAM service or host-present cadence. | No register write authorizes a second scheduler/arbitrator. | Board transaction/visibility phase is L2; receiver is the queued VADP phase contract. |
| VADP-T5 | T449; current project display boundary | Core must retain one VADP owner and one display-snapshot output boundary; VM/profile selects an admitted personality and platform consumes copied frames. | Reset/finalization must stay in the owner lifecycle; profile/platform cannot mutate Core VADP state. | Unallocated selected EGA/CECG clock, monitor, contention and adapter facts are inputs, not a code defect.  Receiver: queued VADP phase contract. |

## Completeness And S16 Transfer

The finite universe is `VADP-R1`--`VADP-R5`, `VADP-F1`--`VADP-F5` and
`VADP-T1`--`VADP-T5`.  It covers all IBM EGA programming groups, planar
memory/mode rules, clocks/retrace/contention signals and selected-adapter/board
limits.  S16 must retain every identifier and add only current-code/test
disposition; every nonconforming or unallocated row transfers once to the
queued VADP phase contract.
