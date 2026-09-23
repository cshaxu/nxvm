# T460 S2 DMA Logical Service Contract

The 8237A manual's DREQ-to-DACK relation is causal: after selection, a request
is acknowledged for the duration of its service and is released on terminal,
mode release, cancellation, transaction rejection or reset. It does not give
NXVM an electrical pin waveform or a guest-clock duration.

`dma.c` now records that relation once as the private `acknowledged` mask.
`dma_service_begin` is the sole ISR/DACK logical entry and
`dma_service_end` is the sole release. Existing terminal completion, demand/
single release, rejected routes, reset and paired-controller cascade all use
that release path. The mask is not a peripheral API, does not duplicate DRQ,
and does not create a scheduler: `machine_scheduler.c` remains the one T449
hold/transaction caller and `machine_board.c` remains the one binding adapter.

The sweep of `core_machine_dma_advance*`, pending request, assert/deassert/
terminate, all `VDMA_COMMAND_*` consumers and scheduler hold calls found no
second production route. Existing focused tests cover pending/priority, normal
and word service, terminal/EOP/auto-init, route failure, reset, binding
identity, FDC boundary and transaction competition. The reset regression now
also proves acknowledgement state is transient while the opaque binding
survives reset.

The focused five-test group passed after a configured rebuild, as did diff
hygiene. DREQ/DACK electrical polarity, HRQ/HLDA waves and elapsed service
clock values remain L4; selected AT signal binding remains S4. `DMA-R2`,
`DMA-F1` and `DMA-F2` now have one logical Core service/release owner, not a
parallel device handshake.

`M5:T460:S2:DMA-LOGICAL-SERVICE:OK`
