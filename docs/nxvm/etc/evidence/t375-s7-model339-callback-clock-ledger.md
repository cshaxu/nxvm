# T375 S7: Model-339 Callback, Clock And Service Ledger

## Scope

This executable-model audit records the relation between a published machine
source tick, the three recurring timeline callbacks, delivered device ticks,
and selected Model-339 board evidence. A deterministic callback is not a
hardware oscillator, controller service period, bus phase or physical waveform.

The only production machine-time publishers are successful CPU retirement and
the S6 VM-composition source boundary. The latter accepts already-selected
source ticks and is neither host sleep nor a board clock. Both use the one
`core_machine_publish_elapsed_ticks` and timeline route.

## Complete Callback Ledger

| Consumer | Actual route | Model-339 source status | Decision / receiver |
| --- | --- | --- | --- |
| Callback cadence | Reset schedules arbitration, readiness and peripheral at source tick one; each reschedules at `due + 1`. | Project scheduler only. | Retain order; no sub-cycle claim. |
| DMA / HOLD | Arbitration calls `dma_clock.advance(1)`, then logical DMA grant/transfer. Descriptor is `1/1`. | IBM/8237A establish topology/state, not DMA rate, grant latency or DACK/AEN phase. | Generic cadence; later service/availability receiver. |
| PIT / IRQ0 / PIC | Arbitration calls `pit_clock.advance(1)`, PIT, then PIC refresh. | IBM 6280099 System Board 1-22: 1.193182 MHz `CLKIN`; S2 binds `596591/4000000`. | Keep PIT conversion; PIC/INTA phase remains open. |
| FDC / DMA2 / IRQ6 | Readiness calls `fdc_advance` then refresh once per callback. No clock-domain input exists. | IBM/uPD765 facts do not identify timing for the selected aftermarket 1.44 MB controller/drive chain. | Ordering only; service/rotation/DRQ pace transfers. Raw-IMG sidecar is separate. |
| HDC | Readiness invokes shared HDC advance/refresh. | Absent from Model 339. | No 5170 claim; ATA/HDC remains current-product work. |
| RTC / CMOS / IRQ8 | Readiness calls `rtc_clock.advance(1)` and advances RTC by delivered ticks. | IBM 6280099 System Board 1-57: 32.768 kHz; S2 binds `64/15625` and 32768 ticks/second. | Keep conversion; IRQ phase remains open. |
| KBC / keyboard / IRQ1 | Peripheral calls `kbc_clock.advance(1)`, then KBC advance. Descriptor is `1/1`; delay fields are zero. | IBM/UPI protocol provides no source-domain conversion. | Preserve functional immediate state; serial/controller duration transfers. |
| CGA / VADP | Peripheral calls `vadp_clock.advance(1)`, then synthetic VADP text state. Descriptor is `1/1`; text geometry is `{48,8,8}`. | CGA topology/register facts do not map this cadence to dot clock, scanline, retrace or contention. | Presentation only; raster/availability stays open. |
| Execution provider | Scheduler advances `provider_clock` from elapsed ticks then calls its frozen provider if present. Descriptor is `1/1`. | Generic core provider route, not a Model-339 device clock. | Retain reset/publication ownership; infer no board rate. |

## Reset, HLT And Ordering

`core_machine_reset` resets all six clock domains and schedules the callbacks
at tick one. S4 prevents retained `HLT` from being charged as CPU retirement.
S6 permits one explicit nonzero source batch only at active, non-step
`WAITING_FOR_INTERRUPT`; reset rebases its source before core reset. Thus no
host loop or reset callback manufactures device time.

At a shared due tick, logical order is arbitration (`DMA`, `PIT`, `PIC`),
readiness (`FDC`, `HDC`, `RTC`), then peripheral (`KBC`, `VADP`). It is not
proof of electrical propagation or physical timing phase.

## Reproducible Sweep

`tools/Verify-Model339CallbackClockLedger.ps1` checks the profile PIT/RTC
conversions, all callbacks/domains/device routes, reset schedule, S6 source
publication, and that no other production C file calls
`core_machine_advance_time`. The local replay passes:

```text
M5:T375:S7:MODEL339-CALLBACK-CLOCK-LEDGER:OK
```

## Result And Transfer

No numeric repair is admitted. Only PIT and RTC have a primary-source
Model-339 source-domain conversion. DMA, FDC, KBC and VADP must not become
"more accurate" by changing generic ratios; each needs selected-board/component
evidence and a service contract, or an explicit reference-exhausted final-audit
transfer. CPU/ISA availability, PIC/INTA, RTC IRQ phase, CGA
contention/retrace, FDC/drive mechanics and physical waveforms remain open
T375 work. This is not a Model-339 L3 result.
