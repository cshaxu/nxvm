# T375 S20: FDC DMA Byte Cadence

The Model-339 FDC owner now consumes the existing machine elapsed-tick
publisher through `core_machine_fdc_advance_at`.  At CCR data rate zero, the
selected TEAC FD-235HF-A540 500-kbit/s mode gates each DMA data byte by
`CORE_MACHINE_FDC_500K_BYTE_TICKS` (128 nominal 8-MHz ticks, or 16 us).

After a DMA byte, `fdc.c` withdraws its existing DMA2 request and records the
next eligible tick.  Readiness republishes the same request only after that
deadline.  The FDC still owns transfer state, DRQ timing and IRQ6 publication;
the existing DMA owner still selects/grants/transfers the byte and its terminal
state.  No second DMA route, test ingress, host clock, or imported code exists.

The focused FDC smoke configures demand-mode DMA2 for a two-byte 500-kbit/s
read. It proves the first request is eligible, one byte transfers, no request
is pending at tick 127 after that gate starts, the next request is eligible at
tick 128, and only after the second byte does the pending-completion path raise
IRQ6. It also proves DOR reset clears a pending gate. Observed markers:

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T375:S20:FDC-DMA-CADENCE:OK
M5:T269:S1:DMA-GRANT:PORT:OK
M5:T269:S4:DMA-MODES:OK
M5:T230:S3:DMA-CHANNEL:OK
M5:T348:S2:DMA-PORT-PAGE:OK
M5:T348:S3:DMA-REQUEST-CASCADE:OK
M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK
```

The 128-tick value follows the S19 indexed TEAC 500-kbit/s conversion; it is
not a claim about gap bytes, sector search, index phase, seek/recalibrate,
motor spin-up, controller command latency, 250-kbit/s media, non-DMA service,
or electrical waveform timing. Those remain explicit T375 receivers.
