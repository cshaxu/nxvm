# T450 S7 PIT 8254 Original Function And Timing Checklist

## Scope And Source Key

This is the complete PIT source universe transferred unchanged to T450 S8.
`I8254` means Intel *8254 Programmable Interval Timer*, order 231164-005
(Sep. 1993); pages are printed data-sheet pages. `AT` means IBM *Personal
Computer AT Technical Reference*, 1502243 (Mar. 1984), system-unit pages. The
inventory preserves counter-clocked functional phases and the selected AT
wiring. Electrical setup/hold/propagation values remain L4 and do not become
Core ticks.

## Register, Programming And Read Universe

| ID | Source | Finite function | Reset/cancellation | Timing or relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| PIT-R1 | I8254 pp. 2--6 | The device has three independent 16-bit counters selected by A1:A0, plus a write-only control-word register. Each counter has count register, counting element, output latch, control logic, CLK/GATE/OUT. | Power-up mode, count and OUT are undefined; each counter must be programmed before use. A control word resets that counter's control logic and immediately sets its initial OUT state. | Bus access selects counter/control; counter operation follows its own CLK. | L3. |
| PIT-R2 | I8254 pp. 5--7, 9 | Control word selects counter, binary/BCD, mode, and LSB-only/MSB-only/LSB-then-MSB count format. A new count follows the programmed format and may be written without changing mode. | Reprogramming cancels/restarts the selected mode as documented; for two-byte count the second byte establishes the new count. | Count transfers from CR to CE at mode-defined clock phase. | L3 logical programming; bus waveform L4. |
| PIT-R3 | I8254 pp. 2, 6--9 | Simple reads follow the programmed byte order; counter-latch command freezes one count until read; read-back can latch selected count and/or status, with status returned before a simultaneous count. | A reprogram or completed read releases the applicable latch; repeated unread latch requests are ignored after the first. | A simple live read is undefined while counting unless CLK is inhibited; a latched read is stable. | L3. |
| PIT-R4 | I8254 pp. 6, 8--9, 17 | Read-back status reports OUT, null-count, RW, mode and BCD. Null count remains set after a new control/count write until CR loads CE. Binary zero is 2^16 and BCD zero is 10^4. | Control-word/write/load transitions define null-count; counters wrap in modes 0/1/4/5 and reload periodically in 2/3. | Load/decrement phases are mode dependent; status/read electrical delay is L4. | L3. |

## Modes, Gates And Counter-Phase Universe

| ID | Source | Finite function | Reset/cancellation | Timing or relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| PIT-F1 | I8254 pp. 10--11, 16--17 | Mode 0 interrupt-on-terminal-count starts OUT low; GATE level enables counting; terminal count makes OUT high until new count/mode. | GATE low pauses counting; a new mode/count performs the documented restart. | Initial count N reaches terminal output after N+1 clock pulses under the stated load convention. | L3 counter phase; electrical edge widths L4. |
| PIT-F2 | I8254 pp. 11--12, 16--17 | Mode 1 hardware retriggerable one-shot starts OUT high, a rising GATE trigger loads/counts, and OUT stays low for N clocks; retrigger extends it. | New count affects a subsequent retrigger; no separate reset input exists. | GATE rising edge is trigger-sensitive and sampled on CLK. | L3. |
| PIT-F3 | I8254 pp. 12--13, 16--17 | Mode 2 rate generator has periodic N-clock cycle and one-clock low OUT pulse; GATE level enables, low GATE forces OUT high, rising trigger reloads. | New count loads at the documented boundary; low GATE cancels the current sequence. | Mode 2 is both level- and edge-sensitive at GATE. | L3. |
| PIT-F4 | I8254 pp. 13--14, 16--17 | Mode 3 square-wave generator repeatedly reloads count and produces high/low phases, including the documented odd-count asymmetry. GATE level enables and rising trigger restarts. | Low GATE disables and forces OUT high; a new count follows the stated boundary. | Mode 3 is both level- and edge-sensitive at GATE. | L3. |
| PIT-F5 | I8254 pp. 14--15, 16--17 | Mode 4 software-triggered strobe starts OUT high, counts when GATE enables, then strobes low for one clock at terminal count. | GATE low pauses count; a new count has the documented first/second-byte effect. | For count N, strobe occurs after N+1 clocks under the load convention. | L3. |
| PIT-F6 | I8254 pp. 15--16, 16--17 | Mode 5 hardware-triggered strobe starts OUT high, a rising GATE trigger loads/counts, then OUT pulses low for one clock at terminal count; it is retriggerable. | New count takes effect on subsequent trigger as documented. | For count N, strobe is after N+1 clocks under the load convention. | L3. |
| PIT-F7 | I8254 pp. 16--17 | GATE is sampled on CLK rising edge; modes 0/2/3/4 use level effects, 1/2/3/5 recognize rising trigger effects. The gate-operation table fixes each mode's disable, initiate and enable behavior. | A control word immediately resets selected control logic/OUT; there is no generic device reset pin. | Count loading/decrement occurs on falling CLK edge; trigger is captured for the next relevant edge. | L3 causal phase. |
| PIT-F8 | I8254 pp. 16--17 | Each counter is a synchronous down-counter; initial count zero has documented maximum meaning, and mode-specific reload/wrap behavior persists without software polling. | Reprogramming changes only the selected counter's state. | Input CLK may range DC to 10 MHz for 8254-2; exact input and output timing table is electrical. | L3 logical count progression; L4 electrical limits. |

