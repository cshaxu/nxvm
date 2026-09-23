# T449 S3: Transaction Classification And Lifecycle Closure

`M5:T449:S3:TRANSACTION-CLASSIFICATION:OK`

## Complete In-Scope Class Ledger

| Class | Caller and owner | Commit/cancel boundary | Reset/regression disposition |
| --- | --- | --- | --- |
| CPU fetch/data/page-table read | `cpu_instructions.c:_kma_read_physical`; CPU requests the shared transaction and `memory.c` validates the access. | Checked read commits; failed read cancels before `#CE`. Fetch versus data/page-table provenance remains CPU trace detail, not a second transaction kind. | `core_machine_cold_reset` resets the one transaction state. `core-machine-transaction-s2-smoke` and prefetch/locality regressions cover it. |
| CPU physical write | `cpu_instructions.c:_kma_write_physical`; CPU plus shared transaction owner. | Checked write commits; failed write cancels before `#CE`. | Same reset owner and transaction smoke. |
| CPU port read/write | `cpu_instructions.c:_p_input` / `_p_output`; port registry owns provider execution and restoration, while the CPU uses the shared transaction. | Provider failure cancels; only successful CPU publication commits. | Stopped port APIs remain outside guest transactions. Existing port and transaction regressions retain the boundary. |
| DMA device-to-memory | `dma.c` owns request/channel/preflight and uses the shared transaction for the checked memory write. | Preflight or write failure cancels before channel progress; successful write commits before count/terminal progress. | DMA reset and `core-machine-transaction-s2-smoke` retain proof. |
| DMA memory-to-device | `dma.c` owns request/channel/preflight and uses the shared transaction for the checked memory read. | Preflight/read failure cancels; successful provider delivery commits before channel progress. | Same DMA/reset proof. |
| DMA memory-to-memory | `dma.c` owns the one M2M branch and uses one shared copy transaction after both addresses are selected. | Failed read/write cancels before count/terminal mutation; completed copy commits. | Same DMA/reset proof. |
| Refresh occupancy | `machine_scheduler.c` is the only producer, with the shared HOLD and transaction owner. | An accepted logical refresh occupancy commits; it has no invented DRAM electrical result. | Cold reset resets transaction/HOLD state. D4 refresh-HOLD regression retains proof. |

## Explicit Transfers

`CPU_FPU_COMMAND` is not CPU-to-board memory/port traffic: it represents the
separate x87 ESC control boundary and remains its named x87 receiver. Device
command/DRQ/IRQ phases remain the next controller/device candidate. Physical
fetch overlap, READY duration, HOLD/HLDA waveform and controller service
duration remain explicit L2 transfers; none is converted to a synthetic class
or delay.

## Sweep Result

Every production `core_machine_transaction_begin`, `commit` and `cancel` is
accounted for by the seven rows above or the explicit x87 transfer. The only
reset is `core_machine_transaction_reset` from `core_machine_cold_reset`; it
cancels an active transaction, releases HOLD, preserves the copied trace
binding, then the machine resets timeline and dynamic owners. No duplicate
CPU/DMA direct commit path, unpaired cancellation route or missing reset owner
was found. Consequently S3 adds no code: adding a wrapper around these already
shared paths would violate the one-owner and no-additive-layer constraints.

Focused direct regressions pass the retained T354 S2 transaction, T354 S3
competition and T354 S4 lifecycle markers. Documentation governance and
`git diff --check` pass.

## S4 Prerequisite

S4 may consume the accepted single transaction class/lifecycle boundary to
close availability, BUSRDY and prefetch ownership. It must retain this class
ledger and cannot add another kind or transaction owner without a separately
admitted capability.
