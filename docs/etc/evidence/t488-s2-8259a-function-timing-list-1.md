# T488 S2 IBM 5160 8259A Function And Timing List 1

`M5:T488:S2:8259A-LIST-1:ACCEPTANCE-CANDIDATE`

The complete chip universe is the 18 Intel rows `PIC-R1`--`PIC-R5`,
`PIC-F1`--`PIC-F8`, and `PIC-T1`--`PIC-T5` reproduced from the visually
checked Intel source ledger: IRR/ISR/IMR; ICW1--4; OCW1--3; IMR/IRR/ISR/poll
reads; edge/level request; nested/rotating/special-mask priority; EOI/AEOI;
cascade; 8086/88 two-INTA vectoring/spurious IRQ7; static/no-clock causal
order; and electrical timing diagrams. Each row remains Manual-L3 for stated
logic/order, while elapsed delivery latency is L2 and electrical waveform is
outside L3.

The only XT-specific replacement is topology:

| Key | IBM 5160 source | Complete selected rule | Level/boundary |
| --- | --- | --- | --- |
| PIC-XT1 | 1-6, 1-8 | One 8259A owns eight IRQ levels at `20h`--`21h`; no slave window or cascade is selected. | Manual-L3 topology. |
| PIC-XT2 | 1-6, 1-14 | Timer/keyboard/channel IRQ producers reach the single PIC; NMI/check remains outside PIC. | Manual-L3 causal routing; producer semantics stay separate. |
| PIC-XT3 | Intel 3--10 | 8086/88 acknowledgement is logically two INTA cycles; no IBM/Intel elapsed-tick placement exists. | Manual-L3 sequence; L2 visibility latency. |

No AT cascade row, controller clock, or physical INTA duration is inherited.
S3 must map all eighteen Intel rows plus three XT rows to `pic.c` and its sole
Core consumers before any repair.
