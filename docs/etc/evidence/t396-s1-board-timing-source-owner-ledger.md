# T396 S1 DeskPro Board-Timing Source And Owner Ledger

## Publisher And Consumer Boundary

`core_machine_publish_elapsed_ticks()` is the only point that makes elapsed
Core time visible. A successful instruction publishes its selected instruction
cost after retirement; deterministic sessions can also advance stopped or
paused time through `core_machine_advance_time()`. Model 40 is deterministic,
so neither publication path names a 16 MHz CPU cycle. T394's physical-mode
guard rejects an unallocated success before publication, but it does not make
Model 40 physical or select its copied descriptor.

At each due Core tick, the timeline invokes the following callbacks in this
order: arbitration, readiness, then peripheral. The callback order is the
only current visibility contract. It deliberately implies no same-tick
electrical phase, bus ownership duration, READY wait insertion, or propagation
delay.

## Reconciled Facts

| Domain | Confirmed fact | Current owner/state | Disposition |
| --- | --- | --- | --- |
| CPU | D3PE identifies 80386/16 and a phase-related 32 MHz oscillator. | Both Model-40 constructors use deterministic six-domain 1:1 plans. | No retirement-tick-to-cycle mapping. |
| Dual PIT | D3PE identifies two 8254s and 1.19318 MHz TIMCLK. | Core advances both through pit_clock; auxiliary block is at 48h. | No ratio or reset phase is published. |
| DMA | D3PE identifies 4 MHz DCLK and one wait per DMA cycle. | Core owns dual-8237 HOLD/ack/release through dma_clock. | No inferred ratio or wait placement. |
| PIC/RTC/KBC/NMI/reset | Logical topology/order is closed. | One Core publisher and deterministic callbacks. | No physical propagation/settle duration. |
| Port 61h/refresh | D3PE defines composite state and divisors 18/19. | D4/platform and PIT own functional state/reset. | Phase and physical period remain unselected. |

## Finite Consumer Ledger

| Domain / consumer | Model-40 declaration | Core owner and current logical delivery | Reset / visibility boundary | Source status and receiver |
| --- | --- | --- | --- | --- |
| CPU retirement | 80386/16; phase-related 32 MHz oscillator | `core_machine_publish_elapsed_ticks()` publishes completed instruction cost; private and BYOB constructors select deterministic time | Reset clears elapsed time and re-arms timeline at due tick 1; CPU retirement is visible before timeline advancement | D3PE frequency fact; no retirement-to-cycle unit or phase. A physical Model-40 descriptor and conversion contract are the next receiver. |
| DMA / dual 8237A | 4 MHz DCLK; one wait on every DMA cycle | `dma_clock` drives one logical DMA transaction advance before PIT/PIC | Clock phase resets to declared neutral zero; trace sees `DMA_ADVANCE`, not DCLK/READY phases | D3PE fact lacks Core-tick conversion and transaction-to-DCLK relation. Board clock-plan plus DMA wait-placement contract required. |
| System and auxiliary 8254 | Two 8254 blocks; 1.19318 MHz TIMCLK; auxiliary base 48h | One `pit_clock` advances both blocks after DMA; port-61 functional wiring remains in Core D4/platform owner | Both PITs reset before clock-domain reset; `PIT_ADVANCE` observes only aggregate logical ticks | Frequency is sourced, but reset phase and elapsed-tick conversion are absent. Physical PIT/port-61 observable contract required. |
| PIC master/slave | Selected topology/routing | Refreshed after DMA and both PITs in arbitration | PIC resets before timeline reset; same-due interrupt eligibility is logical and traceable as `PIC_REFRESH` | No D3PE settle/priority electrical timing scalar. Preserve order; device/firmware visibility receiver owns any needed observable delay. |
| RTC/CMOS | Selected RTC route and IRQ8 topology | `rtc_clock` advances only in readiness, after FDC/HDC service | RTC resets before its domain; earliest PIC arbitration follows on next due tick | No Model-40 oscillator/phase source in admitted corpus. Physical RTC timing remains a device/firmware receiver. |
| FDC/HDC readiness | Selected DMA2/IRQ6 and IRQ14 functional routes | FDC then HDC advance/refresh before RTC in readiness | Both reset before timeline; requests become eligible for next arbitration callback | Service cadence is not board DCLK/ATA/FDC timing. Transfer to physical-device/firmware timing. |
| KBC | Auxiliary absence is selected | `kbc_clock` advances after readiness | KBC resets before its domain; input is consumed at peripheral boundary | No source-backed keyboard controller clock/settle timing. Transfer to physical-device/firmware timing. |
| VADP / host presentation | No Model-40 physical video clock selected here | `vadp_clock` advances after KBC; presentation consumes copied state outside machine time | VADP resets before its domain; no host wall-clock coupling | Explicitly outside board-clock publication and L3. Retain existing display receiver. |
| Provider time | No Model-40 provider timing declaration | `provider_clock` advances after all due callbacks and may call an execution provider | Its domain resets with all Core domains | No current Model-40 provider consumer. Do not use it as a hidden board clock. |
| NMI, reset, port 61h | D3PE composite bit facts; D4 control topology | Core owns D4/platform latches, PIT functional input and reset transaction | Cold reset clears latches and domains, then invokes provider/firmware reset | Functional ownership is proven, but physical pulse/failsafe/IOCHK propagation is not. Physical-device/firmware receiver. |

## Blocking Contract

T394 qualifies a finite C0 semantic set but does not convert every successful
retirement into a 16 MHz cycle. The retained one-tick success is deterministic
progress. T396 therefore must not select a PIT/DCLK ratio until an exact
80386 physical-retirement-to-board-clock consumer contract proves unit, phase,
reset and transaction visibility. Generic AT or emulator behavior is not
Model-40 authority.

## Verification

- Static publisher/consumer sweep: `rg -n "core_machine_clock_domain_advance|core_machine_advance_time|retirement_time_contract|clock_plan" src tests -g "*.c" -g "*.h"` found the six Core domain advances, both deterministic Model-40 constructors, rational-clock coverage and no Model-40 physical descriptor consumer.
- Focused regression command: `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R "current\.(core-machine-rational-clock-smoke|core-machine-auxiliary-pit-s3-smoke|core-machine-d4-platform-s4-smoke|vm-model40-private-composition-s7-smoke|vm-model40-integration-s8-smoke)$"` passed all five tests.
- The sweep treats Model 339 and generic PC/AT clock plans as separate profile contracts, never as a Model-40 source.
## Implementation Decision

No production timing change is admissible in S1. The neutral 1:1 plan is
not a provisional frequency; it is the only declaration compatible with the
deterministic Model-40 retirement contract. Changing it would couple every
consumer above to an invented unit and violate the Core/VM split: VM may select
a sourced board contract, while Core remains the common rational-clock and
transaction owner.

## Next Batch

The next admitted S must either supply an exact primary-backed Model-40
retirement-to-board-clock conversion, reset phase and a named observable
consumer, or close this board-clock candidate with the following explicit
transfers: DMA wait placement, PIT/port-61 phase, PIC/RTC/KBC/NMI propagation,
and FDC/HDC service timing. Generic AT/emulator implementations remain
secondary observations only and cannot satisfy that source requirement.