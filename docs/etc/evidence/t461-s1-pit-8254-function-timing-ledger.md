# T461 S1: 8254 Function And Logical-Timing Ledger (List 1)

## Evidence Method

Intel 231164-005, *8254 Programmable Interval Timer* (September 1993), is
the normative source. It is a 24-page, text-extractable PDF; rendered figures
15--20 were checked where column extraction loses timing-diagram layout. The
manual supplies the selected logical state and tick rules, but no selected
machine input-clock conversion. Thus every `Manual-L3` row below is a chip
state-transition rule, not a claim about host time or electrical waveform.

External implementations are read-only corroboration, never imported or made
normative: [86Box PIT](https://github.com/86Box/86Box/blob/master/src/pit.c),
[MAME PIT8253/8254](https://github.com/mamedev/mame/blob/master/src/devices/machine/pit8253.cpp),
[PCjs PIT](https://github.com/jeffpar/pcjs/blob/master/machines/pcx86/modules/v2/pit.js),
[Bochs 82C54](https://github.com/bochs-emu/Bochs/blob/master/bochs/iodev/pit82c54.cc),
and [QEMU i8254](https://github.com/qemu/qemu/blob/master/hw/timer/i8254.c).
They independently model counter-local control/count/latch/gate/output state;
their host-time adapters and platform speaker wiring are outside this ledger.

| ID | Selected contract and Intel provenance | Cross-check disposition | Required level |
| --- | --- | --- | --- |
| P1 | Three independent counters; ports select counter 0--2/control. Manual pp. 3--5, Fig. 4, Fig. 14. | All five models retain three counter-local state records. | Manual-L3 |
| P2 | Control word: SC, RW, modes 0--5 (6=2, 7=3), binary/BCD. Manual p. 5, Fig. 7. | 86Box/MAME/Bochs/QEMU normalize aliases and retain BCD. | Manual-L3 |
| P3 | Count write obeys LSB/MSB/LSB-then-MSB; two-byte write becomes effective only after byte two. Manual pp. 5--6. | All observed models retain write-byte state. | Manual-L3 |
| P4 | Binary zero denotes 65536; BCD zero denotes 10000. Manual mode definitions pp. 9--13. | 86Box/MAME/Bochs/QEMU explicitly expand both zero values. | Manual-L3 |
| P5 | Counter-latch captures one count and ignores a later latch until read. Manual pp. 6--7. | MAME/Bochs/86Box retain a latched-count flag. | Manual-L3 |
| P6 | Read-back latches selected count and/or status; first pending status is read before count; repeated pending latch is ignored. Manual pp. 7--8, Figs. 10--13. | MAME/Bochs/86Box model independent status/count latch state. | Manual-L3 |
| P7 | Status byte reports OUT, null-count, RW, mode and BCD; null count clears only when CR loads into CE. Manual pp. 7--8, Figs. 11--12. | MAME/Bochs/86Box retain null-count/status records. | Manual-L3 |
| P8 | Mode 0: next CLK loads without decrement; gate level enables count; terminal OUT rise after N+1 clocks. Manual p. 9, Fig. 15. | All observed models have load/count/terminal phases. | Manual-L3 |
| P9 | Mode 1: GATE rising trigger; next CLK loads and drives OUT low; retriggerable N-clock one-shot. Manual pp. 9--10, Fig. 16. | MAME/Bochs/86Box retain edge-triggered one-shot state. | Manual-L3 |
| P10 | Mode 2: periodic rate generator; one-clock low at count 1; GATE low stops/reloads/forces OUT high and rising edge reloads; count 1 is illegal. Manual pp. 9--11, Fig. 17. | All observed models have a low-pulse/reload phase. | Manual-L3 |
| P11 | Mode 3: periodic square wave; even/odd duty rules, decrement-by-two behavior; GATE low forces high and rising edge reloads. Manual pp. 10--12, Fig. 18. | 86Box/MAME/Bochs explicitly separate high/low phases. | Manual-L3 |
| P12 | Mode 4: software-triggered one-clock low strobe; gate level pauses only count; replacement count loads next CLK. Manual p. 12, Fig. 19. | MAME/Bochs/86Box retain strobe and delayed-load phases. | Manual-L3 |
| P13 | Mode 5: GATE rising trigger, retriggerable one-clock low strobe; count writes wait for trigger. Manual pp. 12--13, Fig. 20. | MAME/Bochs/86Box retain trigger/strobe state. | Manual-L3 |
| P14 | Programming control word immediately resets counter control and establishes mode initial OUT without a CLK. Manual pp. 13--14, Fig. 21. | MAME/Bochs/86Box reset mode-local phase/output on control write. | Manual-L3 |
| P15 | Output callback is a chip pin transition; selected channel 0 becomes IRQ0 only through the existing Core PIT-to-PIC binding and the next PIC refresh. Intel supplies OUT; the deterministic Core visibility order is project L2 scheduler contract. | 86Box/MAME/Bochs also separate PIT OUT from machine IRQ adapters. | Manual-L3 for OUT; Other-L3 corroboration for binding; scheduler visibility remains L2 |
| P16 | PIT reset power-up state is unspecified until a control word. Manual p. 3 functional description; no exact reset waveform/phase is supplied. | MAME explicitly chooses a defined emulator reset state; models differ. | fallback-to-L2 |
| P17 | Selected rational conversion from Core elapsed ticks to PIT CLK requires board input-clock facts and a validated plan; the chip manual specifies acceptable CLK rate, not a PC profile ratio. Manual p. 1 and mode figures. | Every external model uses its own host/platform clock adapter. | fallback-to-L2; receiver: Core L3 integration/time-plan task |
| P18 | Electrical CLK/GATE setup/hold, oscillator phase, pin propagation and speaker/PPI/audio topology. Manual timing diagrams describe pins but no selected board electrical contract is admitted. | External projects embed platform-specific choices. | fallback-to-L2; no L4 work |

The decisive finding is that all selected 8254 internal transition rows P1--P14
are manual-sufficient L3 candidates. P15 is deliberately split: the chip's OUT
transition is L3, while scheduler/IRQ visibility is an existing Core contract
to be verified, not recreated. P16--P18 cannot be raised by emulator agreement.

`M5:T461:S1:PIT-MANUAL-LEDGER:OK`
