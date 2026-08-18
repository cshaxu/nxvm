# T416 S2 DMA HOLD Locality Closure Audit

M5:T416:S2:DMA-HOLD-LOCALITY-CLOSURE:OK

Fresh coordinator review of P1 `dd459b30` confirms one existing Core
transaction receiver: only an acknowledged DMA HOLD invalidates the CPU
external-memory locality key. A request does not do so; release does not create
a second rule. The following committed CPU page-table read receives a page miss,
and reset clears the key and accumulated locality ticks. The focused locality
smoke and the retained CPU/DMA competition regression prove that lifecycle.

The original D3PE source proves DMA/HOLD/HLDA and refresh topology. It does not
prove a D4 row-retention rule or a calibrated DMA/refresh/BWAIT phase. T416 is
therefore accepted only as a generic-AT locality boundary. It preserves the
single Core transaction owner and Core/VM direction; Model-40 continues only
to select the existing generic 2 KiB, miss +2, hit +0 locality values.

T416 closes this bounded CPU/DMA ownership receiver. Exact D4 page retention,
DMA and refresh waveforms, BWAIT, locked/RMW shape, device service timing and
Model-L3 acceptance remain transferred. T417 is active for the next bounded
refresh-pulse receiver.