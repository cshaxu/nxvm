# T370 S2: PIT/PIC/RTC Domain Reconciliation

## Decision

The selected Model-339 descriptor provides deterministic core ratios, not a
source-backed physical clock conversion: DMA `1/1`, PIT `1/4`, RTC `1/1`,
VADP `1/1`, KBC `1/1` and provider `1/1`, all relative to completed core
elapsed ticks. The configured RTC `ticks_per_second` is `50000`, but no IBM,
Intel or Motorola source maps that project quantity to the 8 MHz 80286 clock,
the 8254 input, or the MC146818 oscillator. Consequently none of those ratios
or `50000` is admitted as a Model-339 physical time scalar.

The existing core preserves one logical event graph and established controller
semantics. It is appropriate to retain it as a lifecycle/ordering foundation,
but not to label it device service timing. No S2 runtime change is warranted:
changing a ratio would select an unsupported conversion, and adding a second
clock/scheduler would violate the single-owner contract.

## Source-to-route disposition

| Device boundary | Determinate source and existing owner | Domain result |
| --- | --- | --- |
| 8254 counter state and IRQ0 | Intel 8254 defines programmable counters, modes, gates, output and latching; IBM connects the PC/AT timer path to IRQ0. `pit.c` owns ports/state/output and `machine.c` binds channel 0 to one PIC source. T350 S2 proves output/IRQ/reset lifecycle. | The data establishes controller semantics, not a conversion from the 8254 input clock to core elapsed ticks. The `1/4` descriptor ratio is retained as deterministic scheduling only. |
| 8259A selection and logical acknowledgement | Intel 8259A defines request, priority, cascade, ISR/IRR and vectored acknowledgement; IBM supplies dual-PIC topology. `pic.c` owns source aggregation, select and logical `get_interrupt`. | The PIC has no timing input that turns the project callback order into physical INTA timing. Arbitration retains `DMA -> PIT -> PIC`; physical INTA remains phase work. |
| MC146818 event and IRQ8 | Motorola defines calendar/event flags, register-C acknowledgement and IRQ behavior; IBM supplies ports `70h/71h`, IRQ8 and NMI-mask wiring. `rtc.c` owns events/IRQ8 and `machine.c` keeps index-bit 7 as a mask-only adapter. T350 S3 proves the controller lifecycle. | The manual’s oscillator/event rules do not authorize the project `ticks_per_second=50000` or RTC `1/1` as a physical oscillator conversion. Readiness ordering remains logical: FDC/HDC refresh, RTC event, then next arbitration opportunity. |
| Reset and copied observation | `core_machine_cold_reset` resets RTC, PIC and PIT before clock domains/timeline; trace copies PIT/PIC/RTC ordering after state publication. Focused T346 timeline/arbitration/readiness tests retain the exact callback order. | Resettable order is proven; elapsed ticks and trace sequence are not a measurement of device propagation or controller-cycle duration. |

## Existing proof coverage

T350 already owns the retained controller semantics: `M5:T350:S2:PIT-IRQ0:OK`
proves PIT output-to-IRQ0 lifecycle, while `M5:T350:S3:RTC-CMOS:OK` proves
MC146818 event/IRQ8/register-C and mask/index separation. T346’s
`M5:T346:S3:ARBITRATION:OK` and `M5:T346:S4:RTC-STORAGE-READINESS:OK` prove
the deterministic callback ordering. These markers are not reclassified as a
physical-clock or service-duration proof.

The S2 sweep reviewed `core_machine_clock_plan`, all PIT/PIC/RTC initialize,
advance, refresh, acknowledgement, reset/finalize routes, the three timeline
callbacks, descriptor composition and the focused clocks/timeline/controller
tests. No second state, callback, timing publisher or copied-consumer breach
was found.

## S3 receiver

S3 receives dual-8237A/FDC only: request/terminal/result/reset routes and a
controller-domain admission decision. It must use the same rule: a controller
manual’s duration becomes runtime timing only after its clock and its mapping
to an existing project domain are source-labelled. uPD765 SRT/HLT/HUT values
therefore remain non-admitted until that condition is met. PIT/PIC/RTC physical
oscillator phase, IRQ propagation and INTA cycles transfer to selected-profile
phase refinement after T370 completes.

**This S makes no 5170 Model-339 L3 claim.**
