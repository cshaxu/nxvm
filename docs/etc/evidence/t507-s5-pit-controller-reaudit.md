# T507 S5: 8253/8254 PIT Controller Re-Audit

## Research quality and boundary

The primary sources are Intel `231306-001`, *8253/8253-5 Programmable
Interval Timer* (November 1986), and Intel `231164-005`, *8254 Programmable
Interval Timer* (September 1993), in the owner-managed manual archive. Both
are text-native PDFs. Text extraction was used only for navigation; rendered
8253 page 3-56 and 8254 page 8 were visually reviewed. The former supplies the
six mode timing diagrams, gate behavior, binary/BCD count forms and aliases;
the latter supplies count/status latching, the read-back command and NULL COUNT
and OUT status semantics.

The rendered IBM 5160 and IBM 5170 technical-reference material previously
reviewed for T507 S4 corroborates the PC board routes: counter 0 is the timer
interrupt source, counter 1 is the refresh source and counter 2 is the speaker
source. It does not turn every pin edge into an NXVM physical-time value.

Available local implementations were read only as non-normative corroboration:

- 86Box `src/pit.c` keeps per-counter gate, latch and read-back state, and
  separates timer outputs from their machine consumers.
- Bochs `iodev/pit82c54.cc` corroborates independent count/status latches and
  `iodev/pit.cc` corroborates the PC timer-0, refresh and speaker consumer
  roles. Its host timer and host audio paths are not adopted.
- PCjs `machines/pcx86/modules/v2/chipset.js` corroborates timer 1 as refresh,
  timer 2 as speaker and the PC/AT port-61 observation convention. Its host
  presentation policy is not adopted.
- No local MAME or QEMU PIT implementation was available. No external source
  was copied, imported or used as a timing authority.

## List 1 / List 2 reconciliation

| ID | Required 8253/8254 or board relation (List 1) | Sole current owner and direct consumer (List 2) | Disposition |
| --- | --- | --- | --- |
| P1 | Three counters expose data ports `40h`--`42h` and control port `43h`; `8253` does not implement the `8254` read-back command. | `pit.c` owns the ports and personality gate; `core_machine_pit_8253_smoke` proves read-back is ignored for 8253. | Manual-L3. |
| P2 | Counter latch, status latch, LSB/MSB ordering, interleaved accesses, NULL COUNT and current OUT retain independent per-counter state. | `t_pit_data` plus `core_machine_pit_{latch,read,write}()` are the only state and port route; `core_machine_pit_readback_smoke` proves the selected cases. | Manual-L3. |
| P3 | Modes 0--5, aliases 6/7, binary/BCD zero-load conventions and mode-specific load/reload timing are deterministic counter behavior. | `core_machine_pit_tick_mode0()` through `core_machine_pit_tick_mode4_or_5()` and the owner-local waveform smoke are the only implementation. | Manual-L3. |
| P4 | GATE pauses, triggers or restarts only as the selected mode requires; an OUT consumer is not a GATE source. | `core_machine_pit_set_gate()` is the sole gate mutation path. `core_machine_pit_set_output()` binds only a consumer. The waveform smoke covers both separation and mode gates. | Manual-L3. |
| P5 | A counter has a next output transition only when a pending load, active counting or eligible trigger can produce one. | `core_machine_pit_ticks_until_output()` derives it from the same counter state; the Core scheduler observes it without a second timer queue. | Manual-L3 logical counter deadline; physical edge width remains board-time dependent. |
| P6 | Counter 0 reaches IRQ0 through its output transition and reset/finalize withdraws the source. | Construction binds `shared_pit` counter 0 to the one PIC IRQ source; PIT reset/finalize deassert the bound output. `core_machine_pit_irq0_s2_smoke` covers edge, PIC delivery and reset. | Manual-L3 route/order. |
| P7 | Counter 1 is a board refresh source, not a second DMA or PIT state machine. | `machine_board.c` binds the existing counter-1 output to the opaque DMA0 refresh request for non-D4 DMA boards, and publishes the corresponding port-61 status. `core_machine_dma_rtc_authority_smoke` covers the low/high request transition. | Manual-L3 route; its physical refresh cadence is only as qualified by the copied board clock contract. |
| P8 | Counter 2 and port 61 control the logical speaker gate/data/output relation. | One board-local consumer samples the sole shared-PIT counter 2 state; XT PPI, planar-AT and D4 write their board-owned port-B state into that consumer. `core_machine_d4_platform_s4_smoke` covers gate, data and output transitions. | Manual-L3 logical route; host audio remains deliberately outside Core. |
| P9 | A configured D4 auxiliary PIT is a distinct four-port controller and may publish only its documented board receiver. | `auxiliary_pit` is a separate `t_pit` owner at its configured ports; its chosen output reaches only the D4 failsafe receiver. `core_machine_auxiliary_pit_s3_smoke` proves topology, isolation and reset. | Source-backed board route; L2 for unqualified electrical timing. |
| P10 | Reset drops live outputs before clearing local state, restores counter-local defaults, and leaves board reset to the board owner. | `core_machine_pit_reset()` withdraws outputs once, clears only PIT state and resets gates; `core_machine_board_after_pit_reset()` rebinds/programs board routes. | Manual-L3 state/order; board electrical reset is an explicit L2 boundary. |
| P11 | Scheduler composition chooses the earliest PIT output state change through the copied clock domain and advances the one PIT owner before the existing PIC refresh point. | `core_machine_deadline_consider_pit()` and `core_machine_arbitration_advance()` are the only scheduler observation/publication path. | Manual-L3 logical event order where the copied ratio qualifies it; otherwise L2 proportional timing, not a guessed oscillator. |

## Outcome and simplicity review

Every source-backed controller and direct-consumer row already has one state
owner and one production route. No PIT code change is warranted: a new timer
wrapper, speaker timer, refresh state or scheduler path would duplicate a
current owner. The retained flow is:

`guest port/GATE write -> PIT sole state -> bound IRQ0/DMA0/board callback -> existing Core consumer`.

The scheduler observes the PIT state through its one bounded deadline query; it
does not own counter state or advance a private timer. The transient focused
set is the PIT waveform, 8253 personality, read-back, IRQ0, DMA/RTC authority,
D4 platform, auxiliary PIT and XT-PPI keyboard smokes. The complete
repository-only unit suite and documentation governance remain the closure
gates.

## Verification

- Focused PIT/consumer cohort: 9/9 passed (`pit-waveform`, `pit-8253`,
  `pit-readback`, `pit-divider`, `pit-irq0`, `auxiliary-pit`, `d4-platform`,
  `xt-ppi-keyboard` and `dma-rtc-authority` smokes).
- Complete repository-only unit: 312/312 passed with `ctest -L unit -j 8`
  in 15.67 seconds real time.
- Documentation governance: passed for `vm-0-5-0506`.

The implementation/test delta is `+0/-0`: this re-audit confirms the existing
single PIT route and deliberately adds no redundant production or test code.
