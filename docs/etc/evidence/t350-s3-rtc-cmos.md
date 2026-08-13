# T350 S3: MC146818 CMOS, Event, And IRQ8 Reconciliation

## Contract And Owner

The retained `core_machine_rtc` is the only owner of MC146818 calendar state,
register images, event flags, IRQF, and its bound IRQ8 source. `machine.c`
only adapts ports 70h/71h: bit 7 of an index-port write changes the core NMI
mask, while the low seven bits select the RTC register. The adapter does not
own calendar state or reinterpret RTC events as NMI.

The primary contract is the Motorola MC146818A data sheet and the IBM PC/AT
Technical Reference. The former defines Register C as the event/IRQ
acknowledgement image and permits C0h through FFh in each alarm field as a
don't-care value. The latter supplies the PC/AT CMOS index/data and NMI-mask
wiring. Research informed requirements only; no reference source was imported.

## Reconciled Mechanism

| Behavior | Owner and result | Focused proof |
| --- | --- | --- |
| Calendar, BCD/binary, 12/24-hour, SET, rollover | `rtc.c` calendar encode/decode and advancement | S3 holds a BCD 12 PM value under SET, then proves second-to-minute rollover; it also proves binary 12-hour readback. |
| Periodic, update, alarm flags | `rtc.c` advancement writes PF, UF, and AF | S3 programs all three alarm fields to C0h, advances one second, and observes PF/AF/UF without IRQF while enables are clear. |
| Alarm don't-care match | owner-local alarm-field predicate | C0h fields now match their corresponding calendar fields without changing date/time representation. |
| IRQF and IRQ8 lifecycle | one `raise_if_enabled` synchronization boundary | enabling PIE/AIE/UIE after retained flags asserts IRQF/IRQ8 immediately; disabling all enables clears IRQF/releases IRQ8 while raw event flags remain until Register C read. |
| Register C acknowledgement | RTC selected-register read | S3 consumes slave vector 70h, reads C with IRQF/PF/AF/UF, proves source release, and completes both PIC EOIs. |
| 70h/71h mask/index separation | PC/AT adapter in `machine.c` | S3 writes 94h, reads low-seven-bit register 14h through 71h, and proves the NMI mask is set; a later 14h index write clears only the mask. |
| Reset/finalize | RTC owner | S3 proves NVRAM survives reset while calendar and source state reset, and proves finalize releases an asserted IRQ8 source. |
| Deterministic visibility | T346 readiness owner | T346 retains the sole readiness schedule: RTC advancement follows PIT/PIC arbitration and a newly asserted RTC source is eligible on the next arbitration tick. S3 does not create a second schedule. |

## Repairs And Sweep

Two reproduced RTC-local defects were repaired:

1. Exact alarm equality ignored the MC146818A don't-care field convention.
   `core_machine_rtc_alarm_field_matches` now treats a field whose top two
   bits are both set as a match.
2. Writing Register B left a prior PF/AF/UF condition disconnected from the
   current enable image. The existing owner-local synchronization routine now
   both asserts and deasserts IRQF/IRQ8, and every selected-register write
   invokes it after publication.

The similar-issue sweep reviewed all RTC selected and direct register
readers/writers, event-flag transitions, IRQ source operations, CMOS adapter
routes, reset/finalize calls, and T346 readiness scheduling. No duplicate RTC
event or IRQ8 owner was found. The intentionally retained boundary is physical
MC146818 oscillator/UIP electrical timing; this task implements deterministic
guest ticks, not a host clock or clone-specific waveform.

## Verification

`tests/machine/core_machine_rtc_cmos_s3_smoke.c` emits
`M5:T350:S3:RTC-CMOS:OK`. It supplements retained RTC, CMOS-authority,
timer/firmware, T346 timeline, and T349 PIC-lifecycle coverage. Its target is
registered as `current.core-machine-rtc-cmos-s3-smoke` and compiles under the
existing target-local strict GCC test policy.

Primary references:

- [Motorola MC146818A data sheet](https://www.ardent-tool.com/datasheets/Motorola_MC146818A.pdf)
- [IBM PC/AT Technical Reference](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf)
