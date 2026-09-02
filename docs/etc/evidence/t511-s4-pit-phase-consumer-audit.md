# T511 S4 PIT Phase-Consumer Audit

`M5:T511:S4:PIT-PHASE-CONSUMER:NO-HIT`

## Sources

Intel *8254 Programmable Interval Timer*, order 231164-005 (September 1993),
owner-managed copy
`../../../../nxvm-assets/manuals/pit/intel-8254-interval-timer-sep1993.pdf`,
defines three independent counters, control/count-to-CE loading, GATE-triggered
and periodic output transitions, and the read-back latches.  The XT 8253
personality is separately bounded by Intel 231306-001.  The selected output
state rules are Manual-L3; board wiring and clock conversion remain their
already recorded board contracts.

86Box and Bochs retain counter-local control/count/GATE/OUT records and make
machine IRQ/speaker adapters distinct consumers.  They corroborate the
separation only as Other-L2 sources; no third-party code was copied.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Control/count, reload and GATE | `pit.c` retains every counter's pending load, restart, trigger, phase and OUT state. | A new clock/GATE transition is consumed only at its documented owner edge. |
| Counter 0 OUT to IRQ0 | The one Core binding calls the PIC source operation on every actual OUT level change; the PIC then independently selects the IRQ. | Every periodic rising edge is fresh; no IRQ consumer reuses an earlier edge. |
| Counter 1 OUT to refresh/DMA | The one board callback translates each selected refresh output level to the DMA0 request state. | A new waveform level is observed directly; it is not a PIO data phase. |
| Counter 2 OUT and port-B gate | The board speaker owner samples the same PIT2 output after its gate update or callback. | One Core-owned state path; no VM/presentation mirror. |
| Reset/reprogram | PIT reset first withdraws live outputs; control/count changes reset local counter state before later output transitions. | Prior consumers cannot retain an asserted line across reset/reprogram. |
| 8253 versus 8254 | The immutable PIT personality admits 8254 Read-Back only where selected; XT 8253 ignores it. | No parallel timer or compatibility consumer. |

## Executed Proof And Conclusion

The focused Debug tests `core-machine-pit-readback-smoke`,
`core-machine-pit-waveform-smoke`, `core-machine-pit-irq0-s2-smoke`,
`core-machine-dma-rtc-authority-smoke`, `core-machine-d4-platform-s4-smoke`
and `core-machine-pit-8253-smoke` passed.  Together they cover repeated
periodic OUT transitions, counter-0 IRQ0 reassertion after EOI, counter-1
refresh/DMA publication, counter-2 speaker gating, reset withdrawal and
personality separation.

No ATA-style stale-ready consumer exists: an OUT change is an owner-published
level transition, not permission to transfer a later data unit.  Existing
counter-local state and explicit callbacks are the minimal correct design;
no production change is warranted.
