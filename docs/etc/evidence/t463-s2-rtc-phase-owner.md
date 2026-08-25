# T463 S2 RTC Phase Owner Evidence

## Scope And Authority

This evidence consumes the complete S1 ledger, `RTC-R1`--`RTC-R4`,
`RTC-F1`--`RTC-F7` and `RTC-T1`--`RTC-T5`. Motorola MC146818A printed pages
8--16 are normative. IBM AT printed pages 1-45--1-48 are normative only for
the selected AT ports, IRQ8 and board CMOS map. The five named emulator trees
in the S1 checklist are corroboration only; no external source was imported.

## Retained Owner And Data Flow

`machine_scheduler.c` remains the only timeline consumer. Its copied RTC clock
domain supplies guest ticks to `rtc.c`; `rtc.c` alone owns calendar phase,
divider state, flags, SQW state and the PIC IRQ source. `machine_board.c`
copies `core_machine_rtc_timing_plan` exactly once at construction. A plan is
either direct selected-board `L3_SOURCE`, or explicitly labelled `L2_RATIO`.
There is no host-time input, mutable timing setter, duplicate calendar store or
century side path.

| Ledger batch | S2 disposition and proof |
| --- | --- |
| R1/R4 | The register file is 64 bytes; index selection is six-bit; 00h--0Dh use the sole calendar/register owner and 0x32 is ordinary NVRAM. `core-machine-rtc-cmos-s3-smoke` proves the 32h path. Time-read value during the manual's unavailable update interval remains an explicit L2 bus-result boundary; UIP itself is L3. |
| R2/F1/F5/F7 | Register A owns running divider selections, reset hold/release, periodic base/rate and calculated UIP. The L3 Model-339 plan supplies 8 lead ticks and 65 update ticks at 32,768 Hz; L2 plans retain their label. The focused smoke proves UIP, reset cancellation and half-second divider restart. |
| F2/F4 | Alarm equality including C0h don't-care, sticky AF/UF/PF and enabled OR to the existing PIC IRQ8 source remain in `rtc.c`. Register-C read clears the observed flags and IRQ source. Existing RTC and DMA/RTC authority smokes cover cascade delivery. CPU visibility remains board L2. |
| F3 | All manual divider bases and RS values drive sticky PF; SQWE drives one RTC-owned square-wave state observable through `core_machine_rtc_get_square_wave`. Board pin/electrical routing remains L2. |
| R3/F6/T3 | RESET preserves calendar, RAM, SET/divider selection while clearing interrupt enables/flags and SQW. VRT remains a selected power-policy L2 boundary; host persistence is not introduced. |
| T1/T2/T5 | Existing 70h/71h, NMI-bit and IRQ8 adapter remains one board owner. The copied timing plan plus `rtc_clock` declaration prevents an L3 claim without an explicit rational clock and L3 phase values. Default PC/AT is L2; Model 339 is L3. |

## Verification

- `ninja -C build/mingw-gcc-x64 -j1` for the RTC, plan and profile targets:
  passed.
- Focused CTest RTC cohort: 7/7 passed.
- `core-machine-plan-smoke`, `vm-default-pc-at-profile-smoke`,
  `vm-model-339-clock-contract-smoke` and `core-machine-rtc-cmos-s3-smoke`:
  passed.

## Similar-Issue Sweep

Every direct `core_machine_rtc_config` initializer was converted to designated
initialization, every RTC reset expectation was reviewed for the manual's
calendar retention rule, and every controller-timing-rule initializer received
the new RTC member. No generic century reference remains.
