# T375 S24: FDC Non-DMA Byte Cadence

S19 selects the TEAC FD-235HF-A540 1.44 MB route at 500 kbit/s.  Its
16-microsecond byte interval converts to 128 nominal Model-339 ticks.  S20
applied that input only to DRQ/DMA2 handoff, leaving the identical 8272A
non-DMA execution path able to consume successive `3F5h` data bytes without
elapsed time.

S24 makes byte eligibility one FDC-owner rule.  A successful non-DMA read,
write, or format data byte records the same 128-tick deadline used by the DMA
path.  Until that deadline, the execution MSR retains CB/NDM/direction state
but omits RQM, and the `3F5h` handler cannot consume or produce another byte.
At the due tick, the gate clears and normal RQM service resumes.  DMA keeps its
existing DRQ withdraw/reassert owner; no second DMA request route was added.

The focused FDC smoke preserves its pre-existing ordinary 250-kbit/s
non-DMA functional vectors, then selects CCR=0 and proves: first data byte;
RQM absent at due-minus-one; RQM present at due; second data byte; and DOR
reset cancellation.  It also retains the DMA, seek, IRQ6, result, media-error,
and reset paths.

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T375:S20:FDC-DMA-CADENCE:OK
M5:T375:S21:FDC-SEEK-CADENCE:OK
M5:T375:S24:FDC-NDMA-CADENCE:OK
```

## Boundary and sweep

The sweep covers all `3F5h` read/write execution routes, format, MSR, CCR,
DMA gate, transfer completion/error, DOR reset, controller reset, and
finalize.  The shared interval is a payload-byte availability contract only;
it does not model encoded gaps, sector search/index phase, motor spin-up,
250-kbit/s media, controller oscillator precision, or raw-IMG Deleted Data /
Control Mark / Scan fidelity.
