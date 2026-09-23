# T490 S3 IBM 5160 8253 Current-Code Gap List 2

`M5:T490:S3:8253-XT-LIST-2:ACCEPTANCE-CANDIDATE`

This maps every frozen T490 S2 row to one current production route. It is a
code audit only: it neither changes PIT behavior nor creates a second XT path.
The sole state owner is `t_pit` in `src/core/machine/pit.c`; `core_machine`
owns only its construction, output bindings and scheduler advance.

## Owner route and audit scope

`guest port 40h--43h -> executor_port -> t_pit -> bound PIC/DMA/board consumer`
is the only production route. `core_machine_config` is copied at construction;
the XT profile is its only selected configuration producer. The audit inspected
every PIT construction site, all control-word/read-back logic, PIT output
bindings, the scheduler/deadline route, the XT declaration and the focused PIT
and XT profile tests.

Code-size baseline before the later implementation batch: `pit.c` is 550
lines and `pit.h` is 111 lines at commit `16f35564`. The proposed repair adds
one immutable selected-chip value to this existing owner; it adds no device,
state mirror, scheduler or VM dispatch path.

## Complete row mapping

| List-1 rows | Current owner and proof | Disposition |
| --- | --- | --- |
| PIT-F1 | `t_pit` owns exactly three counters and their port callbacks; `core_machine_pit_waveform_smoke` exercises all counters/modes. | Present, except the selected chip identity below. |
| PIT-F2 | `io_write_0043` decodes all selectors but unconditionally interprets `SC=11` as 8254 Read-Back. `t_pit` has no chip personality. | **Gap A:** select 8253 and reject the 8254-only command at this owner. |
| PIT-F3--F5 | `core_machine_pit_write`, `core_machine_pit_read`, latch/count/BCD helpers and the waveform/readback smokes provide the sole control/data/read route. | Present for the shared mechanism; status read-back must remain 8254-only after Gap A. |
| PIT-F6--F12 | `core_machine_pit_tick_mode0` through `core_machine_pit_tick_mode5`, `core_machine_pit_set_gate`, output callbacks and `core_machine_pit_ticks_until_output` implement the sole deterministic mode/GATE/OUT/deadline route. | Present; focused waveform smoke covers all six modes, zero counts, BCD and gate/output cases. |
| PIT-F13 | `core_machine_pit_reset` clears Core-owned emulation state only; it does not claim an Intel power-on register image. | Retained L1 deterministic Core lifecycle boundary. |
| PIT-F14 | No electrical nanosecond conversion exists in PIT code. | Retained L4 exclusion. |
| PIT-X1 | `core_machine_pit_initialize` installs the shared four-port block at `40h`; XT construction calls the same Core path. | Present topology; Gap A is required to make the selected part 8253 rather than the current 8254 superset. |
| PIT-X2 | `machine.c` binds shared counter 0 to the sole PIC IRQ0 source; `core_machine_pit_irq0_s2_smoke` proves the route and reset release. | Present. |
| PIT-X3 | `core_machine_configure_dma` binds shared counter 1 through `core_machine_dma_refresh_pit_output` to the sole DMA refresh request; DMA owns the transfer. | Present causal route; exact physical-axis conversion remains L2. |
| PIT-X4 | Existing PC/AT board helpers bind counter 2 and port-B speaker semantics, but XT PPI port-61 ownership is deliberately outside this unit and is the next 8255 T. | Present PIT output ownership; transfer PPI board-bit audit to T491 candidate. |
| PIT-T1 | `machine_scheduler.c` advances PIT only through the copied `pit_clock` domain; no XT profile supplies a sourced 1.19318-MHz compatible clock value. | Manual-L3 input fact; retained L2 integration boundary. |
| PIT-T2 | The same single scheduler/clock-domain route has no unqualified elapsed-axis conversion. | Retained L2 integration boundary. |
| PIT-T3 | Core reset has deterministic release behavior but no claimed chip register image. | Retained L1 chip-state boundary. |
| PIT-T4 | Counter-1 output reaches DMA, but no selected XT board formula is converted into the Core axis. | Manual-L3 board formula; retained L2 integration boundary. |

## Corroboration and finite implementation batch

The Intel 8253 source makes `SC=11` illegal but does not specify its precise
resulting state. A read-only 86Box corroboration of its `pit.c` `pit_write`
also gates Read-Back on its `PIT_8254` flag. It does not override the Intel
source and is not copied. The project fallback for the 8253 illegal encoding
is a no-state-change control write: this is a deterministic L1 choice, not a
Manual-L3 claim.

The only implementation batch is therefore:

1. add one validated, copied shared-PIT personality to the existing Core
   construction config and `t_pit`, defaulting to 8254 for existing users;
2. make `SC=11` execute the existing 8254 Read-Back path only for that
   personality and otherwise leave the 8253 state untouched;
3. select 8253 once in the immutable IBM 5160 profile; and
4. preserve the 8254 read-back proof and add focused 8253/profile assertions.

This batch has one state owner, one port route and one configuration direction.
There is no additional PIT, XT callback, scheduler, VM command or timing
conversion work. The subsequent 8255 T owns the selected XT PPI board bits;
the board/physical-axis tasks own the retained timing conversions.
