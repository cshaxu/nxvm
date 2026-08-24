# T450 S8 PIT 8254 Current-Code Gap Audit

## Scope And Method

This audit consumes, without changing, all 17 rows in the T450 S7
[PIT source checklist](../research/t450-s7-pit-8254-function-timing-checklist.md).
The reviewed owners are `src/core/machine/pit.c`, `pit.h`, `machine.c`,
`machine_scheduler.c` and `machine_board.c`. Focused tests are
`tests/machine/core_machine_pit_waveform_smoke.c`,
`core_machine_pit_readback_smoke.c`, `core_machine_pit_irq0_s2_smoke.c`,
`tests/core/machine_pit_divider_smoke.c`, plus PC/AT topology and speaker
smokes. Every nonconforming row transfers once to the queued
[Core PIT 8254 phase contract](../../proposals/m5-core-pit-8254-phase-contract.md).
No source or test is modified by this audit.

## Row Dispositions

| S7 ID | Current owner and observed behavior | Current proof | Disposition and unique receiver |
| --- | --- | --- | --- |
| PIT-R1 | `pit.h` owns three independent counter state sets; `pit.c` owns control port, count/latch/status and output state. `core_machine_pit_reset` clears local state and drops published output without resetting a consumer. | PIT waveform, read-back and IRQ0 smokes cover counter/port/reset ownership. | Conforming for the project reset boundary; hardware power-up is not claimed. |
| PIT-R2 | `io_write_0043`, `core_machine_pit_write` and `core_machine_pit_load` implement counter select, binary/BCD, normalized modes, and all RW byte forms. The load occurs immediately on the completed write rather than at the next documented clock phase. | IRQ0 counter-forms, waveform and read-back smokes cover control/byte formats and BCD. | Partial: correct register form, missing selected load phase. Receiver: queued PIT phase contract. |
| PIT-R3 | `core_machine_pit_read`, `core_machine_pit_latch_count` and `core_machine_pit_latch_status` implement byte order, first-latch-wins, read-back selection and status-before-count. | Read-back smoke covers count/status combinations, repeated latches and latch ordering. | Conforming logical read/latch behavior. |
| PIT-R4 | `core_machine_pit_capture_status`, BCD encode/decode and reload/count state implement OUT/null/RW/mode/BCD status and zero-count encodings. The null-count transition is coupled to the immediate load rather than a later CE load edge. | Read-back and waveform smokes cover status, BCD and zero count. | Partial: status form conforms; null-count/load phase awaits the PIT contract. |
| PIT-F1 | `core_machine_pit_tick_mode0` models level GATE, countdown and terminal OUT. It begins decrement on the first generic advance after immediate load, omitting the source's distinct next-CLK load phase. | Waveform and IRQ0 smokes cover pause/resume and terminal output. | Partial: mode function exists; documented initial-count phase is wrong/unselected. Receiver: queued PIT phase contract. |
| PIT-F2 | `core_machine_pit_set_gate` recognizes rising GATE for mode 1 and `core_machine_pit_tick_mode1` implements retriggerable one-shot countdown. The trigger immediately drives OUT low and loads state, although the manual places the load/OUT transition at the relevant clock phase. | Waveform smoke covers rising-GATE start and retrigger. | Partial: trigger function exists; gate-to-clock phase is wrong/unselected. Receiver: queued PIT phase contract. |
| PIT-F3 | `core_machine_pit_tick_mode2` models periodic low pulse, reload and level/rising gate behavior. Reload/trigger takes effect in call order rather than a selected sampled-clock phase. | Waveform and IRQ0 smokes cover one-low-tick waveform, reload and IRQ output. | Partial: waveform function exists; documented gate/load phase is wrong/unselected. Receiver: queued PIT phase contract. |
| PIT-F4 | `core_machine_pit_tick_mode3` implements high/low periods including odd-count split; `core_machine_pit_set_gate` manages disable/restart. Gate/reload changes are immediate rather than sampled at the source phase. | Waveform smoke covers odd count and GATE changes. | Partial: waveform function exists; documented gate/load phase is wrong/unselected. Receiver: queued PIT phase contract. |
| PIT-F5 | `core_machine_pit_tick_mode4_or_5` implements one-low-strobe and level gate for mode 4, but shares the immediate-load simplification. | Waveform smoke covers mode-4 pause and strobe. | Partial: strobe function exists; documented initial-count/load phase is wrong/unselected. Receiver: queued PIT phase contract. |
| PIT-F6 | `core_machine_pit_set_gate` and `core_machine_pit_tick_mode4_or_5` implement rising-GATE mode-5 retrigger/strobe state, but no test isolates its retrigger and source-clock phase. | Waveform smoke exercises a mode-5 strobe only. | Partial and missing test: gate-to-clock phase plus focused mode-5 retrigger proof. Receiver: queued PIT phase contract. |
| PIT-F7 | `core_machine_pit_set_gate` changes mode state synchronously with its API call, while `core_machine_pit_advance` has no rising/falling clock-edge distinction. | Waveform smoke covers level/rising behavior qualitatively. | Missing: one selected GATE sampling and CLK-edge phase model, shared by all modes. Receiver: queued PIT phase contract. |
| PIT-F8 | `core_machine_pit_advance` is deterministic, supports binary/BCD maximum-zero values and periodic/terminal behavior. It advances abstract elapsed ticks, not the admitted 8254 falling-edge counter phase. | Waveform and divider smokes cover zero encodings and rational advance. | Partial: count mechanics conform; selected clock-to-counter conversion/phase is unallocated. Receiver: queued PIT phase contract. |
| PIT-T1 | `core_machine_pit_initialize` retains 0040h--0043h; `machine.c` binds OUT0 to PIC IRQ0 and holds OUT1 as Core state; `machine_board.c` maps channel-2 gate/output through port 0061h speaker ownership. | IRQ0, PC/AT topology and speaker smokes cover ports and routes. | Conforming selected Core topology. |
| PIT-T2 | `machine_scheduler.c` advances a plan-derived `pit_clock`; no accepted 5170 declaration binds the Intel 1.190 MHz source or channel-1's 15-microsecond rate-generator output to the AT refresh request/transaction path. | Divider and Model-339 clock smokes prove generic rational clock plumbing, not the selected AT formula/refresh chain. | Missing input and missing test. Receiver: queued PIT phase contract, with the DMA candidate owning the downstream DMA service. |
| PIT-T3 | OUT0-to-PIC and OUT2-to-speaker are owner-local routes; `machine_scheduler.c` refreshes PIC after PIT advance. The selected command-to-visible IRQ0 phase and speaker cadence are not sourced as a 5170 contract. | IRQ0, speaker and competition smokes prove route/order, not selected elapsed phase. | Partial L2 input, not duplicate device state. Receiver: queued PIT phase contract. |
| PIT-T4 | No production code turns AC tables into ticks. | Source and owner-path inspection. | Correctly L4 excluded. |
| PIT-T5 | T449's scheduler is the sole time/arbitration/reset/observation owner; PIT only advances when that owner supplies ticks. The needed selected PIT phase declaration has not yet been admitted. | Competition and divider smokes prove the current single advance route/reset. | Unallocated L2 input, not a code defect. Receiver: queued PIT phase contract. |

## Completeness, Minimality And Transfer

All `PIT-R1`--`PIT-R4`, `PIT-F1`--`PIT-F8` and `PIT-T1`--`PIT-T5` rows have
one disposition. The existing compact PIT owner already serves all six modes;
the audit rejects six per-mode timing wrappers. The later candidate must add
one source-selected load/GATE/clock-phase mechanism in that owner, consume the
existing rational clock and scheduler, and bind 5170 oscillator/refresh terms
once. It must not add a PIT-local scheduler, a second IRQ publisher, duplicate
speaker state, or a second DMA refresh transaction path.
