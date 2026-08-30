# T503 S8: RTC/CMOS To PIC, NMI, And Firmware Route Audit

## Authority And Scope

The normative device source is `MC146818A_Real_Time_Clock_Plus_RAM.pdf`,
rendered and visually read at printed pages 8--16.  Its update-cycle page
defines alarm comparison (including `11xxxxxx` don't-care fields), AIE/AF/
IRQ behavior, and the 244-us pre-update plus 1.984-ms 32.768-kHz update
interval.  IBM's March 1984 5170 Technical Reference supplies the `70h/71h`,
IRQ8 and index-bit-7 NMI-mask board route.  No external code was imported.

86Box `nvr_at.c` independently compares all three alarm fields at update,
sets AF, gates IRQ from `(B & C)`, and clears Register C plus IRQ on read.
PCjs `chipset.js` follows the same periodic/alarm/update ordering; Bochs
`iodev/cmos.cc` uses separate one-second, UIP and periodic timers, compares
the same three alarm fields, and clears IRQ8 on Register-C read.  No local
MAME or QEMU source tree is available, so neither is claimed as corroboration.

## Single Route And Disposition

| Form | Core owner and downstream route | Disposition |
| --- | --- | --- |
| Periodic / update / alarm state | `rtc.c` retains calendar, divider, PF/UF/AF and Register C. | Manual L3 with a source-qualified RTC clock plan; an L2 plan retains L2 provenance. |
| IRQ8 acknowledgement | `rtc_refresh_irq` is the only publisher to the bound PIC source; selected Register-C read clears flags and that same source. | Manual L3 logical device behavior. |
| Port `70h` / `71h` and NMI mask | `machine_board.c` alone separates index bit 7 into the board NMI mask and sends low six bits to the RTC owner. | IBM-board L3 wiring; RTC does not own NMI. |
| HLT/deadline consumption | `machine_scheduler.c` asks the RTC owner for its earliest enabled IRQ and converts it through the existing copied RTC clock. | One existing Core time path; no polling or VM clock injection. |
| Reset / CMOS observation | RTC reset retains calendar/RAM/divider/SET form required by the admitted chip contract while clearing event delivery; firmware observes CMOS only through the port adapter. | Manual L3 selected reset semantics; battery/power persistence remains separately unselected. |

## Repair And Complete Sweep

`core_machine_rtc_ticks_until_irq()` previously considered only PIE and UIE.
An AIE-only guest that halted before its selected alarm therefore had a real
future IRQ8 transition, but published no scheduler deadline.  The sole repair
is an owner-local alarm search over the maximum 24-hour alarm period using the
same `rtc_increment_second()` and `rtc_alarm_matches()` predicates that
produce AF.  It returns the exact remaining RTC ticks to the next match.
There is no second calendar, timer, RTC state, board callback or compatibility
path.

The owner-local regression programs a two-second AIE-only alarm, proves the
eight-tick deadline at its four-tick fixture rate, advances exactly that amount,
and observes AF/IRQF and the bound IRQ8 source.  Existing RTC/CMOS proofs
retain periodic, update, C-read acknowledgement, NMI-mask separation, reset,
NVRAM, divider and SQW coverage.

## Verification

- Rebuilt `core-machine-rtc-cmos-s3-smoke` after the source change and ran the
  RTC/clock focused cohort: 4/4 passed.
- Complete repository-only unit suite: 312/312 passed in 16.31 seconds.

The current repair is `+32/-1` production lines and `+24/-0` test lines.  It
removes no route because it completes an existing RTC deadline query rather
than adding a parallel mechanism.
