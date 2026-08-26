# T488 S1 IBM 5160 8259A Original-Source Ledger

`M5:T488:S1:8259A-ORIGINAL-SOURCE:ACCEPTANCE-CANDIDATE`

Primary sources are Intel, *8259A Programmable Interrupt Controller*, order
231468-003 (December 1988), and IBM, *Personal Computer XT Hardware Reference
Library*, 1502237 (April 1983). Both remain only in the owner-managed external
manual archive. No source, firmware or media is imported into NXVM.

The Intel 24-page PDF is a visual scan; no OCR text is treated as authority.
Rendered pages 1, 3, 5, 8 and 10 were visually inspected. IBM 5160 source
pages 1-6 through 1-8 were previously rendered and visually inspected.

| Topic | Primary page(s) | Verified fact | S2 implication |
| --- | --- | --- | --- |
| Core state | Intel 1, 5 | 8259A has IRR, priority resolver, ISR and IMR; IR0--IR7 request lines produce INT and are serviced through INTA. | Complete register/priority/request lifecycle. |
| Electrical/time boundary | Intel 1 | The part is static and requires no clocks. | No controller-clock formula; physical signal waveform is not implied. |
| Interrupt/acknowledge | Intel 3, 8 | PIC selects highest eligible request, signals INT and supplies vector sequence during INTA. | Complete logical acknowledgement and vector rows; pin waveform spacing is separate. |
| Initialization/cascade | Intel 10 | ICW1--ICW4 sequence reaches ready state; ICW3 applies only for cascaded systems. | XT single-PIC setup must not inherit AT cascade behavior. |
| IBM XT topology | IBM 5160 1-6, 1-8 | Board data flow contains one 8259A; I/O map assigns `20h`--`21h` to it and identifies eight IRQ levels. | Freeze single controller and master port mapping. |
| Board routes | IBM 5160 1-6; 1-14 | Timer/keyboard/channel paths enter board interrupt/NMI logic; NMI is distinct from PIC IRQ delivery. | Keep PIC IRQ and NMI routes distinct; producer details belong to their unit T. |

## Explicit Limits

Intel documents command/state and logical INTA sequencing, but provides no
clocked duration that can map INT/INTA edges into Core elapsed ticks. IBM gives
XT topology, not an electrical INTA waveform. 86Box, MAME, PCjs, Bochs and
QEMU may only corroborate a residual observable semantic after List 1; none is
primary authority or admitted by S1.
