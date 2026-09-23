# T419 S4 D4 Refresh Hold Bridge

M5:T419:S4:D4-REFRESH-HOLD:OK

## Evidence Tier

`original`. The approved Compaq DeskPro 386/16 D3PE processor material
describes counter-1 refresh requests, REFRS arbitration, one refresh cycle,
and back-to-back refresh and DMA holds when a request remains pending. It is
design evidence only; no source, firmware, or local research path is imported.

## Implemented Contract

The D4 counter-1 low pulse queues one Core-owned refresh hold. At the next
shared arbitration boundary, Core emits refresh hold request, acknowledge,
refresh memory-cycle begin/commit, and release before its existing pending DMA
grant route. The bridge represents bus occupation only: it does not claim a
calibrated duration, DRAM electrical refresh, row retention, CPU BWAIT, or a
second VM transaction path.

The request is private D4 state, advances one bounded refresh address token on
service, and is cleared on cold reset. No non-D4 configuration wires the
counter output to this request path.

## Verification

`core-machine-d4-refresh-hold-smoke` proves that a D4 pulse queues one hold,
the refresh hold lifecycle precedes a pending DMA memory write without an
intervening CPU transaction, reset clears the request state, and a non-D4
machine produces no refresh transaction. The focused S4 smoke and retained
T417/T419 S1-S3/Model-40 smokes passed. The full `run-current-smokes` gate
passed. The rebuilt T419 developer artifact is `nxvm_0_5_0419.exe`, SHA-256
`B25078C401D466C54BC08445BB3E7A4846A0491B58D17052332D143D655DC5D6`.

## Remaining Transfer

Calibrated BCLK/DCLK timing, physical refresh electrical effects and row
retention, CPU BWAIT, PAL-level sequencing, peripheral BUSRDY wiring, and
external bus-master behavior remain outside this discrete bridge.
