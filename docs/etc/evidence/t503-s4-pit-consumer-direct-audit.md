# T503 S4: PIT-To-Consumer Direct Audit

`T503-S4-PIT-CONSUMER-ROUTE`

## Sources and method

The direct chip sources were visually reviewed: Intel 231306-001, *8253-5
Programmable Interval Timer* (November 1986), pp. 5--13, and Intel 231164-005,
*8254 Programmable Interval Timer* (September 1993), pp. 3--14.  The former
defines the six mode and GATE rules; the latter adds the 8254 read-back/status
forms.  Their text extraction was used only for search; the cited pages and
figures were rendered before classification.

The direct board sources were also rendered: IBM 5160 *Technical Reference*
(April 1983), System Unit pp. 1-4 and 1-20; IBM 5170 *Technical Reference*
(March 1984), System Board pp. 1-8--1-9; and the already-indexed original
Compaq DeskPro D3PE evidence in [T421 S1](t421-s1-d4-speaker-line.md).

Read-only external comparison examined local 86Box `src/pit.c` plus its XT
machine wiring, Bochs `iodev/pit.cc` and `pit82c54.cc`, and PCjs
`machines/pcx86/modules/v2/chipset.js`.  MAME and QEMU were not present in the
approved local reference tree, so no row relies on either.  PCjs documents
that it shortcuts part of XT refresh state on DMA-register reads; that shortcut
is rejected here because it is not the board signal route.

## Complete selected route ledger

| Route | Direct source fact | NXVM sole route and proof | External comparison and disposition |
| --- | --- | --- | --- |
| Counter semantics, ports, modes, loads, latches, read-back | Intel 8253/8254 cited pages define three counters, control/count forms, modes 0--5, GATE rules and 8254 status/read-back. | `pit.c` is the one three-counter owner; `core-machine-pit-waveform`, `pit-8253`, `pit-readback` and `pit-divider` cover the selected forms. | 86Box and Bochs have separate counter-local OUT/GATE state; PCjs models the same port/counter grouping. **Manual-L3.** |
| Channel 0 OUT to IRQ0 | IBM 5160 System Unit p. 1-4 attaches interrupt level 0 to channel 0. IBM 5170 System Board p. 1-8 says CLK OUT 0 goes to 8259A IRQ0. | `machine.c` binds shared PIT output 0 once through `shared_pit_irq0_source` to `core_machine_pic_timer_output`; scheduler order is PIT then PIC. `core-machine-pit-irq0` and `pit-waveform` prove edge, deassertion and reset release. | 86Box, Bochs and PCjs all bind timer 0 to IRQ0. **Manual-L3** for board route; Core callback ordering is the project deterministic scheduler contract. |
| Channel 1 XT refresh | IBM 5160 System Unit p. 1-4 says channel 1 requests a DMA refresh transfer. | `core_machine_configure_dma` owns channel-0 refresh binding; PIT output 1 is its only producer. The added `core-machine-dma-rtc-authority` regression proves a mode-2 low pulse asserts DMA DRQ0 and the following high output releases it. | 86Box connects XT PIT1 rising output to DMA channel 0. PCjs documents this physical route but deliberately shortcuts its register-visible effect; NXVM retains the route. **Manual-L3** logical board relation; electrical refresh width/bus phases remain outside this route. |
| Channel 1 IBM AT / Model 339 refresh | IBM 5170 System Board pp. 1-8--1-9: GATE1 tied on, CLKIN1 1.190 MHz, CLKOUT1 requests refresh; channel 1 is rate-generator programmed for 15 microseconds. | The PC/AT board owner programs counter 1 once per cold reset (divider 18) and publishes its output at port 61h bit 4. The existing DMA binding remains one Core binding. | 86Box has a distinct AT refresh callback; Bochs synthesizes port-61 refresh visibility from its timer domain. **Manual-L3** for source/frequency/route; Core's source-tick conversion preserves its declared provenance. |
| Channel 2 XT speaker | IBM 5160 System Unit p. 1-20 diagram specifies PPI 8255 port 61h bit 0 to GATE2, bit 1 ANDed with timer output, and channel 2 output drives speaker. | XT PPI writes have one `core_machine_board_set_xt_ppi_speaker` route: it sets GATE2 and the Core speaker owner computes the direct-or-timer output. `core-machine-xt-ppi-keyboard` covers controls and reset. | Bochs and PCjs independently model port-61 gate/data and timer-2 output. **Manual-L3** logical line; host audio/electrical sound is not claimed. |
| Channel 2 AT / Model 40 speaker | IBM 5170 System Board p. 1-9 assigns GATE2 to port 61h bit 0 and CLKOUT2 to speaker; D3PE defines the same selected DeskPro port-61 gate/data line. | Planar-PC/AT and D4 port-B providers share `core_machine_speaker_refresh`; neither owns a second frame or timer state. `core-machine-d4-platform-s4` proves port, timer, reset and D4 auxiliary-PIT adjacent behavior. | Bochs and PCjs model the same port-61 gate/data/TIMER2 decomposition. **Manual-L3** logical line. |
| Auxiliary Model-40 PIT fail-safe | D3PE is the direct source already audited in T421; it assigns this selected auxiliary counter to the board fail-safe input. | The auxiliary `t_pit` owns its counter; only `core_machine_d4_platform_failsafe_output` consumes the selected output. `core-machine-d4-platform-s4` proves assertion, NMI and reset. | PCjs names its DeskPro second PIT but differs in broader board policy. **Manual-L3** for the selected logical input; physical fail-safe duration remains unclaimed. |
| Reset, rebinding and HLT deadline | Intel leaves power-up state unspecified; board initialization establishes the programmed channel-1 forms. | `core_machine_reset` resets PIT(s), then the board reapplies its documented consumers/programming. `core_machine_pit_ticks_until_output` supplies the already-composed Core deadline without exposing PIT state to VM. | Bochs reschedules its PIT from next event; 86Box uses its own event layer. **L2** only for emulator reset choice and project scheduler visibility; no host-time claim. |

## Repair and full sweep

`core_machine_pit_set_output` previously also wrote `flagGate[id] = true`.
That conflated a consumer registration with the independent chip input and
could overwrite a board-selected GATE during a legitimate rebind.  The sole
PIT owner now only records the consumer; reset remains the one place that
sets its deterministic initial GATE values.  `core-machine-pit-waveform`
regresses this exact separation.

The downstream sweep found no second IRQ0, refresh or speaker state owner.
The only missing regression was the actual PIT1-low-to-DMA0-DRQ transition;
it is now covered by `core-machine-dma-rtc-authority`.  The selected D4
refresh, speaker and auxiliary fail-safe routes already had focused ownership,
port and reset proof; no profile, firmware, VM or polling workaround was
added.

Tracked source/test result: `src/core/machine/pit.c` removes one production
line; the two owner-local regressions add 37 test lines, for a tracked net of
`+36` lines.  The retained production path is still one `t_pit` state owner
with one callback slot per OUT pin.  The added tests replace no production
path and make the previously implicit separation observable.

Focused command:

```text
ctest --test-dir build/mingw-gcc-x64 -R "core-machine-(pit-waveform|pit-irq0|pit-8253|pit-readback|pit-divider|dma-rtc-authority|d4-platform-s4|xt-ppi-keyboard)" -j 8 --output-on-failure
```

Result: 8/8 passed.  Complete repository-only unit replay:

```text
ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure
```

Result: 312/312 passed in 13.83 seconds real time.
