# T387 S1: Model-40 Board And Device Timing Inventory

`M5:T387:S1:MODEL40-BOARD-TIMING-INVENTORY:OK`

## Current Timing Graph

`core_machine_publish_elapsed_ticks()` is the sole successful-retirement
publisher. At each due tick Core orders three callbacks: arbitration
(DMA -> both PITs -> PIC refresh), readiness (FDC service/refresh -> Compaq
HDC service/refresh -> RTC), then peripheral (8042 -> VADP). A source made
ready by readiness is therefore eligible to PIC arbitration on the following
due tick. Reset clears timeline and every clock domain before re-scheduling all
three callbacks at tick one. This is deterministic order, not a Model-40
frequency or bus-phase contract.

## Source-To-Owner Ledger

| Boundary | Primary/source status | Current owner and gap | Next receiver |
| --- | --- | --- | --- |
| CPU/D4 bus and expansion slots | D3PE: CPU is phase-related to 32 MHz; expansion bus is re-synchronized to simulate 286 timing and averages below 8 MHz. No exact whole-cycle conversion is retained. | Shared retirement publisher and transaction owner; Model-40 uses neutral clock plan. | Source/measurement contract before board availability or wait implementation. |
| DMA | D3PE specifies 4 MHz DCLK and one wait on all DMA cycles. | Core dual-8237 transaction/arbitration owner has logical HOLD/ack/release but no DCLK ratio or wait publication. | First bounded implementation candidate after source-to-project-tick conversion and consumer trace are defined. |
| PIT and CPU-to-PIT | D3PE specifies 1.19318 MHz source for both 8254 blocks; S25 explicitly transfers CPU-to-PIT calibration. | Shared PIT domains advance together; second PIT has no output consumer. | Clock-ratio receiver, then source-backed counter-1/port-61 observation replay. |
| PIC/RTC/8042/NMI/reset | Primary corpus fixes topology/routes but retained evidence has no propagation/settle scalar. | Core has logical lifecycle and deterministic equal-tick order. | Board source/probe ledger; do not invent latency. |
| FDC/DMA2/IRQ6 | Compaq source fixes 500 kbit/s, ports, DMA2 and IRQ6; T386 establishes logical service. | FDC has generic 500 kbit/s cadence, but no Model-40 controller-to-DCLK/bus availability conversion. | DeskPro storage service and bus arbitration receiver. |
| Compaq HDC/IRQ14 | Primary register/command contract complete; raw IMG excludes physical drive service. | Core Compaq HDC service is logical readiness work. | Board service timing plus existing physical-media TODO. |
| CECG/VADP | CECG function is complete only for selected digital controls; monitor/raster/ISA waits remain deferred. | VADP is one shared clock-domain owner. | CECG board timing receiver and retained CECG TODO; no IBM-EGA borrowing. |
| ROM/RAM replacement | Primary mapping and memory-cycle classes exist; exact published timing model not yet selected. | Core immutable mappings and D4 memory route are functional. | D4 availability/wait source-to-model receiver. |

## Disposition

No timing behavior changes in S1. The earliest implementable mechanism is a
source-labelled Model-40 clock-plan/consumer contract, beginning with the
CPU-to-PIT calibration and DCLK conversion only if the primary corpus or a
bounded owner-approved measurement supplies a project-tick mapping. Until
then, the 1:1 Model-40 plan is explicitly neutral and cannot support a L3
claim. Secondary emulators may only cross-check a primary-constrained range.

The next S must either obtain that conversion with a reproducible source/probe
contract or record its exact non-admission; it may not turn the "below 8 MHz"
bus statement into an invented scalar.