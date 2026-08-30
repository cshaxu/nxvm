# T507 S6: MC146818A RTC/CMOS Controller Re-Audit

## Research quality and boundary

The normative device source is Motorola *MC146818A Real-Time Clock Plus RAM*
in the owner-managed manual archive. It is a 21-page scanned PDF, so extraction
is not used as evidence. Rendered pages 8--11 were visually reviewed. They
state the 4.194304-MHz, 1.048576-MHz and 32.768-kHz time bases; SQW selection;
IRQ/Register-C acknowledgement; and, crucially, that RESET does not affect the
clock, calendar or RAM while it clears delivery flags/enables and SQWE.

The retained T503 review visually read the Motorola update-cycle pages and the
IBM 5170 technical reference's `70h`/`71h`, IRQ8 and NMI-mask board route.
Those materials remain the source for 244-us UIP lead, 1.984-ms update interval
and board wiring.

Available local references are non-normative corroboration only:

- 86Box `src/nvr_at.c` separates periodic, update, alarm and IRQ state; its
  comments and reset behavior likewise retain clock/calendar state rather than
  creating a second calendar.
- Bochs `iodev/cmos.cc` has distinct periodic, one-second and UIP callbacks,
  binds IRQ8, and clears Register C plus IRQ on read. Its host-time and CMOS
  image policy are not adopted.
- PCjs `chipset.js` retains RTC register state and event deadlines, but its
  host-cycle and presentation policy are not adopted.
- No local MAME or QEMU RTC source was available. No external code was copied
  or used as a timing authority.

## List 1 / List 2 reconciliation

| ID | Required device or board relation (List 1) | Sole current owner and direct receiver (List 2) | Disposition |
| --- | --- | --- | --- |
| R1 | `70h` selects one of 64 RTC/CMOS locations; `71h` reads/writes that selected location while bit 7 is a board NMI mask, not an RTC bit. | `rtc.c` owns the 64-byte register/calendar state; `machine_board.c` owns index/data adapter and NMI mask. | Manual/IBM-L3. |
| R2 | Register A selects a running divider/time base and rate; Register B selects SET, PIE/AIE/UIE, SQWE, BCD/binary and 12/24-hour forms. | `rtc_divider_{running,hz}()`, `rtc_periodic_hz()` and calendar encoding own the selected forms. | Manual-L3 where copied clock/phase inputs are qualified; otherwise L2 ratio. |
| R3 | Calendar, update, UIP, periodic PF, alarm AF and Register-C IRQF are one RTC state machine; C read acknowledges flags and IRQ. | `core_machine_rtc_advance()`, `rtc_refresh_irq()` and `rtc_read_register()` own those transitions and the one bound PIC IRQ8 source. | Manual-L3 logical behavior. |
| R4 | Alarm comparison accepts `11xxxxxx` don't-care fields and can create an enabled AIE-only IRQ deadline. | `rtc_alarm_matches()` and `rtc_ticks_until_alarm()` reuse the same RTC calendar state; the latter is the only alarm deadline helper. | Manual-L3. |
| R5 | The earliest enabled periodic, update or alarm interrupt is an RTC-local deadline converted by the existing copied RTC clock, not a scheduler-side calendar. | `core_machine_rtc_ticks_until_irq()` is the only query; `machine_scheduler.c` converts through `rtc_clock` and preserves the existing PIC order. | Manual-L3 where the timing plan qualifies; otherwise explicit L2. |
| R6 | IRQ8 wiring and Register-C acknowledgement remain distinct from the NMI mask and reach the cascaded PIC through the one bound source. | Construction binds `rtc->irq_source` to IRQ8; board index bit 7 changes only NMI masking. | IBM-board L3 route. |
| R7 | RESET preserves clock/calendar/RAM and divider phase, but clears delivery enables/flags, IRQ and SQWE. | `core_machine_rtc_reset()` now preserves `second_ticks` and periodic phase while clearing the documented delivery state. `core_machine_rtc_cmos_s3_smoke` proves RESET during UIP retains the update phase. | Manual-L3. |
| R8 | Battery/VRT transitions, host persistence, electrical bus windows and physical IRQ/SQW waveform timing are not selected Core behavior. | No host clock, persistence layer, battery model or pin waveform route exists. | Explicit L2/L1 boundary; no invented model. |

## Repair and simplicity review

The audit found one source-backed defect: `core_machine_rtc_reset()` cleared
`calendar.second_ticks`. That field is the RTC's sole clock/calendar phase, so
clearing it contradicted the manual's RESET rule and silently shifted the next
update deadline. The direct repair deletes that reset, leaving the sole RTC
state owner intact. The existing reset-focused test now proves a RESET applied
during UIP leaves UIP and the update phase live while still clearing delivery
state.

No calendar helper, scheduler path, host-time source, profile workaround or
second CMOS store was added. The retained flow is:

`Core clock ticks -> RTC sole calendar/divider state -> RTC IRQ8 source -> existing PIC route`.

## Verification

- Fresh rebuild outside the stalled sandbox completed after four identified
  idle Ninja processes were terminated.
- Focused RTC cohort: 9/9 passed (`scheduler`, RTC storage, planar-NMI,
  RTC/CMOS, RTC, DMA/RTC authority, VM CMOS port, Model-339 clock contract and
  plan smokes).
- Complete repository-only unit: 312/312 passed with `ctest -L unit -j 8` in
  14.34 seconds real time after the orphan build process was removed.
- Documentation governance: passed for `vm-0-5-0506`.

The current tracked delta is `+1/-1` production lines and `+1/-1` test lines:
one stale phase reset is removed and its regression expectation is corrected.
