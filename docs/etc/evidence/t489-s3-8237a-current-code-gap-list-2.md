# T489 S3 IBM 5160 8237A Current-Code Gap List 2

`M5:T489:S3:8237A-LIST-2:ACCEPTANCE-CANDIDATE`

All selected DMA state is owned by `src/core/machine/dma.c`; Core construction
owns registration and `machine_scheduler.c` is the sole DMA advance/transaction
consumer. The audit maps the complete 16-row List 1 as follows.

| List-1 rows | Current owner, route and proof | Disposition |
| --- | --- | --- |
| DMA-R1--R4 | `dma.c` owns address/count, first-last flip-flop, command/mode/request/mask/status/temp, port read/write and reset. `core-machine-dma-channel-smoke` covers programming, page state, read-to-clear status and reset/finalize. | Present, one owner. |
| DMA-F1 | `dma_pending_requests`, `GetRegTopId`, request bindings and `dma_service_begin/end` own request, mask, priority and logical DACK. Focused DMA channel and binding-token smokes cover it. | Present, one owner. |
| DMA-F2 | `dma_service_advance` models S1--S4/SW; `core_machine_arbitration_tick` is the only production advance/transaction route. | Present logical order; elapsed source rate is the DMA-T3 gap. |
| DMA-F3--F6 | `Execute`, `Transmission`, `dma_complete_transfer` and M2M phases own demand/single/block/cascade, transfer direction, EOP/TC, auto-init and M2M. The channel smoke exercises phases, terminal behavior and failure cancellation. | Present, one owner. |
| DMA-F7 | `dma_service_advance` tests `VDMA_COMMAND_TM` and changes S2 directly to S4; high-address change restores S1. | Present, one owner. |
| DMA-T1 | XT profile selects one DMA controller; `core_machine_dma_initialize` correctly suppresses `C0h`--`DEh` secondary controller ports. However it unconditionally registers page ports `87h`, `89h`, `8Ah`, `8Bh` and `8Fh`, which are not part of the selected XT page window. | Gap A: make page-port registration topology-specific at the existing owner. |
| DMA-T2 | XT profile freezes `dma = { fdc_channel=2, refresh_channel=1, cascade_channel=0 }`; Core binds FDC through one opaque DMA request binding. | Present route; each producer's service timing stays at its own unit. |
| DMA-T3--T4 | `machine_scheduler.c` advances the one DMA clock domain and Core controller-timing validation can label a source rational clock only when its ratio is relative to a qualified Core elapsed axis. The XT axis is not physically qualified; IBM's five-clock/1.05 microsecond transfer and 72-clock refresh formula cannot truthfully be converted into its current ticks. | Explicit L2 boundary, not a code gap. Preserve the manual formula for the later Core physical-time-axis receiver; do not add a scheduler, VM tick path or false L3 declaration. |
| DMA-T5 | No electrical pin waveform model exists, as required. | Explicit L4 boundary; no gap. |

The implementation batch is exactly one owner-local correction: selected XT
page-port registration. It does not authorize a second DMA state object,
scheduler, profile runtime setter, device-service estimate, false timing claim
or AT-controller path. S4 must repair that complete topology gap, add focused
XT regression coverage and preserve the complete DMA channel suite.
