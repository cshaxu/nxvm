# T450 S9 MC146818A RTC/CMOS Original Function And Timing Checklist

> S19 direct PDF check: scan without usable text layer (122 extracted bytes);
> rendered pages govern every value. Chip facts are sufficient; crystal/IRQ8
> and persistence remain L2. No external emulator is primary.

## Scope And Source Key

This is the complete RTC/CMOS source universe transferred unchanged to T450
S10. `RTC` means Motorola *MC146818A Real-Time Clock Plus RAM*, ADI1026R3;
page references are the printed manual pages. `AT` means IBM *Personal
Computer AT Technical Reference*, 1502243 (Mar. 1984), system-unit pages.
The list distinguishes the RTC's own divider/update relations from electrical
oscillator/bus limits and from selected board IRQ/CMOS ownership.

## Register, Calendar And Storage Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| RTC-R1 | RTC pp. 1, 8--10 | The 64 locations comprise 10 time/calendar/alarm bytes, registers A--D and 50 bytes of general-purpose battery-backed RAM. Time/calendar/alarm data are binary or BCD under one selected data mode; alarm bytes support documented don't-care codes. | Registers C/D are read-only; Register-A UIP and seconds high bit are read-only. RESET does not alter clock/calendar/RAM. | The 10 time bytes are unavailable during update; general RAM remains available. | L3. |
| RTC-R2 | RTC pp. 13--15 | Register A owns UIP, divider selection and periodic/SQW rate selection. Register B owns SET, PIE/AIE/UIE, SQWE, data mode, 24/12 and daylight-saving choice. | Divider-reset selections hold divider; SET aborts/prevents update; RESET clears listed interrupt/SQW enables but not SET/divider selection. | Divider selections choose one of 4.194304 MHz, 1.048576 MHz or 32.768 kHz time bases and a selected divider tap. | L3 chip semantics; selected board clock is L2. |
| RTC-R3 | RTC pp. 10--11, 15--16 | Register C reports IRQF/PF/AF/UF; it is read-only and reading it clears then-active flags/IRQF while holding new events. Register D reports VRT and zeroes in unused bits; reading it sets VRT when power sense permits. | RESET clears pending flags and leaves IRQ high-impedance; low PS clears VRT. | IRQF is the enabled-source OR; Register-C read releases the asserted IRQ condition. | L3. |
| RTC-R4 | RTC pp. 8--10, 14--16; AT pp. 1-45--1-48 | Address/data bus access reaches the indexed RAM/register locations; 24/12, BCD/binary and calendar rollover determine the chip time representation. IBM AT assigns general-RAM byte 32h as its board CMOS century convention; it is not an MC146818 calendar register. | Software must SET before initialization and clear SET to resume update; reset does not establish a time. | Bus setup/hold and oscillator start/reset delays are electrical. | L3 function; L4 electrical limits. |

## Update, Alarm, Periodic, IRQ And Power Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| RTC-F1 | RTC pp. 1, 10--11, 13--15 | With a valid running divider and SET clear, update runs once per second, increments the chip calendar through year rollover and compares the three alarm fields. IBM's century convention remains board-map state. | SET aborts an in-progress update and holds further updates; divider reset or absent oscillator prevents the update. | UIP precedes update; update is 248 microseconds for 4.194304/1.048576 MHz or 1,984 microseconds for 32.768 kHz. | L3 causal/update formula; selected Core phase is L2. |
| RTC-F2 | RTC pp. 9--10, 13--16 | Alarm can occur once per second through once per day according to time equality and don't-care bytes; AF records the event independently of AIE. | RESET/Register-C read clears AF; AIE controls only IRQ assertion. | Alarm compare belongs to each update cycle. | L3. |
| RTC-F3 | RTC pp. 1, 11--15 | Periodic interrupt selects the Table-5 rate (500 ms to 30.517 microseconds); PF records selected-divider edges independently of PIE. SQW shares the rate selector but SQWE independently controls the output. | RESET clears PIE/PF/SQWE; Register-C read clears PF. | Rate is a divider formula from selected base plus RS bits, not an arbitrary host cadence. | L3 formula; selected base/phase L2. |
| RTC-F4 | RTC pp. 10--11, 15--16 | IRQ is active low whenever one enabled PF/AF/UF source is present; IRQF records that combined condition. Multiple flag sources may coexist and Register-C read acknowledges all then-active flags. | RESET or Register-C read releases pending interrupt state; sources retain their documented enable/flag distinction. | IRQ output is an asserted causal relation, not a documented CPU-delivery duration. | L3 logical IRQ; L2 board/CPU visibility. |
| RTC-F5 | RTC pp. 7--10, 13--16 | UIP provides a valid-data window; update-ended interrupt provides a post-update notification. Static CMOS RAM is usable through update and can retain configuration. | PS/VRT reports battery validity; RESET does not invalidate RAM/time. | UIP low guarantees at least the documented pre-update interval; the update interval blocks time/calendar/alarm bus access. | L3 update/data integrity; electrical power transition excluded. |
| RTC-F6 | RTC pp. 5--8, 14--16 | RESET clears interrupt enables/flags and SQWE, stops a pending update cycle and tri-states IRQ; it does not reset time/calendar/RAM, SET or divider-selection state. | RESET release has data-sheet electrical delay only. | The RTC has no general host-reset time initialization rule. | L3 reset consequences; L4 pulse/release characteristics. |
| RTC-F7 | RTC pp. 1, 5--8, 12--15 | The oscillator/divider has 22 stages, selectable external/crystal bases and a one-hertz update output; divider control can reset/precisely start the chain. | Divider reset provides initialization hold; releasing it gives first update at the documented half-second relation. | Oscillator startup, crystal characteristics and bus timing tables are electrical unless an AT board clock selects a formula. | L3 divider state; L2 selected board time base; L4 electrical values. |

