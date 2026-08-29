# T499 S1 Core Scheduler List 1 And List 2

## Scope And Method

This is the complete inventory of the three production callbacks installed by
`core_machine_cold_reset()` and re-armed by `machine_scheduler.c`.  It is not a
new timing authority: hardware relations come from the retained controller
List 1/List 2 records below.  The current source was searched with:

```text
rg -n "core_machine_(arbitration_tick|readiness_tick|peripheral_tick)|due_tick\\s*=.*\\+\\s*1" src/core tests
rg -n "ticks_until|next_.*tick|due_tick|remaining_ticks|advance_at" src/core/machine
```

The original route had exactly three production recurring clients, all
scheduled at tick 1 by `machine.c`: arbitration, readiness and peripheral
maintenance. Each re-armed itself at `due_tick + 1`. The post-migration query
retains only the explicitly conditional one-tick progression for an active L1
causal blocker; it is not a scheduled callback or an idle maintenance route.
A separate source review also found
the VM execution-provider `refresh` hook called before every CPU round; its
sole product route called empty FDD/HDD refresh functions. It is a redundant
maintenance path, not a fourth time owner, and is removed with its empty
media functions.

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
| VADP CGA/EGA | Next raster-status boundary only when a sourced copied VADP clock/timing declaration makes that state guest-observable. | `t493-s2-cga-function-timing-list-1.md`, `t493-s3-cga-current-code-gap-list-2.md`, `t480-s2-vadp-code-gap-ledger.md`; IBM/Motorola relations plus 86Box/MAME/PCjs cross-check. | Current profile construction supplies no source-qualified raster deadline, so VADP is an idle Core delta consumer and cannot wake HLT. Output-disabled/no-timing state is likewise idle; copied presentation is never a clock owner. |
| Execution provider | Optional `advance_time` callback has no product implementation; the VM provider explicitly supplies `STD_NULL`. | `execution_provider.h` and `vm/composition/session/lifecycle.c`; no hardware source applies. | The product provider retains reset only. Its former per-CPU `refresh` route reached empty FDD/HDD functions and is deleted. A future non-null advance callback receives completed Core delta only; it cannot receive every-tick maintenance or write guest time. |

## List 2: Current Code Gap And Receiver

| Production path | Current behavior | Gap | Sole receiver subtask |
| --- | --- | --- | --- |
| `machine.c` reset | Schedules arbitration, readiness and peripheral callbacks at tick 1. | Creates the three perpetual maintenance roots. | S2 removes them and leaves timeline tokens for real one-shot events only. |
| `core_machine_arbitration_tick` | Advances DMA/PIT by one source tick, services D4/DMA, then refreshes PIC and re-arms. | Mixes a real PIT deadline with DMA, D4 and PIC polling. | S3 composes the earliest qualifying DMA/PIT event, settles D4 as blocker, then PIC once. |
| `core_machine_readiness_tick` | Calls FDC advance/refresh and HDC advance/refresh every tick, then advances RTC one tick. | FDC absolute due facts are ignored; HDC has a hidden phase-per-tick behavior; media is polled. | S4 queries FDC due fields, exposes HDC active blocker, and removes media polling. |
| `core_machine_peripheral_tick` | Advances KBC/XT keyboard and VADP every tick. | KBC has a composable owner-local deadline. Current VADP has no source-qualified raster deadline, so it must remain an idle delta consumer rather than acquire a guessed query. | S5 settles delta once; KBC supplies its local query and VADP remains unable to wake HLT. |
| VM execution-provider refresh | Called before every CPU execution round and invokes empty FDD/HDD refresh functions. | It is a parallel no-op maintenance route outside Core's event seam. | S5 deletes the call chain; reset and completed-delta observation remain distinct interfaces. |
| `core_machine_capture_time_observation_private` | Previously hid unqualified active work behind the recurring maintenance route. | It did not distinguish an idle machine from an active owner lacking an admissible duration. | S2 returns either an exact deadline, idle state, or explicit active blocker. The scheduler's one-tick branch is permitted only for that active blocker during normal Core progression. |
| `core_machine_publish_elapsed_ticks` | Is the sole elapsed-time writer but advances the timeline after the whole published delta. | A large CPU retirement can skip the intended interleaving point unless settlement limits it to the earliest event. | S2 makes this writer settle every due group up to its requested target without a second elapsed axis. |
| Dirty timeline heap candidate | Improves the selection cost of existing timeline tokens. | It preserves all three `due_tick + 1` clients and cannot meet the task exit criterion alone. | Review only after S2; retain only if its equal-due/cancel behavior passes the final single-seam tests. |

## S1 Result

`CORE-SCHEDULER-LIST-1`: all current periodic clients have a source and
next-change/idle/blocker disposition.

`CORE-SCHEDULER-LIST-2`: every current polling route has one designated
receiver. No implementation is accepted in S1. In particular, the dirty
timeline heap and the old unconditional `elapsed_ticks + 1` maintenance route
are not treated as a partial solution: the former is a data-structure
candidate and the latter must be replaced by exact, idle, or active-blocker
classification. The latter classification may retain one-tick normal Core
progress only while that active L1 owner exists.
