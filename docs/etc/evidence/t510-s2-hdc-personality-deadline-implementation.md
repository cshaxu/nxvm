# T510 S2 HDC Personality Deadline Implementation

`M5:T510:S2:HDC-DEADLINE-OWNER:OK`

## Implemented List-2 Batch

| Personality | Frozen VM input | Core behavior and proof |
| --- | --- | --- |
| default-AT ATA PIO | `200/200` Other-L2 command/next-sector quanta | Existing PIO route retains one command and one inter-sector deadline. `vm-hdc-port-smoke` asserts both inputs and the pending command deadline. |
| IBM 5170 WD1003/ST-506 | `16000/7840` Other-L2 command/next-sector quanta | The existing 8-MHz macro axis expresses 2 ms and 980 us without making a physical-time claim. `vm-ibm-5170-model-339-composition-smoke` proves the frozen values; `core-machine-hdc-smoke` proves that command and next-sector schedules are independently selected. |
| DeskPro Model 40 Compaq/WD | `0/0` | The previous unqualified `16000` is removed.  Its established PIO/IRQ14/shared-`3F7h` functionality remains, but a deadline at the current elapsed tick is deliberately not labelled L2. `vm-model40-hdc-s26-smoke` proves the explicit zero boundary. |
| IBM 5160 Xebec | `250/0` Other-L2 command quantum; no separate DMA inter-sector phase | Completing a six-byte DCB or initialize payload now enters `PENDING_COMMAND`, publishes the existing HDC due tick, and only then starts DMA or publishes a response/IRQ. `core-machine-xebec-wiring-smoke` proves the 250-step due tick and no pre-deadline IRQ. |

## Ownership And Simplification

`core_machine_hdc_service_config` replaces one false common scalar with the
two actual task-file transition meanings.  `core_machine_hdc_schedule_service`
remains the only setter of `next_service_tick`; `core_machine_hdc_advance_elapsed`
remains the only completion path; `core_machine_hdc_next_due_tick` remains the
only scheduler query.  The Xebec branch reuses those three HDC functions.

There is no HDC-specific scheduler, VM callback, controller object, mutable
timing setter, firmware polling workaround, parallel CHS state or media cache.
DMA remains the sole Xebec data mover.  Xebec's omitted inter-sector timer is
intentional: its current DMA callback has no distinct observable service phase,
and inventing one would claim a physical model not supplied by the sources.

## Similar-Issue Sweep And Verification

The `service_ticks` sweep finds no residual configuration.  Every construction
site now names `service.command_ticks` and `service.next_sector_ticks`; every
deadline consumer remains under `src/core/machine/hdc.c` and
`machine_scheduler.c`.  Focused ATA/WD1003/Compaq/Xebec/profile tests pass
8/8.  The complete repository-only unit gate passes 313/313 in 18.02 seconds;
documentation governance also passes.

Tracked code/test delta, calculated by `git diff --numstat` for the S2
implementation paths, is +72/-22, net +50 lines.  The increase is the one
explicit Xebec phase plus source-value assertions; it deletes the misleading
single-scalar interpretation and retains one production path.