## Selected IBM AT Binding And Timing Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Disposition |
| --- | --- | --- | --- | --- |
| RTC-T1 | AT pp. 1-10, 1-24, 1-28, 1-45--1-48 | IBM AT selects MC146818 with 64 CMOS bytes; address port 0070h and data port 0071h supply the indexed RTC/CMOS interface, and address-port bit 7 carries the NMI-mask input. The AT CMOS map gives RTC bytes at 00h--0Dh and board configuration/checksum bytes thereafter, including its board-defined 32h century byte. | CMOS checksum/configuration belongs to board/firmware policy, not to another RTC instance. | L3 board topology and indexed-port identity. |
| RTC-T2 | AT pp. 1-10, 1-45--1-47 | The AT RTC uses a 32.768 kHz time base and outputs its interrupt as IRQ8 through the secondary PIC. The address-port high NMI-mask bit is a board input distinct from RTC data index. | PIC acknowledgement and NMI mask state remain their respective owners. | L3 wiring; selected RTC-to-PIC/CPU visibility phase is L2. |
| RTC-T3 | AT pp. 1-45--1-48; RTC pp. 8--10 | Battery-backed CMOS retains configuration and RTC contents; VRT/power-loss handling determines whether firmware trusts it. The AT's defined CMOS address map and checksum are board facts, not generic MC146818 register semantics. | Battery loss/VRT transfers to board/firmware recovery policy. | L3 selected storage boundary; host persistence lifecycle is L2. |
| RTC-T4 | RTC pp. 18--21 | Oscillator, RESET, IRQ-release and bus AC tables provide physical voltage, crystal and nanosecond limits. | No Core timing unit follows from these tables. | L4 excluded. |
| RTC-T5 | T433 S6/S7; T449 | Core's sole time, reset, arbitration and observation owners must receive a selected RTC clock/divider/update/IRQ contract; RTC must not publish a second scheduler or own PIC/CMOS policy. | No implementation conclusion is made here. | L2 receiver: queued Core RTC CMOS phase contract. |

## T463 S1 Primary And Reference Cross-Validation

