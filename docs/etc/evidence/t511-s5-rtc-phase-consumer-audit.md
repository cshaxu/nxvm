# T511 S5 RTC Phase-Consumer Audit

`M5:T511:S5:RTC-PHASE-CONSUMER:NO-HIT`

## Sources

Motorola *MC146818A Real-Time Clock Plus RAM*, owner-managed copy
`assets/manuals/controllers/motorola/MC146818A_Real_Time_Clock_Plus_RAM.pdf`,
defines the calendar/update, periodic and alarm flag sources, Register-C
acknowledgement and IRQ output.  IBM AT material supplies the separate
70h/71h index/data, NMI-mask and IRQ8 board route.  Existing direct review
records source-backed update/reset values and the one Core owner.

86Box, Bochs and PCjs independently retain separate update, periodic, alarm
and IRQ state.  They corroborate the state separation as Other-L2 only; no
external implementation was copied or used as a timing authority.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Calendar/update, periodic and alarm edges | `core_machine_rtc_advance` owns calendar phase and independently sets UF, PF and AF. | Every new edge is a new RTC event; no past IRQ authorizes a later event. |
| IRQF/IRQ8 publication | `rtc_refresh_irq` derives IRQF solely from the current flags and enables, then operates the one PIC IRQ8 source. | The PIC is a downstream consumer, not an RTC state mirror. |
| Register-C acknowledgement | The RTC read path returns C, clears its flags and withdraws IRQ8 atomically in the same owner. | A later advance must and does publish a fresh event. |
| Alarm deadline | `core_machine_rtc_ticks_until_irq` queries the same calendar/periodic state consumed by `rtc.c`; the scheduler only converts the copied clock. | No scheduler calendar or stale deadline cache exists. |
| 70h/71h and NMI mask | The board adapter owns index bit 7; RTC owns the six-bit register selection/data. | NMI masking never acknowledges or fabricates an RTC event. |
| Reset | RTC reset preserves the running calendar phase while clearing delivery state and the sole IRQ source. | A pre-reset flag cannot leak; the next RTC event is new. |

## Executed Proof And Conclusion

The focused Debug cohort passed: `core-machine-rtc-cmos-s3-smoke`,
`core-machine-rtc-smoke`, `core-machine-dma-rtc-authority-smoke`,
`core-machine-scheduler-smoke`, `vm-cmos-rtc-port-smoke` and
`vm-model-339-clock-contract-smoke`.  It covers repeated events after a
Register-C read, enabled IRQ8 delivery, update/alarm deadlines, reset phase
retention, and the independent NMI-mask adapter.

No ATA-style stale observation exists.  The existing RTC-to-PIC path is a
single event source with fresh re-publication after each acknowledgement; no
production change is warranted.
