# T375 S17: Model-339 Virtual-Time Source

## Decision

The selected Model-339 session now owns a VM-platform monotonic virtual-time
source at its declared nominal 8 MHz source rate. It is installed only when no
caller supplied source. Default PC/AT remains source-free.

The platform source retains the prior counter unit, fractional conversion
remainder, unit frequency, and initialized state. Its first sample and a reset
sample return zero and establish a new baseline. Later samples convert elapsed
monotonic units plus the retained remainder into source ticks; the existing
`vm_session_virtual_time_on_waiting` path remains the only caller that can
publish a nonzero batch through `core_machine_advance_time`.

This is host pacing for a stopped CPU, not an IBM board-duration fact. It does
not change CPU retirement publication, device-local service timing, waits,
PIC propagation, or physical waveform claims. A future replay artifact must
retain published batches before deterministic external replay can be claimed.

## Proof

`vm-model339-virtual-time-s17-smoke` passed:

```text
M5:T375:S17:MODEL339-VIRTUAL-TIME:OK
M5:T375:S6:VIRTUAL-TIME-SOURCE:OK
M5:T375:S4:EXPLICIT-MACHINE-TIME:OK
M5:T350:S2:PIT-IRQ0:OK
```

It proves Model-339 source installation, zero first sample, a positive
post-wait sample, reset rebasing, and default-profile exclusion. The S6/S4
proofs retain the one provider-to-core path and waiting-state restriction.

## Transfer

This closes only HLT-time source binding. The recorded-batch/replay policy,
bounded long host-pause policy, board availability/waits, DMA/FDC service,
RTC/PIC phase, KBC serial timing, CGA contention/raster, and final Model-339
L3 audit remain open.

`M5:T375:S17:MODEL339-VIRTUAL-TIME:OK`
