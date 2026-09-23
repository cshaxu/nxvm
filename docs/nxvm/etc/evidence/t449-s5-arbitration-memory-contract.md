# T449 S5: Arbitration And Memory Classification Closure

`M5:T449:S5:ARBITRATION-MEMORY-CONTRACT:OK`

## Arbitration Ledger

| Contender | Sole production route | Commit/release/reset disposition |
| --- | --- | --- |
| CPU | CPU execution owns retirement; `machine.c` waits only before final CPU publication. | Transaction state rejects a begin while another owner or unacknowledged HOLD exists; cold reset cancels and releases it. |
| DMA | `machine_scheduler.c` is the only production DMA advance caller and supplies the shared transaction state. | 286/386 HOLD request, acknowledge, transfer and release are one bracket; failed memory operation cancels before DMA progress. |
| Refresh | `machine_scheduler.c` is the only refresh HOLD producer. | It acknowledges, commits logical occupancy only, releases, and cold reset clears pending refresh/HOLD state. |

`core_machine_dma_advance` has no production caller. It remains the DMA
module's direct unit-test entry point with a null transaction, not a second
machine path or public machine operation.

## Memory-Class Ledger

| Class | Sole owner/disposition |
| --- | --- |
| Ordinary RAM, registered device and immutable ROM | `memory.c` resolves one frozen route before each physical read/write/query; a provider may explicitly decline only to ordinary RAM, while terminal failures do not fall through. ROM is a registered immutable provider, not a CPU/DMA exception path. |
| A20 | `memory.c` applies the selected wrap policy before provider/ordinary-RAM routing; the existing A20 port changes the same RAM-owned flag. |
| Parity | `memory.c` writes/checks the parity store at its ordinary-RAM read/write boundary and calls the board-owned fault observer once; the board owns latch/NMI delivery. |
| Reset | `core_machine_cold_reset` resets memory plus transaction/arbitration dynamic state; immutable mappings and frozen route definitions remain configuration-owned. |

No CPU, DMA, firmware or board code contains a separate RAM/ROM/A20/parity
classification decision. Direct physical memory calls are consumers of the one
memory owner; stopped APIs are intentionally bounded observations/mutations.
Unsupported electrical grant and memory latency remain explicit L2, not a
synthetic wait model.

## Verification And S6 Prerequisite

Focused competition, transaction lifecycle, DMA, checked-memory, immutable-ROM
and planar-parity regressions pass. Documentation governance and `git diff
--check` pass. S6 may now reconcile the sole retirement/observation path and
perform the T-level route/evidence audit; it cannot add a second arbitrator or
memory classifier.
