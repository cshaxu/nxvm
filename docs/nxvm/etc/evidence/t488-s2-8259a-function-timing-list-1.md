# T488 S2 IBM 5160 8259A Function And Timing List 1

`M5:T488:S2:8259A-LIST-1:ACCEPTANCE-CANDIDATE`

The complete chip universe is the 18 Intel rows `PIC-R1`--`PIC-R5`,
`PIC-F1`--`PIC-F8`, and `PIC-T1`--`PIC-T5`.  The table below freezes every
row; it is a selected XT ledger, so the cascade row records chip behavior but
does not select an AT slave.  The Intel manual is the sole normative source.

| ID | Source pages | Complete function, reset/cancellation and timing rule | Disposition |
| --- | --- | --- | --- |
| PIC-R1 | Intel 3, 15 | IRR holds requests, ISR accepted service and IMR masks. ICW1 clears the documented logical state; request recognition is edge- or level-selected. | Manual-L3 state/order; visibility latency L2. |
| PIC-R2 | Intel 3, 12--13 | ICW1 starts initialization and selects trigger, single/cascade, call interval and ICW4 presence; it starts a new programmed sequence. | Manual-L3. |
| PIC-R3 | Intel 6--8 | ICW2 supplies vector base; ICW3 identifies cascade relation; ICW4 selects 8086/88, AEOI, buffered and special fully nested modes. Reinitialization replaces the personality. | Manual-L3 chip semantics. |
| PIC-R4 | Intel 9--11 | OCW1 programs IMR; OCW2 selects EOI/rotation/priority; OCW3 selects poll, IRR/ISR read and special mask. EOI releases documented ISR service. | Manual-L3. |
| PIC-R5 | Intel 14--16 | Reads return IMR or OCW3-selected IRR/ISR; poll read performs the documented acknowledge effect. | Manual-L3 logical read/poll; electrical bus timing L4. |
| PIC-F1 | Intel 3--5 | Edge mode latches a rising IR; level mode recognizes assertion. Highest eligible unmasked request raises INT. | Manual-L3 function; asynchronous latency L2. |
| PIC-F2 | Intel 4--6 | Fully nested priority starts IR0-high/IR7-low; an in-service level blocks lower/equal priority; OCW2 rotation/set-priority changes the resolver. | Manual-L3. |
| PIC-F3 | Intel 9--11 | Specific/non-specific EOI and rotate variants release selected service; AEOI releases at the documented acknowledge point. | Manual-L3 logical phase; waveform L4. |
| PIC-F4 | Intel 11--13 | OCW3 special-mask set/reset controls whether a masked in-service level affects nesting. | Manual-L3. |
| PIC-F5 | Intel 12--13 | Poll makes the next read an acknowledge and returns the highest eligible level. | Manual-L3; elapsed read phase L2. |
| PIC-F6 | Intel 4--6, 16 | Cascade uses ICW3 and CAS0--CAS2; this XT selection does not configure a slave. | Manual-L3 topology; pin waveform L4. |
| PIC-F7 | Intel 4--6 | The selected machine is single-PIC; no AT secondary port, IRQ8--15 route or cascade binding belongs to this unit. | Manual-L3 exclusion/topology. |
| PIC-F8 | Intel 4--6 | In 8086/88 mode first INTA resolves/acknowledges and second supplies the vector; no-request behavior is IRQ7. | Manual-L3 two-cycle order; elapsed delivery L2 and electrical widths L4. |
| PIC-T1 | Intel cover, 3--6 | The 8259A is static and has no clock input. | Manual-L3 absence of a controller clock. |
| PIC-T2 | Intel 4--6 | Request recognition, priority, INT assertion and INTA form deterministic causal order. | Manual-L3 order; latency L2. |
| PIC-T3 | Intel 14--24 | Read/write, INTA and cascade diagrams give electrical setup, hold and propagation characteristics. | L4/out of scope; never Core ticks. |
| PIC-T4 | IBM 5160 1-6, 1-8 | IBM selects one 8259A at `20h`--`21h`, with eight IRQ levels. | Manual-L3 selected topology. |
| PIC-T5 | Intel 3--10; IBM 5160 1-14 | CPU-visible delivery consumes the selected logical acknowledge; no source fixes an elapsed Core placement. NMI remains separate. | Manual-L3 causal route; elapsed placement L2. |

The only XT-specific replacement is topology:

| Key | IBM 5160 source | Complete selected rule | Level/boundary |
| --- | --- | --- | --- |
| PIC-XT1 | 1-6, 1-8 | One 8259A owns eight IRQ levels at `20h`--`21h`; no slave window or cascade is selected. | Manual-L3 topology. |
| PIC-XT2 | 1-6, 1-14 | Timer/keyboard/channel IRQ producers reach the single PIC; NMI/check remains outside PIC. | Manual-L3 causal routing; producer semantics stay separate. |
| PIC-XT3 | Intel 3--10 | 8086/88 acknowledgement is logically two INTA cycles; no IBM/Intel elapsed-tick placement exists. | Manual-L3 sequence; L2 visibility latency. |

No AT cascade row, controller clock, or physical INTA duration is inherited.
S3 must map all eighteen Intel rows plus three XT rows to `pic.c` and its sole
Core consumers before any repair.
