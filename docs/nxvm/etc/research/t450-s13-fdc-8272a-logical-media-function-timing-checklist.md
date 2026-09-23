# T450 S13 Intel 8272A FDC And Logical-Media Original Function And Timing Checklist

## Authority And Method

The selected Core device is `Intel 8272A` (`src/core/machine/fdc.h`). Intel,
*Peripheral Design Handbook* (1981), 8272 section, printed pp. 1-146--1-152,
is the original authority: command table, command/result sequences, MSR/data
phases, signals, timing fields and reset were read from rendered pages, not
inferred from OCR. IBM *Personal Computer AT Technical Reference* (1502243,
Mar. 1984) is the board authority for AT ports, IRQ6, DMA2 and CMOS drive
categories. Neither manual selects an image grammar, drive mechanics, BIOS
policy, or Core-clock conversion.

NEC uPD765 is compatible-family comparison only, never the current selection.
Cross-validation on 2026-08-25 read 86Box `4fef696` (`src/floppy/fdc.c`),
Bochs 2.6 (`iodev/floppy.cc`), PCjs (`machines/pcx86/modules/v2/fdc.js`),
current MAME `src/devices/machine/upd765.cpp`, and current QEMU
`hw/block/fdc.c`. MAME has a separate `i8272a_device`; 86Box and Bochs dispatch
later/vendor commands; PCjs deliberately implements a common subset; QEMU is
a later PC controller. They corroborate only labelled models and never extend
the selected Intel set. No reference code is imported.

## Cross-Validation Matrix

| ID | Comparison result | Final source tier |
| --- | --- | --- |
| FDC-R1 | MAME separates 8272A MSR/data mapping; all retain command/result state. Board reset wrappers differ. | Manual L3 chip phase; wrapper is board L3 only when selected, otherwise fallback to L2. |
| FDC-R2 | 86Box/MAME gate extensions by device; Bochs accepts Version; PCjs implements ten common commands. | Manual L3: exactly Intel's fifteen forms and invalid `10h -> ST0 80h`. |
| FDC-R3 | All use command/result bytes, DRQ/IRQ and TC, but differ on media error and CHRN mapping. | Manual L3 phase/bytes; mapping is Other/board L3 only when selected, otherwise fallback to L2. |
| FDC-R4 | All retain Specify/Seek/Recalibrate/Sense; PCjs ignores physical delay and MAME supplies its own timers. | Manual L3 fields/order; time conversion fallback to L2. |
| FDC-R5 | MAME names distinct `i8272a_device`; 86Box/Bochs expose later Version behavior. | Manual L3 selected identity/invalid opcode; extensions fallback to L2. |
| FDC-F1 | MAME has per-drive lines; other models choose differing drive abstractions. | Manual L3 pin relation; mechanics fallback to L2. |
| FDC-F2 | All retain DRQ, DMA/TC and completion IRQ; bus scheduling differs. | Manual L3 relation; bus phase fallback to L2. |
| FDC-F3 | Each emulator owns a different scheduler and clock conversion. | Manual L3 chip values; conversion/deadline fallback to L2. |
| FDC-F4 | MAME retains per-drive state; PCjs logical cylinders; timings/interleavings differ. | Manual L3 parallel-seek rule; mechanics fallback to L2. |
| FDC-F5 | Every comparison supplies media format behaviour beyond the chip manual. | Manual L3 capability; grammar fallback to L2 unless selected as Other/board L3. |
| FDC-T1 | PC models bind conventional ports/IRQ/DMA, but adapters are not chip facts. | Manual L3 IBM AT topology. |
| FDC-T2 | Drive type is platform configuration in every comparison. | Manual L3 IBM categories; mapping fallback to L2. |
| FDC-T3 | BIOS/motor policy is outside controller models. | fallback to L2. |
| FDC-T4 | Image parsing/format detection materially differ. | fallback to L2 until a board/media grammar is Other/board L3. |
| FDC-T5 | Host scheduling stays outside command decode. | fallback to L2 until board timing inputs are admitted. |

## Intel 8272A Function And Timing Universe

