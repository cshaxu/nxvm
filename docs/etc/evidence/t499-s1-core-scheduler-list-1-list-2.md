# T499 S1 Core Scheduler List 1 And List 2

## Scope And Method

This is the complete inventory of the three production callbacks installed by
`core_machine_cold_reset()` and re-armed by `machine_scheduler.c`.  It is not a
new timing authority: hardware relations come from the retained controller
List 1/List 2 records below.  The current source was searched with:

```text
rg -n "core_machine_(arbitration_tick|readiness_tick|peripheral_tick)|due_tick + 1" src/core tests
rg -n "ticks_until|next_.*tick|due_tick|remaining_ticks|advance_at" src/core/machine
```

The first query has exactly three production recurring clients, all scheduled
at tick 1 by `machine.c`: arbitration, readiness and peripheral maintenance.
Each re-arms itself at `due_tick + 1`.  No profile, VM, renderer, or host
waiting path is a fourth scheduler client.

The cited external implementations were inspected only as read-only
cross-checks under the source policy.  86Box, Bochs and MAME use one
CPU-progressed/virtual-time timer order; QEMU keeps guest virtual time separate
from host/realtime clocks; PCjs supplies only an External-L2 lead where its
comments or behavior are BIOS-compatible rather than manual-defined.  No
source, structure, or timing value is imported from any reference.

## List 1: Next-Change Authority

| Owner and current callback group | Earliest meaningful transition | Authority and cross-check | Required scheduler disposition |
| --- | --- | --- | --- |
| Timeline | Earliest already-scheduled token; equal due ticks settle by insertion sequence. | Core contract; the existing timeline smoke proves order/cancel/nested events. Bochs/MAME corroborate ordered equal-time settlement. | Core-private exact event. Retain one timeline, not a controller scheduler. |
| PIT and auxiliary PIT | Next `OUT` transition of any armed counter, converted through the copied rational clock. | `t490-s2-8253-function-timing-list-1.md`, `t490-s3-8253-current-code-gap-list-2.md`; Intel/IBM mode and board relations; 86Box/MAME/Bochs cross-check timer-edge delivery. | Manual-L3 where the selected clock input is L3; selected fallback ratio remains External-L2/internal L2 as already labelled. `core_machine_pit_ticks_until_output()` exists. |
| RTC | Next periodic/update/alarm IRQ transition, converted through the copied RTC clock. | `t463-s2-rtc-phase-owner.md`; MC146818A relations, with 86Box/MAME/Bochs/QEMU calendar timer cross-check. | Manual-L3 with an L3 plan; existing L2 plan remains L2. `core_machine_rtc_ticks_until_irq()` exists. |
| DMA transaction service | Next qualified DMA service state after a pending DREQ, DACK/hold and selected board service clock. | `t489-s2-8237a-function-timing-list-1.md`, `t489-s3-8237a-current-code-gap-list-2.md`; Intel state sequence and IBM XT five-clock/refresh formula; 86Box/MAME/Bochs signal wiring cross-check. | Manual-L3 only when the selected Core clock/board input proves the conversion; otherwise existing L2 rate or explicit active blocker. No standalone DMA time owner. |
| PIC delivery | IRQ input change or completed CPU acknowledgement; the PIC itself has no recurring clock. | `t456-s1-pic-source-reference-contract-reconciliation.md`; 8259A logic/order, cross-checked by 86Box/MAME/Bochs/QEMU. | No independent event. Settle after the owning input transition in fixed Core order; never schedule a PIC poll. |
| XT refresh/PPI route | PIT channel-1 output drives DMA refresh request. | `t490-s2-8253-function-timing-list-1.md` rows PIT-X3/T4 and `t489-s2-8237a-function-timing-list-1.md` rows DMA-T2/T4; IBM board formula; 86Box/MAME wiring cross-check. | A PIT-derived event, then DMA/PIC settlement. No PPI/global poll. |
| DeskPro D4 refresh hold | A pending hold can occupy one Core bus transaction, but no source-qualified duration is present. | `t419-s2-d4-dma-grant-wait.md` and `t419-s3-d4-dma-busready.md`; no compatible manual or reproducible external duration receiver. | L1 causal blocker while active. It must be visible to the one Core seam, not converted to a guessed deadline. |
| FDC reset, seek and byte gates | Existing absolute reset, seek, DMA-byte and NDMA-byte due fields. | `t492-s2-8272a-function-timing-list-1.md`, `t492-s3-8272a-current-code-gap-list-2.md`, `t375-s19-fdc-service-time-crosswalk.md`; 8272A data and 86Box/MAME/Bochs cross-check. | Existing absolute values are eligible exactly at their recorded provenance: Manual-L3 fact plus L2 conversion where no selected Core scale exists. Add one owner-local minimum query. |
| FDC pending command/complete and media readiness | Command/result phases are causal; host-media generation/ready changes are external input changes, not an elapsed guest deadline. | Same FDC List 1/List 2; external models corroborate phase order but do not supply a transferable host-media polling cadence. | Pending command/complete is L1 causal until it is settled at the current Core boundary. Media change must move to the existing external media mutation/readiness boundary; no periodic refresh poll remains. |
| HDC personalities | A pending command/read/write phase has no source-qualified service duration in the selected ATA, WD1003, Compaq/WD, or Xebec records. | `t494-s2-xebec-function-timing-list-1.md`, `t494-s3-xebec-current-code-gap-list-2.md`, `t479-s3-immutable-hdc-personality-contract.md`; external models are only behavior cross-checks. | L1 causal blocker while a phase is active. The owner may settle its immediate phase at the current boundary, but cannot fast-forward it or invent a disk delay. Idle HDC is unscheduled. |
| KBC 8042 serial, response and typematic | Minimum nonzero serial, response or typematic remaining count in the KBC clock domain. | `t351-s1-kbc-aux-ledger.md`, `t464-s3-kbc-closure-audit.md`; copied construction inputs are L3 receivers; Bochs/86Box cross-check the queue-before-IRQ pattern. | Input-L3 where selected values are sourced, otherwise retained L2. Add a value-only minimum query; no VM setter or KBC scheduler. |
| XT keyboard | BAT or serial-delivery remaining count. | `t496-s1-xt-keyboard-original-source-ledger.md` and its List 1/List 2; IBM keyboard relation plus 86Box/PCjs cross-check. | Existing `core_machine_xt_keyboard_ticks_until_event()` is the one owner-local query. Its conversion retains its existing input provenance. |
| VADP CGA/EGA | Next raster-status boundary only when a sourced copied VADP clock/timing declaration makes that state guest-observable. | `t493-s2-cga-function-timing-list-1.md`, `t493-s3-cga-current-code-gap-list-2.md`, `t480-s2-vadp-code-gap-ledger.md`; IBM/Motorola relations plus 86Box/MAME/PCjs cross-check. | Input-L3 receiver with selected-value split. Add a local next-status-boundary query; output-disabled/no-timing state is idle, and copied presentation is never a clock owner. |
| Execution provider | Optional `advance_time` callback has no product implementation; the VM provider explicitly supplies `STD_NULL`. | `execution_provider.h` and `vm/composition/session/lifecycle.c`; no hardware source applies. | Current product is idle. A future non-null provider must publish a bounded Core-private next change or become an explicit blocker; it cannot receive every-tick maintenance by default. |