## Selected IBM AT Binding And Timing Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Disposition |
| --- | --- | --- | --- | --- |
| PIT-T1 | AT pp. 1-8--1-10, 1-24 | IBM AT maps the 8254-2 as three counter ports plus control at 0040h--0043h. Channel 0 has tied-high GATE and OUT0 to 8259A IRQ0; channel 1 has tied-high GATE and OUT1 as refresh request; channel 2 has GATE from port 0061h bit 0 and OUT2 to speaker path. | Board reset/reprogramming uses normal 8254 programming; output consumers own their own response. | L3 selected topology and port identity. |
| PIT-T2 | AT pp. 1-8--1-9, 1-12 | All three AT counter clocks are 1.190 MHz. Channel 1 is rate-generator programmed to make a 15-microsecond refresh request; its downstream DMA/refresh transaction is not a PIT-owned event. | The channel-1 request route resets/restarts through PIT and DMA owners respectively. | L3 source formula/wiring; publication into Core time/arbitration is L2. |
| PIT-T3 | AT pp. 1-9--1-10, 1-30 | Channel 0 output enters PIC IRQ0; channel 2 output and port 0061h speaker control drive the speaker path. The manual names no unifying delay between OUT edges and those consumers. | PIC EOI/IRQ and host speaker presentation are separate owners. | L3 wiring; selected IRQ visibility/speaker cadence inputs are L2. |
| PIT-T4 | I8254 pp. 18--21 | Bus, clock and gate tables give nanosecond setup/hold/propagation and waveform limits. | These are electrical implementation constraints, not project time quanta. | L4 excluded. |
| PIT-T5 | T433 S6/S7; T449 | Core must use its sole time, arbitration, reset and observation owners. A later PIT candidate must select oscillator conversion, counter/output publication and refresh/IRQ/speaker consumer contracts without a PIT-local scheduler. | No implementation conclusion is made here. | L2 receiver: queued Core PIT 8254 phase contract. |

## Completeness And S8 Transfer

The finite universe is `PIT-R1`--`PIT-R4`, `PIT-F1`--`PIT-F8`, and
`PIT-T1`--`PIT-T5`. It covers all registers, count formats, latch/read-back and
status forms, modes 0--5, clock/gate/output lifecycle, reset/programming
effects, electrical exclusion, and the selected AT oscillator/IRQ/refresh/
speaker bindings. T450 S8 must preserve every identifier and add only
current-code/test disposition; all nonconforming or unallocated rows transfer
once to the queued Core PIT 8254 phase contract.
