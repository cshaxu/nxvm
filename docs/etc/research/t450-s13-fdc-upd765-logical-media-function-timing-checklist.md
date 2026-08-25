# T450 S13 uPD765 FDC And Logical-Media Original Function And Timing Checklist

## Scope, Source Form And Sufficiency

**S19 verification:** direct rendered-page inspection confirms scan with OCR
text layer; OCR errors remain visible, so the rendered source governs.

`FDC` denotes NEC *uPD765A/uPD765B Single/Double Density Floppy-Disk
Controller*; `AT` denotes IBM *Personal Computer AT Technical Reference*,
1502243 (Mar. 1984). The admitted FDC PDF is a scanned/OCR document: extracted
text is usable for headings and tables but contains visible recognition errors;
S19 must directly verify this form and every cited page against the rendered
PDF. The AT source supplies selected IRQ6, DMA2, port-range and CMOS drive-type
facts, but not a complete logical image/sector-layout or motor/rotation contract.

The manual is sufficient for the chip command/state/timing rows below except
where a row explicitly names a missing exact selected-board or media fact.
Those gaps are L2 or blocked, not inferred implementation requirements. No
mature-emulator reference is yet selected; S19 may name one only as a labelled
reference-derived investigation for an exact identified gap.

## T465 S1 Cross-Validation

Rendered NEC pages remain normative. On 2026-08-25, read-only comparison used
86Box `4fef696` (`src/floppy/fdc.c`), local Bochs 2.6
(`iodev/floppy.cc`), local PCjs (`machines/pcx86/modules/v2/fdc.js`), current
MAME `src/devices/machine/upd765.cpp`, and current QEMU `hw/block/fdc.c`.
MAME models the uPD765 family directly; QEMU models a later 82078 and its
format autodetection is not a uPD765 or IBM AT authority. No reference source
is imported.

| Row | Cross-check result | Final tier |
| --- | --- | --- |
| FDC-R1 | All five models retain MSR/data phase and reset state. | Manual L3. |
| FDC-R2 | All implement command/result phase families; Version differs by selected variant. | Manual L3 for named forms; variant is board L3 or fallback to L2. |
| FDC-R3 | All model result/status and transfer phases, but differ on image error/CHRN mapping. | Manual L3 chip phase; media mapping is board L3 or fallback to L2. |
| FDC-R4 | All retain Specify/Seek/Sense; none supplies universal physical drive timing. | Manual L3 register relation; drive phase is board L3 or fallback to L2. |
| FDC-R5 | MAME/86Box expose variant behavior; QEMU is a later controller. | Manual L3 difference; selected revision is board L3 or fallback to L2. |
| FDC-F1 | References model drive signals at differing abstraction levels. | Manual L3 pin relation; mechanics fallback to L2. |
| FDC-F2 | References agree on DMA/IRQ logical sequencing. | Manual L3; bus service phase fallback to L2. |
| FDC-F3 | Reference delays are scheduler-specific. | Manual L3 only for stated formulas; AC conversion L4 or board L3. |
| FDC-F4 | References retain per-drive seeks but choose incompatible mechanical timing. | Manual L3 logical state; mechanics fallback to L2. |
| FDC-F5 | Every reference adds a media grammar/format table beyond NEC. | Manual L3 capability; selected grammar is Other/board L3 or fallback to L2. |
| FDC-T1 | AT ports/IRQ6/DMA2 are represented consistently. | Manual L3 topology. |
| FDC-T2 | Drive-type mapping is platform-specific in every reference. | fallback to L2 until selected drive source. |
| FDC-T3 | BIOS/motor policy is deliberately outside controller models. | fallback to L2. |
| FDC-T4 | Format autodetection/CHS mapping differs materially across references. | fallback to L2 absent an admitted grammar. |
| FDC-T5 | References keep controller scheduling separate from board time. | board L3 when selected; otherwise fallback to L2. |

## Registers, Commands And Result Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| FDC-R1 | FDC pp. 1--5, 10--11 | A0 selects main-status versus data register. Main status reports RQM, DIO, NDMA, controller/execution and per-drive seek busy; data register carries every command, execution byte and result byte. | RESET enters idle, clears drive outputs/INT/DRQ and leaves Specify SRT/HUT/HLT unchanged. | RQM gates each command, execution and result transfer. | Primary sufficient for chip register state: L3. |
| FDC-R2 | FDC pp. 1, 10--15, Table 4 | Command phase accepts every named command form: Read Data, Read Deleted Data, Write Data, Write Deleted Data, Read Diagnostic, Read ID, Format Write, Scan Equal/Low-or-Equal/High-or-Equal, Specify, Sense Drive Status, Recalibrate, Sense Interrupt Status, Seek and Version. Table 4 fixes command/result byte forms. The manual's summary/table command count is internally ambiguous around Version and must not be silently normalized. | Command acceptance is constrained by busy/phase; invalid/unsupported forms receive their documented status outcome. | Commands progress Command -> Execution where applicable -> Result. | Primary sufficient for named forms/state; command-count wording is an explicit manual ambiguity for S19. |
| FDC-R3 | FDC pp. 10--15, Tables 3--4 | Read/write/deleted/diagnostic/scan commands use MT/MF/SK, drive/head, C/H/R/N, EOT, GPL and DTL; return ST0--ST2 plus resulting C/H/R/N. Read ID returns its seven result bytes; Format Write consumes N/SC/GPL/fill and ID fields. | TC ends transfer; result status records abnormal termination, no-data, CRC, overrun, write-protect and related outcomes. | Execution transfers sector bytes through DRQ/DACK or non-DMA byte interrupts. | Primary sufficient for chip function; physical drive/media encoding is separate L2. |
| FDC-R4 | FDC pp. 10--15 | Specify supplies SRT, HUT, HLT and ND; Seek/Recalibrate select drive/head and update present cylinder; Sense Interrupt Status returns completion state/PCN; Sense Drive Status reports fault/write-protect/ready/track-zero/two-side/head/unit state. | RESET leaves Specify parameters; Sense Interrupt Status clears reset/seek completion interrupt state as specified. | SRT is 1--16 ms in 1-ms increments; HLT 2--254 ms in 2-ms increments; HUT 16--240 ms in 16-ms increments. | Primary sufficient for chip parameters/formula: L3. |
| FDC-R5 | FDC pp. 1, 5--7, 10--15 | Version distinguishes uPD765A/B; scan commands compare host and disk data; multi-sector/multi-track and FM/MFM selections are command fields. | A/B differ in overrun and DRQ-reset detail. | uPD765B clears DRQ before result phase independent of DACK; uPD765A requires DACK after overrun. | Primary sufficient for variant difference: L3; selected revision is an L2 input. |

