# T366 S2: IBM 5170 Model 339 Profile Lock

## Decision

T366 uses an exact late field configuration: **IBM PC/AT 5170 Model 339**,
Type 3 system board, 8 MHz 80286, 512 KB planar RAM, Rev.3 ROM slots dated
1985-11-15 (U27 `61X9266`, U47 `61X9265`), 101-key enhanced keyboard, IBM
Color/Graphics Adapter part 1501981, and TEAC FD-235HF-A529 3.5-inch 1.44 MB
diskette drive on the IBM diskette-controller/DMA-channel-2 route.

The TEAC drive is a documented compatible field upgrade, not a factory Model
339 component or IBM AT option. The Model 339 MFM/ST-506 fixed-disk subsystem,
serial/parallel adapter, 80287, memory expansion and every other ISA option
are deliberately unselected. In particular, the current ATA/HDC model is not
the historical IBM MFM route and is not part of this baseline. A separately
bounded TODO owns any future MFM/ST-506 controller and drive work.

This corrects the earlier 360 KB and 1.2 MB drive assumption. The selected
drive is 1.44 MB; its identity is recorded without assigning FDC, DMA, memory,
or bus time. No vendor ROM, DOS, diagnostics, media, image, hash or local path
is committed or recorded.

## Source Basis

| Fact | Authority and result | Disposition |
| --- | --- | --- |
| Model 339, Type 3, 8 MHz, 512 KB, Rev.3 and keyboard | [IBM Models 319/339 material](https://sharktastica.co.uk/resources/docs/IBM_186-052_5170-319-339_86.pdf) and [IBM PC/AT Technical Reference, March 1986](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf). | Select topology only; waits, refresh, HOLD/HLDA and phases remain unallocated. |
| ROM slot | Rev.3 identifies U27 `61X9266` and U47 `61X9265`. Source policy prohibits ROM material. | A later local-only manifest may validate owner-supplied media. |
| 1.44 MB route | [Type 3 drive-support record](https://minuszerodegrees.net/5170/floppy_diskette/5170_floppy_drive_support.htm) documents the IBM controller/Rev.3 CMOS requirement; [TEAC FD-235HF-A529 specification](https://ftpmirror.infania.net/sites/bitsavers/pdf/teac/floppy/FD235/FD-235HF-A529_A540_A591_Specification_Rev_B.pdf) identifies the named dual-density/high-density drive. | Select drive and channel-2 route; mechanics, DRQ/grant, service timing and DOS-media behavior remain later work. |
| Display | IBM CGA 1501981 matches the bounded current CGA surface. | Physical bus/video contention and display breadth remain later L3 work. |
| MFM versus ATA | Historical Model 339 fixed storage is MFM/ST-506 through IBM's combined adapter; ATA is distinct. | No fixed disk is selected; ATA must not be mapped to this profile. |
| NMI | IBM identifies parity or I/O-channel check as NMI sources. No expansion or adapter-parity source is selected. | The next S may admit only system-board memory parity and must define its complete lifecycle. |

86Box `ibmat` and MAME `ibm5170a` may only cross-check a future precisely
stated observation. They are not authorities for this machine identity, a
timing value, ROM, drive or implementation.

## Project-Owned Probe Corpus

The later corpus contains only repository-authored source and fixtures for:

1. reset/configuration identity and 512 KB planar RAM;
2. CPU/RAM/ROM-slot/CGA transaction traces;
3. a TEAC 1.44 MB FDC command, DRQ, IRQ6 and DMA-channel-2 sequence;
4. PIT/PIC/RTC/KBC/DMA equal-tick/reset observations; and
5. a controlled system-board parity lifecycle observation after implementation.

Each probe records its source, generated binary hash, profile, observation
window, expected trace and cleanup. Emulator outputs and vendor assets never
become committed test inputs or implementation authority.

## Transfers And Exit

The next T366 S may admit only the system-board-memory-parity NMI lifecycle.
It must not introduce I/O-channel check, CMOS/RTC/PIC as a producer, ATA as
Model 339 storage, bus waits, or unselected serial/parallel hardware.
Availability, FDC/DMA service timing, CGA contention, executable probes and
MFM/ST-506 storage remain later bounded work.

S2 makes no runnable, artifact, model-L3, physical-timing or cycle-exact
claim. Documentation governance and `git diff --check` are required for
acceptance.