The Motorola PDF was rendered page-by-page because its text layer is unusable.
The relevant chip pages are printed pp. 8--16 and 18--21; IBM AT pp. 1-45--1-48
were extracted and visually checked for the selected board map and ports. The
manuals remain the only normative sources. The following independently read
implementations are corroboration only: 86Box `src/nvr_at.c` at
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`; MAME
`src/devices/machine/mc146818.cpp` at `b76b3922aa6eb49fe35a85e7689bd1d709bbcaee`;
PCjs `machines/pcx86/modules/v2/chipset.js` at
`c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`; Bochs 2.6-compatible
`iodev/cmos.cc`; and QEMU `hw/rtc/mc146818rtc.c` at
`a02e7fa9e6997a514e41dd47a914dbddfe5aa1bf`.

`Other L3` means the named implementation independently represents the same
manual causal relation. It never upgrades an unsourced board choice. PCjs is
explicitly excluded from divider/update-phase corroboration: it fixes 1024 Hz,
sets UIP atomically for BIOS compatibility, and lacks alarm don't-care handling.
QEMU's host/virtual-clock selection, interrupt coalescing and persistence policy
are likewise product-specific and are not candidate Core requirements.

| ID | Manual result | Reference-only corroboration and boundary | Final authority tier |
| --- | --- | --- | --- |
| RTC-R1 | The 64-byte map, read-only A--D fields, BCD/binary forms and update access restriction are Manual L3. | 86Box, MAME, Bochs and QEMU model the map/form restrictions; PCjs confirms the selected AT map but is not a full chip model. | Manual L3; Other L3; update-window implementation remains a current gap. |
| RTC-R2 | Register-A divider/rate and Register-B control semantics are Manual L3. | 86Box, MAME, Bochs and QEMU consume divider/rate/control state; PCjs fixes its rate and is not corroboration for the full relation. | Manual L3; Other L3; selected board clock remains fallback to L2. |
| RTC-R3 | Register-C flag/IRQF relation, read acknowledgement and Register-D VRT semantics are Manual L3. | 86Box, MAME, Bochs and QEMU independently clear C on read and represent VRT; their host-power handling is not normative. | Manual L3; Other L3; board power policy remains fallback to L2. |
| RTC-R4 | The chip's indexed register representation is Manual L3; the AT 32h century byte is board, not chip, state. | MAME/86Box/Bochs/QEMU retain indexed access; PCjs corroborates the AT 32h map. No emulator converts that IBM convention into a chip fact. | Manual L3; Other L3; generic-Core century ownership is a current gap. |
| RTC-F1 | One-second update, SET/divider hold, UIP lead and 248/1,984-us update durations are Manual L3. | 86Box, MAME, Bochs and QEMU schedule a separate UIP/update phase; PCjs deliberately does not. | Manual L3; Other L3; selected Core phase remains fallback to L2. |
| RTC-F2 | Alarm equality, C0h don't-care and AF independent of AIE are Manual L3. | 86Box, MAME, Bochs and QEMU implement the comparison/flag relation; PCjs exact-only comparison is a known non-corroborating simplification. | Manual L3; Other L3. |
| RTC-F3 | Divider-derived PF/SQW selection and PF independent of PIE are Manual L3. | 86Box, MAME, Bochs and QEMU implement rate-driven periodic state and SQW; PCjs is fixed-rate and has no SQW path. | Manual L3; Other L3; selected base/phase remains fallback to L2. |
| RTC-F4 | Enabled PF/AF/UF OR to IRQF/active-low IRQ and Register-C acknowledgement are Manual L3. | 86Box, MAME, Bochs and QEMU retain this causal state; PCjs corroborates only its selected IRQ8 path. | Manual L3; Other L3; CPU-delivery phase remains fallback to L2. |
| RTC-F5 | UIP valid-data window, update-ended notification and static-RAM availability are Manual L3. | 86Box, MAME, Bochs and QEMU expose a non-atomic UIP/update phase; their persistence choices are outside the chip rule. | Manual L3; Other L3; power lifecycle remains fallback to L2. |
| RTC-F6 | RESET enable/flag/SQW consequences while retaining clock/calendar/RAM and SET/divider selection are Manual L3. | 86Box, MAME, Bochs and QEMU separately reset flags/enables without treating reset as an RTC time initialization contract. | Manual L3; Other L3; reset-pulse electrical values are L4 excluded. |
| RTC-F7 | 22-stage divider state, selected base and reset-release half-second relation are Manual L3. | 86Box, MAME, Bochs and QEMU all represent divider stop/restart or equivalent timer state; PCjs does not. | Manual L3; Other L3; selected board clock remains fallback to L2. |
| RTC-T1 | IBM AT's 70h/71h indexed-port identity and CMOS partition are Manual L3. | 86Box, PCjs, Bochs and QEMU provide an AT-style adapter; MAME's generic device does not claim an IBM port map. | Manual L3; Other L3. |
| RTC-T2 | IBM AT's 32.768-kHz selection, IRQ8 route and independent NMI-mask input are Manual L3. | 86Box, PCjs, Bochs and QEMU distinguish RTC IRQ routing from NMI/address selection; their scheduler units are product choices. | Manual L3; Other L3; visibility phase remains fallback to L2. |
| RTC-T3 | IBM's CMOS map and battery-backed storage boundary are Manual L3. | All five implementations retain some NVRAM/CMOS configuration mechanism, but host files, defaults and recovery policy differ. | Manual L3; Other L3 only for storage existence; persistence/recovery remains fallback to L2. |
| RTC-T4 | AC timing, oscillator and pin limits are physical tables, not Core ticks. | External implementations choose their own host/platform timing. | L4 excluded. |
| RTC-T5 | This is a project ownership boundary, not a chip or IBM timing formula. | Every emulator bundles a scheduler policy with its product; none supplies an NXVM Core deadline contract. | fallback to L2; receiver remains the queued RTC contract. |

## Completeness And S10 Transfer

The finite universe is `RTC-R1`--`RTC-R4`, `RTC-F1`--`RTC-F7`, and
`RTC-T1`--`RTC-T5`. It covers all registers and storage, calendar/alarm/update,
periodic/SQW/IRQ, reset/power/divider states, electrical exclusions, and the
selected AT port, NMI, IRQ8, 32.768 kHz and CMOS binding. T450 S10 must retain
every identifier and add only current-code/test disposition; each nonconforming
or unallocated row transfers once to the queued Core RTC CMOS phase contract.
