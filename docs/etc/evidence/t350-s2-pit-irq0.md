# T350 S2: 8254 Retained Counter and IRQ0 Signal Path

## Mechanism audit

The retained `pit.c` mechanism has one owner for all three counter control
words, count/status latches, binary/BCD reload/count encoding, GATE state,
OUT state, and elapsed-tick progression. Ports 40h--43h only read or write
that owner. `machine.c` binds channel 0 once to a T349 PIC source; its
arbitration callback advances DMA, then PIT, then PIC. The later RTC readiness
callback cannot make an RTC IRQ visible until the following arbitration tick.

| Retained 8254 behavior | Owner / implementation route | Proof |
| --- | --- | --- |
| Counter selection, LSB/MSB/LSB+MSB programming, null count | control port 43h and counter ports 40h--42h | Readback smoke plus S2 counter-form proof. |
| Modes 0--5, aliases 6/7, GATE behavior, OUT transitions | counter-local mode tick and GATE routines | Waveform smoke and S2 mode-0/mode-2 gate proof. |
| Binary zero=65536 and packed-BCD zero=10000/reload/count | decode/encode and synchronization helpers | Existing waveform/readback proof plus S2 BCD count/status proof. |
| Count/status latch and read-back | one latch/status owner and port reader | Existing readback smoke, retained at S2 acceptance. |
| Channel-0 rising OUT to edge IRQ0 | bound output callback -> one PIC source -> PIC acknowledgement | New S2 owner proof. |
| Reset/finalize source release | PIT releases live output before clearing/resetting local state and at finalize | T349 S4 lifecycle proof and S2 reset proof. |

No new production defect was reproduced. The T349 S4 PIT reset/finalize repair
is the accepted lifecycle prerequisite: without it, an already-high channel-0
output could leave its bound source asserted across a controller reset and hide
the next rising edge.

## S2 owner proof

`tests/machine/core_machine_pit_irq0_s2_smoke.c` emits
`M5:T350:S2:PIT-IRQ0:OK` and proves:

- mode 2 count-three produces a low strobe without an IRQ source assertion,
  then exactly one rising output assertion; the master PIC returns IRQ0 vector
  08h, EOI clears ISR, and the following low output releases the source;
- channel-1 LSB/MSB null-count publication, latched count order, packed-BCD
  count progress, and status image are independent of the IRQ0 sink;
- mode-1 gate low holds the one-shot, a rising gate starts it, and terminal
  output asserts the bound IRQ0 source;
- PIC reset followed by PIT reset clears the source/count/output state, so a
  later reprogrammed rising edge is eligible again.

Retained waveform, readback, divider, T346 timeline/arbitration, T349
lifecycle, and CPU hardware-delivery regressions cover the complementary
counter shapes, clock conversion, controller acknowledgement, and CPU frame
boundary. The direct source sweep found no duplicate counter, callback, or
IRQ0 state owner.

## Explicit boundary

Port 61h, PPI/speaker wiring, channel-2 host-audio presentation, oscillator
phase, and pin-level waveform duration are not part of the retained channel-0
IRQ0 mechanism. The S1 ledger transfers those rows to their corpus-gated
receiver; S2 makes no implicit PPI/speaker claim. Physical INTA behavior stays
with the queued L3 bus-timing task.
