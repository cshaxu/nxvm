# T450 S3 PIC 8259A Original Function And Timing Checklist

> S19 direct PDF check: scan with OCR text layer (not born-digital); rendered
> pages govern diagrams/tables. Chip facts are sufficient; selected AT cascade
> and board acknowledge phase remain L2. No external emulator is primary.

## Scope And Source Key

This list is the complete 8259A source universe transferred unchanged to T450
S4. `I8259` means Intel *8259A Programmable Interrupt Controller*, order
231468-003 (Dec. 1988); source pages below are the printed data-sheet pages.
`AT` means IBM *Personal Computer AT Technical Reference*, 1502243 (Mar.
1984), system-unit pages 1-10 and 1-24 plus its I/O map. Electrical waveforms,
propagation values and unselected board-clock timing are deliberately outside
this L3 inventory.

## Registers And Initialization Universe

| ID | Source | Finite function | Reset/cancellation | Timing or relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| PIC-R1 | I8259 pp. 3, 15 | IRR records the eight pending interrupt inputs; ISR records accepted in-service levels; IMR masks levels. | ICW1 clears IMR, ISR and special-mask/read selection; reset semantics are only the documented initialization sequence. | Requests are edge-latched or level-recognized by ICW1; no free-running chip clock. | L3 chip semantics; selected visibility phase is L2. |
| PIC-R2 | I8259 pp. 3, 12--13 | ICW1 starts initialization and selects edge/level, single/cascade, call interval and whether ICW4 follows. | Starts a fresh initialization sequence; its documented clear actions cancel prior logical state. | Initialization requires ICW2, conditionally ICW3 and ICW4; no wall-time delay is specified. | L3. |
| PIC-R3 | I8259 pp. 6--8 | ICW2 supplies vector base; ICW3 identifies master slave inputs or slave identity; ICW4 selects 8086/88 mode, AEOI, buffered mode and special fully nested mode. | Reinitialization replaces the programmed personality. | In 8086 mode, the acknowledged vector is emitted on the second INTA cycle. | L3 for chip state; AT vector/cascade binding is L3 only where AT cites it. |
| PIC-R4 | I8259 pp. 9--11 | OCW1 is the IMR; OCW2 controls EOI, specific/non-specific selection, rotate and set-priority; OCW3 controls poll, IRR/ISR reads and special-mask set/reset. | OCW2 EOI clears the selected ISR service state; OCW3 special-mask reset restores normal mask interpretation. | Writes change logical state without a sourced elapsed delay. | L3. |
| PIC-R5 | I8259 pp. 14--16 | Reads return IMR directly or IRR/ISR after OCW3 read-register selection; poll returns the highest requesting level and updates service state. | A read does not reset a register except the documented poll acknowledgement effect. | RD/CS/A0 are bus transactions; their electrical setup/hold is L4. | L3 logical read/poll semantics; L4 electrical timing excluded. |

## Interrupt, Priority And Cascade Universe

| ID | Source | Finite function | Reset/cancellation | Timing or relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| PIC-F1 | I8259 pp. 3--5 | Edge mode accepts a low-to-high IR transition; level mode recognizes asserted IR. Highest unmasked request eligible under priority raises INT. | ICW1 selects trigger mode; EOI or AEOI releases ISR service as applicable. | IR is asynchronous; no source-backed elapsed interrupt latency is supplied. | L3 function, L2 latency. |
| PIC-F2 | I8259 pp. 4--6 | Fully nested priority is IR0 highest through IR7 lowest; in-service priority blocks lower/equal requests while allowing higher priority requests. | ICW1 restores IR7 lowest; OCW2 set-priority/rotation changes the resolver order. | Priority resolution is causal, not a specified tick duration. | L3. |
| PIC-F3 | I8259 pp. 9--11 | EOI variants are non-specific, specific, rotate-on-non-specific EOI, rotate-on-specific EOI and set-priority; AEOI clears ISR automatically. | Each EOI releases the documented ISR bit; AEOI performs release at the documented acknowledge point. | For 8086/88, AEOI clears after the second INTA pulse; waveform width is L4. | L3 logical phase, L4 waveform excluded. |
| PIC-F4 | I8259 pp. 11--13 | Special mask mode allows a masked in-service level to be treated as not in service for priority; normal mask mode retains normal nesting interpretation. | OCW3 ESMM/SMM enables or resets special mask. | No source selects a board-visible delay for changing mask state. | L3. |
| PIC-F5 | I8259 pp. 12--13 | Poll command makes the next read act as an interrupt acknowledge and returns the highest requesting level. | Poll acknowledgement sets ISR/clears IRR according to the normal acknowledge rule. | Poll is a host read transaction; no elapsed phase is specified. | L3. |
| PIC-F6 | I8259 pp. 4--6, 16 | Cascade supports one master and up to eight slaves through CAS0--CAS2; master ICW3 maps slave inputs and slave ICW3 encodes identity. | ICW1/ICW3 reselect cascade topology. | Master places cascade identity during acknowledge; exact pin waveform is L4. | L3 topology, L4 waveform excluded. |
| PIC-F7 | AT pp. 1-10, 1-24; I8259 pp. 4--6 | IBM AT has two 8259A controllers, master ports 0020h--0021h and secondary ports 00A0h--00A1h; IRQ8--IRQ15 enter through the cascade route. | AT reset/reinitialization uses normal 8259 initialization; no alternate owner is admitted. | AT establishes topology, not a new delay formula. | L3 topology; L2 controller-to-CPU visibility timing. |
| PIC-F8 | I8259 pp. 4--6 | In 8086/88 mode, first INTA resolves/acknowledges and second INTA supplies the vector byte; spurious/no request yields level 7 behavior. | AEOI or later EOI releases ISR as selected. | Exactly two logical INTA cycles; their electrical durations and CPU scheduling point are outside the sources. | L3 cycle count, L2/L4 elapsed delivery. |

## Explicit Timing And Boundary Checklist

| ID | Source | Rule | Disposition |
| --- | --- | --- |
| PIC-T1 | I8259 cover, pp. 3--6 | 8259A is static and has no clock input; timing begins only at an input/host transaction relation. | L3. |
| PIC-T2 | I8259 pp. 4--6 | Edge/level request recognition, priority selection, INT assertion and INTA acknowledgement form a deterministic causal order. | L3 ordering only; latency is L2. |
| PIC-T3 | I8259 pp. 14--24 | Data-sheet read/write, INTA and cascade timing diagrams specify electrical setup, hold and propagation characteristics. | L4/out of scope; never convert to Core ticks. |
| PIC-T4 | AT pp. 1-10, 1-24 | AT supplies two-controller wiring and I/O-port identity. | L3 topology; does not supply a controller clock or IRQ delivery quantum. |
| PIC-T5 | T433 S6/S7 | Core/board requires a selected command-to-visible and CPU-delivery contract. | L2 receiver for later PIC implementation; S3 does not invent it. |

## Completeness And S4 Transfer

The finite universe is `PIC-R1`--`PIC-R5`, `PIC-F1`--`PIC-F8`, and
`PIC-T1`--`PIC-T5`. It covers every register class, ICW/OCW family, read/poll
form, trigger/priority/mask/cascade/acknowledge lifecycle, reset action and
timing category described by the admitted sources. T450 S4 must retain these
identifiers and add only current-code/test disposition; it must not change this
source universe.