| ID | Original-manual rule | Reset, sequence or timing relation | Disposition |
| --- | --- | --- | --- |
| FDC-R1 | A0 selects MSR or data register. MSR exposes RQM/DIO/NDMA/controller state and seek-busy; data carries command, execution and result bytes. | RQM gates each byte; reset returns idle, clears transfer signalling and retains Specify SRT/HUT/HLT. | Manual L3. |
| FDC-R2 | Fifteen forms: Read/Write Data, Read Track, Specify, Sense Drive Status, Read ID, Recalibrate, Sense Interrupt Status, Format Track, Read/Write Deleted Data, Scan Equal/Low/High, Seek. Other opcodes, including `10h`, return ST0 `80h` without IRQ. | A command completes its result phase before another starts. | Manual L3. |
| FDC-R3 | Read/Write/Deleted/Scan/Read Track use MT/MFM/SK, drive/head, C/H/R/N, EOT, GPL and DTL; Read ID returns ST0--ST2 plus CHRN; Format consumes N/SC/GPL/fill and ID fields. | TC ends transfer. DMA uses DRQ/DACK; non-DMA signals each byte through INT. | Manual L3 shape; media grammar/error mapping fallback to L2. |
| FDC-R4 | Specify encodes SRT/HUT/HLT/ND. Seek/Recalibrate change selected-drive cylinder. Sense Interrupt reports completion/PCN; Sense Drive Status reports inputs. | SRT 1--16 ms in 1-ms steps; HLT 2--254 ms in 2-ms steps; HUT 16--240 ms in 16-ms steps; reset keeps Specify timing fields. | Manual L3 fields/order/units; Core tick conversion fallback to L2. |
| FDC-R5 | Intel 8272A is selected; no Version or Read Diagnostic command exists. | `10h` follows invalid-command path. Future models must be immutable separate contracts, never runtime branches. | Manual L3 selected rule; non-8272 identity behavior fallback to L2. |
| FDC-F1 | Four drives exchange READY, INDEX, TRK0, WPRT/2SIDE and read-data/window with select, side, direction/step, head-load, write and encoding outputs. | Reset releases/cancels drive signalling. | Manual L3 relation; mechanics fallback to L2. |
| FDC-F2 | DRQ requests DMA byte, DACK accepts it, TC terminates eligible transfer, INT distinguishes non-DMA byte service from DMA completion. | Reset removes DRQ/INT. | Manual L3 relation; ISA service phase fallback to L2. |
| FDC-F3 | 4/8-MHz clocking and FM/MFM paths; AC tables define electrical/cycle limits. | Values constrain chip interface, not repository event clock. | Manual L3 values; deadline conversion fallback to L2. |
| FDC-F4 | Parallel Seek permits one drive seek while work proceeds for another, subject to busy/status rules. | Completion is consumed through Sense Interrupt Status. | Manual L3 logical state; mechanics fallback to L2. |
| FDC-F5 | FM/MFM, single/double side, multi-sector/multi-track and scan are controller capabilities. | Manual does not prescribe image file, gaps, flux, rotation or complete grammar. | Manual L3 capability; grammar fallback to L2. |

## IBM AT And Ownership Boundary Universe

| ID | Board/manual rule | Disposition |
| --- | --- | --- |
| FDC-T1 | IBM AT assigns 03F0h--03F7h, IRQ6 and DMA channel 2. | Manual L3 topology; PIC/DMA service stays at those owners. |
| FDC-T2 | CMOS byte 10 names no drive, double-sided 48-TPI and high-capacity 96-TPI categories. | Manual L3 categories; mechanics/geometry mapping fallback to L2. |
| FDC-T3 | BIOS retry, motor, timeout and diskette-parameter policy is firmware policy, not 8272A law. | fallback to L2; outside Core FDC. |
| FDC-T4 | Neither manual fixes repository image/flux/CHRN grammar. | fallback to L2 until one board/media grammar becomes Other/board L3; one Core media path remains receiver. |
| FDC-T5 | Core owns device state; board wiring; VM frozen composition and host-media lifecycle. | fallback to L2 until board supplies validated readiness/motor/rotation inputs; no second scheduler/owner. |

## Completeness And Transfer

The finite universe is exactly `FDC-R1`--`FDC-R5`, `FDC-F1`--`FDC-F5` and
`FDC-T1`--`FDC-T5`. S14 must retain every identifier, distinguish implemented
Manual L3 from a code gap, and give every gap one existing owner/receiver.
