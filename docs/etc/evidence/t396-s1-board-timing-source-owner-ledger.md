# T396 S1 DeskPro Board-Timing Source And Owner Ledger

## Reconciled Facts

| Domain | Confirmed fact | Current owner/state | Disposition |
| --- | --- | --- | --- |
| CPU | D3PE identifies 80386/16 and a phase-related 32 MHz oscillator. | Both Model-40 constructors use deterministic six-domain 1:1 plans. | No retirement-tick-to-cycle mapping. |
| Dual PIT | D3PE identifies two 8254s and 1.19318 MHz TIMCLK. | Core advances both through pit_clock; auxiliary block is at 48h. | No ratio or reset phase is published. |
| DMA | D3PE identifies 4 MHz DCLK and one wait per DMA cycle. | Core owns dual-8237 HOLD/ack/release through dma_clock. | No inferred ratio or wait placement. |
| PIC/RTC/KBC/NMI/reset | Logical topology/order is closed. | One Core publisher and deterministic callbacks. | No physical propagation/settle duration. |
| Port 61h/refresh | D3PE defines composite state and divisors 18/19. | D4/platform and PIT own functional state/reset. | Phase and physical period remain unselected. |

## Blocking Contract

T394 qualifies a finite C0 semantic set but does not convert every successful
retirement into a 16 MHz cycle. The retained one-tick success is deterministic
progress. T396 therefore must not select a PIT/DCLK ratio until an exact
80386 physical-retirement-to-board-clock consumer contract proves unit, phase,
reset and transaction visibility. Generic AT or emulator behavior is not
Model-40 authority.

## Next Batch

Reconcile both Model-40 constructors, Core clock-domain initialization/advance/
reset, dual PIT/DMA/PIC/RTC/KBC/NMI consumers and focused regressions. Every
unresolved domain remains nonphysical with an explicit receiver.