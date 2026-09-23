# T490 S2 IBM 5160 8253 Function And Timing List 1

`M5:T490:S2:8253-XT-LIST-1:ACCEPTANCE-CANDIDATE`

This is the complete selected unit: one Intel 8253-5 and its IBM 5160
`40h`--`43h`/IRQ0/refresh/speaker wiring.  Intel and IBM sources in S1 are
normative.  `Manual-L3` names a directly sourced function, formula or board
input; it does not claim the current Core implementation already has it.

| ID | Complete function or timing rule | Primary source | Disposition |
| --- | --- | --- | --- |
| PIT-F1 | The chip has three independent 16-bit counter channels, each with CLK/GATE/OUT, and a write-only control register. | Intel 3-51--3-53 | Manual-L3 state/topology. |
| PIT-F2 | `A1:A0` selects counter 0, 1, 2 or control; control `SC=11` is illegal on 8253. | Intel 3-53--3-54 | Manual-L3 register rule; exclude 8254 Read-Back/status. |
| PIT-F3 | Control selects counter, read/load format, modes 0--5 and binary or four-decade BCD operation. | Intel 3-54 | Manual-L3 register rule. |
| PIT-F4 | A count is written in the programmed LSB/MSB sequence; zero means the maximum binary/BCD count; completed writes restart the selected counter as documented. | Intel 3-54, 3-57 | Manual-L3 data/state transition. |
| PIT-F5 | Direct reads and Counter-Latch reads expose count bytes in the selected order; a latch preserves the sampled count without changing mode. | Intel 3-53, 3-58 | Manual-L3 read/latch relation. |
| PIT-F6 | Mode 0 is interrupt-on-terminal-count: GATE level enables/disables counting and terminal count raises OUT. | Intel 3-54, 3-56 | Manual-L3 functional/timing sequence. |
| PIT-F7 | Mode 1 is a rising-GATE retriggerable one-shot with the documented OUT reset/terminal sequence. | Intel 3-55--3-56 | Manual-L3 functional/timing sequence. |
| PIT-F8 | Mode 2 is a periodic divide-by-N rate generator; low GATE forces OUT high and a new count takes effect at the documented period boundary. | Intel 3-55--3-56 | Manual-L3 functional/timing sequence. |
| PIT-F9 | Mode 3 is a periodic square-wave generator with the documented even/odd count high/low division and GATE behavior. | Intel 3-55--3-56 | Manual-L3 functional/timing sequence. |
| PIT-F10 | Mode 4 is a software-triggered one-clock strobe; GATE enables/disables counting and count reload follows the documented next-clock rule. | Intel 3-55--3-56 | Manual-L3 functional/timing sequence. |
| PIT-F11 | Mode 5 is a rising-GATE retriggerable strobe with the documented terminal one-clock low pulse. | Intel 3-55--3-56 | Manual-L3 functional/timing sequence. |
| PIT-F12 | Gate level/edge semantics, count/reload sequencing and output transitions are those in the six-mode table and timing diagrams. | Intel 3-55--3-56 | Manual-L3 sequence; it is not a host-time conversion. |
| PIT-F13 | Before programming, mode/count/output are undefined; the chip has no Intel software-reset register or reset pin contract in this source. | Intel 3-54 | Manual-L3 absence boundary: no invented chip reset values. |
| PIT-F14 | Bus, CLK/GATE and output nanosecond parameters are electrical component constraints. | Intel 3-59--3-61 | L4/out of scope; no direct Core-tick conversion. |
| PIT-X1 | The selected board maps the 8253-5 at `40h`--`43h`; it is one system-board timer, not an AT second timer or an 8254 superset. | IBM 1-8 | Manual-L3 topology. |
| PIT-X2 | Channel 0 is the periodic general-purpose/time-of-day source and attaches to interrupt level 0. | IBM 1-4 | Manual-L3 board route; PIC delivery retains its sole owner. |
| PIT-X3 | Channel 1 times and requests DMA refresh; refresh remains the one timer-to-DMA board route. | IBM 1-4; IBM 1-14 | Manual-L3 causal route; the DMA unit owns refresh transfers. |
| PIT-X4 | Channel 2 drives speaker tone; PPI port `61h` bit 0 gates it, bit 1 contributes speaker data, and status exposes timer channel-2 output. | IBM 1-10, 1-20 | Manual-L3 board/PPI route; PPI and presentation retain their owners. |
| PIT-T1 | IBM states a 1.19318-MHz oscillator input for channel 2; this is a direct board input, not a Core elapsed-tick rate. | IBM 1-20 | Manual-L3 input fact; the existing copied Core timing-input receiver may consume only a sourced compatible value. |
| PIT-T2 | IBM also states 1.05-us minimum timer resolution and separately gives 1.05-us I/O/DMA transfer timing; those facts do not establish an unqualified elapsed-axis conversion or resolve every counter clock. | IBM 1-4, 1-14 | Manual-L3 facts, L2 integration boundary. |
| PIT-T3 | Board RESET DRV is active high and synchronized to a CLK falling edge, but IBM does not define a 8253 register-reset image. | IBM 1-16; Intel 3-54 | Manual-L3 board signal plus L1 chip-state boundary. |
| PIT-T4 | IBM refresh is one cycle per 72 CLK periods, approximately 15 us, and consumes four clocks; it depends on the channel-1 request/DMA route. | IBM 1-14 | Manual-L3 board formula; exact conversion to the unqualified Core axis remains L2. |

## Completeness and hard exclusions

All chip commands, data/read/latch behavior, six modes, count/Gate/OUT
semantics, startup boundary, electrical boundary, four ports and all three
selected board consumers are represented once.  8254 Read-Back/status latches,
AT auxiliary timers, RTC, host speaker rendering and physical pacing are not
8253 rows.  S3 must map every row to the current owner before selecting any
implementation batch; it may not add a newly discovered row during repair.
