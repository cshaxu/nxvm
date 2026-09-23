# T349 S4: PC/AT PIC Request Lifecycle and Composition

## Lifecycle ownership

The PIC owns controller-visible `IRR`, `ISR`, priority state, and per-line
assertion counts. A device-owned `core_machine_pic_irq_source` owns exactly
one physical producer assertion: duplicate asserts from that source are
idempotent, while independent sources on the same line are counted. The
controller does not manufacture device state during reset or finalization.

| Producer | Bound line | Assert/deassert owner | Reset/finalize disposition |
| --- | --- | --- | --- |
| PIT channel 0 | IRQ0 | PIT output transition callback | Reset and finalize now notify a low output before clearing the PIT latch, releasing the bound source. |
| KBC keyboard/AUX | IRQ1/IRQ12 | FIFO/output readiness | Reset and finalize deassert both source objects. |
| FDC | IRQ6 | Result completion | Reset and finalize deassert IRQ6 and cancel DMA request. |
| RTC | IRQ8 | Enabled RTC status-C event | Reset and finalize deassert IRQ8. |
| HDC | IRQ14 | ATA command/data service | Reset and finalize clear/deassert IRQ14. |

`core_machine_cold_reset` resets KBC, DMA, RTC, FDC, and HDC before the PIC,
then resets PIT. PIT reset performs its source release after the PIC clears
controller state, so the source and controller agree that no IRQ0 level is
live. The deterministic timeline remains `DMA -> PIT -> PIC`; `refresh`
re-materializes only level-triggered assertions and slave cascade eligibility.

## Repaired composition boundary

The prior PIT reset cleared `flagOutput[0]` without notifying its bound sink.
If IRQ0 was high, `core_machine_pic_irq_source.asserted` could remain true
after controller reset; a later PIT rising edge would then be suppressed by
the source's idempotence guard. S4 releases live PIT output before clearing
the latch, and applies the same explicit finalization rule to RTC and HDC.
This is a producer-lifecycle repair only: controller selection, priority,
poll, special mask, SFNM, CPU delivery, and timeline order are unchanged.

## Focused proof and deterministic-L3 boundary

`tests/machine/core_machine_pic_lifecycle_s4_smoke.c` emits
`M5:T349:S4:PIC-LIFECYCLE:OK` and proves:

- edge IRQ1 retains one request through source deassertion and EOI, while an
  empty acknowledgement publishes neither master nor slave ISR state;
- two independent level sources on master IRQ5 and slave IRQ14 retain their
  respective request until the final source deasserts, reasserting after each
  EOI and propagating the slave request through master IR2;
- invalid IRQ2 and IRQ16 source bindings leave an unbound source unchanged;
- a live PIT IRQ0 is released across PIC-plus-PIT reset, then a fresh PIT
  rising edge produces one fresh source assertion and IRR request; finalize
  releases it again.

The deterministic-L3 model performs PIC selection and acknowledgement as one
ordered machine transaction. It therefore has no physical INTA sampling gap
in which a deasserted edge becomes a synthetic IRQ7/IRQ15. The empty-get
contract returns zero and preserves ISR state. Authentic spurious IRQ7/IRQ15
is transferred to the queued [L3 bus-timing convergence]
(../../proposals/m5-l3-bus-timing-convergence.md) task, which alone may add a
physical-INTA transaction boundary; S4 must not invent a fake vector.

## Verification

The owner target is registered once in `PROJECT_CURRENT_SMOKE_TARGETS`; the
T345 exact pure owner-test count is 125, with three mixed targets unchanged.
Focused lifecycle, retained T216/T349 S2/S3, PIT/KBC/RTC/FDC/HDC, CPU-delivery,
T346/T347/T348 regressions, documentation governance, diff check, and the
complete current gate are required before acceptance.
