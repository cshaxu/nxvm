# T433 S3: Core Transaction And Time Reconciliation

`M5:T433:S3:TRANSACTION-LEDGER:OK`

| ID | Current owner / lifecycle | Regression owner | Disposition |
| --- | --- | --- | --- |
| `TIME-CLOCK` | `clock.h` and `timeline.h` are advanced only through `machine.c`; rational domains reset and callbacks are re-armed by cold reset. | `core-machine-rational-clock-smoke`, `core-machine-timeline-s2-smoke` | Conformant deterministic L2/L3 scheduling contract; oscillator phase/physical rate remains explicit L2 transfer. |
| `TIME-LIFECYCLE` | `machine.c:core_machine_cold_reset` clears CPU/DMA transaction state, counters and clocks before provider/firmware reset. | `core-machine-transaction-lifecycle-s4-smoke` | Conformant; no stale transaction or timeline state crosses cold reset. |
| `TXN-MEMORY` | `transaction.h` supplies the CPU/DMA validation-to-commit lifecycle over checked memory; stopped APIs remain outside executing arbitration. | `core-machine-transaction-s2-smoke`, `core-machine-checked-memory-smoke` | Conformant selected logical transaction; DRAM/READY wait values remain L2 transfer. |
| `TXN-PORT` | `port.h` registry owns provider dispatch/atomic restoration; CPU transaction caller commits only after successful provider work. | `core-machine-port-ownership-smoke`, `core-machine-port-io-s55-smoke` | Conformant selected logical I/O; electrical bus timing remains L2 transfer. |
| `TXN-ARBITRATION` | `machine.c` has one arbitration callback; DMA request/grant/HOLD state is owned by transaction/DMA and reset releases it. | `core-machine-arbitration-s3-smoke`, `core-machine-transaction-lifecycle-s4-smoke` | Conformant deterministic order; physical HOLD/HLDA/DREQ/DACK remains explicit L2 transfer. |
| `MEM-RAM-A20-PARITY` | Checked RAM/A20/parity owners remain `memory.*` and machine parity integration; reset retains configured mapping and clears latches. | `core-machine-checked-memory-smoke`, `core-machine-planar-parity-nmi-s3-smoke` | Conformant selected logical contract; electrical memory/controller timing remains L2 transfer. |

T354/T369 remain the source-ledger basis. Current code and the focused replay
above confirm there is no second transaction or elapsed-time owner. No runtime,
ABI or artifact change is made by S3.