## List 2: Current Code Gap And Receiver

| Production path | Current behavior | Gap | Sole receiver subtask |
| --- | --- | --- | --- |
| `machine.c` reset | Schedules arbitration, readiness and peripheral callbacks at tick 1. | Creates the three perpetual maintenance roots. | S2 removes them and leaves timeline tokens for real one-shot events only. |
| `core_machine_arbitration_tick` | Advances DMA/PIT by one source tick, services D4/DMA, then refreshes PIC and re-arms. | Mixes a real PIT deadline with DMA, D4 and PIC polling. | S3 composes the earliest qualifying DMA/PIT event, settles D4 as blocker, then PIC once. |
| `core_machine_readiness_tick` | Calls FDC advance/refresh and HDC advance/refresh every tick, then advances RTC one tick. | FDC absolute due facts are ignored; HDC has a hidden phase-per-tick behavior; media is polled. | S4 queries FDC due fields, exposes HDC active blocker, and removes media polling. |
| `core_machine_peripheral_tick` | Advances KBC/XT keyboard and VADP every tick. | KBC and VADP have state that can advance by delta but no composed next-change query; product provider is also called after every publication. | S5 adds owner-local queries, settles delta once, and gives provider an explicit idle/blocker contract. |
| `core_machine_capture_time_observation_private` | Considers PIT/RTC/XT keyboard but falls back to `elapsed + 1` for DMA/FDC/KBC/HDC/D4/slave-PIC work. | The fallback hides the active owner and retains the polling route. | S2 replaces it with one private composition result: exact, idle, or active blocker. |
| `core_machine_publish_elapsed_ticks` | Is the sole elapsed-time writer but advances the timeline after the whole published delta. | A large CPU retirement can skip the intended interleaving point unless settlement limits it to the earliest event. | S2 makes this writer settle every due group up to its requested target without a second elapsed axis. |
| Dirty timeline heap candidate | Improves the selection cost of existing timeline tokens. | It preserves all three `due_tick + 1` clients and cannot meet the task exit criterion alone. | Review only after S2; retain only if its equal-due/cancel behavior passes the final single-seam tests. |

## S1 Result

`CORE-SCHEDULER-LIST-1`: all current periodic clients have a source and
next-change/idle/blocker disposition.

`CORE-SCHEDULER-LIST-2`: every current polling route has one designated
receiver.  No implementation is accepted in S1.  In particular, the dirty
timeline heap and `elapsed_ticks + 1` fallback are not treated as a partial
solution: the former is a data-structure candidate and the latter is the
precise polling shape that S2--S6 must remove.