## Data, Drive, DMA/IRQ And Timing Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- | --- |
| FDC-F1 | FDC pp. 1--4 | The controller drives up to four units and consumes READY, INDEX, TRK0, WPRT/2SIDE and read-data/window inputs; it emits select, side, direction/step, head-load, write-enable/data, MFM and precompensation signals. | RESET lowers FDD outputs except noted preshift/WDATA conditions. | INDEX begins a track; head load and fault-reset precede applicable read/write work. | Primary sufficient for chip-pin relation: L3; selected drive mechanics remain L2. |
| FDC-F2 | FDC pp. 1--4, 10--15 | DRQ requests DMA transfer; DACK makes a DMA cycle active; TC terminates Read/Write/Scan. In non-DMA mode INT signals every byte; in DMA mode INT signals command termination. | RESET drops DRQ/INT; documented A/B overrun difference controls DRQ release. | Source provides DACK/TC/DRQ/INT relation, not the AT bus scheduler phase. | L3 logical chip relation; board transaction phase L2. |
| FDC-F3 | FDC pp. 3--4, 8--9 | CLK is 4 or 8 MHz; read/write path includes PLL window/RDATA/WCLK and FM/MFM state. | RESET, ready/fault/write-protect and terminal count abort the relevant operation. | AC tables provide nanosecond and clock-cycle electrical limits, including DRQ/DACK/INT relations and step/fault timing. | Chip formula/value is L3 where Table 4 states it; AC/electrical conversion is L4 unless board-selected. |
| FDC-F4 | FDC pp. 10--15 | Parallel Seek permits one drive seek while command work proceeds for another subject to status/busy rules. | Seek completion is reported through Sense Interrupt Status. | Exact physical head stepping follows SRT plus selected drive mechanics. | Primary sufficient for logical parallel-seek state; selected drive phase is L2. |
| FDC-F5 | FDC pp. 1, 10--15 | FM (IBM 3740) and MFM (IBM System 34), single/double sided, multi-sector/multi-track and data-scan are chip capabilities. | Command fields and drive status constrain operation. | Data rate, rotational index phase, gap layout and media geometry are not fully selected by this chip manual. | L3 chip capability; logical-media format/drive timing is blocked pending source. |

## Selected IBM AT And Media Boundary Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Sufficiency and disposition |
| --- | --- | --- | --- | --- |
| FDC-T1 | AT pp. 1-10, 1-24--1-28 | AT assigns diskette controller IRQ6, DMA channel 2 and I/O range 03F0h--03F7h. | PIC/DMA acknowledgement, arbitration and port-provider lifecycle remain their owners. | Primary sufficient topology: L3; visibility/service phase L2. |
| FDC-T2 | AT pp. 1-45--1-49 | CMOS byte 10 describes installed drive A/B as no drive, double-sided 48-TPI, or high-capacity 96-TPI. | Firmware configuration/checksum/recovery remains board/firmware policy. | Primary sufficient for these declared drive categories, not exact track/sector/bit-rate mechanics: L2. |
| FDC-T3 | AT pp. 5-10, 5-12 and diskette BIOS material | BIOS keeps diskette parameter pointers, retry and motor-start policy; it warns that retry can be required for motor startup. | Guest BIOS lifecycle is not an FDC-core state owner. | Exact motor spin-up/off, rotation, selected BIOS parameter block and timeout formulas are not an FDC manual fact: L2. |
| FDC-T4 | FDC pp. 1--4; AT pp. 1-45--1-49 | A logical image needs a selected mapping from tracks/heads/sectors/data rate/encoding and drive type to the FDC pins/commands. Neither admitted source fixes a repository image grammar or all selected geometry terms. | No image/parser may invent controller state or bypass DRQ/IRQ. | Blocked: selected logical-media format/drive source is absent. Receiver: queued FDC/media phase contract. |
| FDC-T5 | T433 S6/S7; T449 | Core's existing scheduler, transaction, DMA, PIC and observation owners must receive selected FDC readiness/motor/rotation/DRQ/IRQ terms; FDC cannot create another scheduler or media authority. | Reset/finalization cancel FDC local work through existing owners. | Unallocated L2 input, not a code defect. Receiver: queued FDC/media phase contract. |

## Completeness And S14 Transfer

The finite universe is `FDC-R1`--`FDC-R5`, `FDC-F1`--`FDC-F5` and
`FDC-T1`--`FDC-T5`. It covers all uPD765 command families, data/result phases,
drive pins, DMA/IRQ/reset/clock relations, selected AT IRQ6/DMA2/port and CMOS
facts, plus the explicitly blocked logical-media/drive inputs. S14 must retain
every identifier and add only current-code/test disposition; each nonconforming
or unallocated row transfers once to the queued FDC/media phase contract.
