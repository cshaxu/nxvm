# T374 S15: Model-339 Platform-Controller Functional Audit

## Scope And Authority

This audit covers only the selected Model-339 system-board controller group:
the cascaded 8259A PICs, 8254 PIT, paired 8237A DMA controllers and
MC146818-compatible RTC/CMOS path.  IBM's March 1986 *Personal Computer AT
Technical Reference* (document 6280099, released with Models 319 and 339) is
the machine/topology authority.  Its I/O map names the two DMA controllers,
master and slave PICs, timer, and real-time-clock/NMI range.  Its system-timer
section assigns PIT channel 0 to IRQ0, channel 1 to refresh-request generation,
and channel 2 to the speaker/PPI route.  The project deliberately has no
speaker/PPI owner; that unselected `61h` route remains TODO and is not made a
synthetic Model-339 implementation here.

The manual is consulted at
[the preserved March-1986 IBM PDF](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf).
No firmware, ROM, media, binary, source text or source-derived code was
imported.  The source asserts topology and observable functional state only;
PIT cadence, DMA arbitration/service, PIC delivery phase, RTC frequency and
all physical-board timing remain T375 work.

## Selected Functional Matrix

| Controller | Selected ports/state and core owner | Reset, route and consumer | Current focused proof | Gap / earliest receiver |
| --- | --- | --- | --- | --- |
| Cascaded 8259A PIC | `pic.{c,h}` owns master `20h/21h` and slave `A0h/A1h`, ICW/OCW state, IRR/ISR/IMR, polling, priority, EOI and bound IRQ sources. | `machine.c` resets both PICs after controller reset; PIT binds IRQ0 and RTC binds slave IRQ8. Generated firmware declares PIC POST, IRQ0/INT 08h and RTC delivery consumers. | `PIC-IRQ-LIFECYCLE`, `PIC-COMMAND-PRIORITY`, `PIC-OCW3`, `PIC-LIFECYCLE` all pass. | Functional register/lifecycle routes exist. IRQ selection/acknowledgement phase and board cascade timing transfer to T375. |
| 8254 PIT | `pit.{c,h}` owns `40h`--`43h`, three counter command/latch/read-back states, gates and OUT callbacks. | `machine.c` binds channel 0 OUT to PIC IRQ0; reset clears/rearms the PIT. The Model-339 firmware consumes IRQ0/INT 08h and timer services. | `PIT-READBACK`, `PIT-WAVEFORM`, and `PIT-IRQ0` pass. | Channel 1 has no retained refresh consumer and channel 2 has no selected PPI/speaker owner. Preserve that explicit support boundary; cadence and controller/board phase transfer to T375. |
| 8237A pair and pages | `dma.{c,h}` owns primary/secondary address/count/page/command/mask/request/status state, transfer modes and bound-provider tokens. The descriptor exposes the AT port leaves; `machine.c` binds FDC DMA2. | Cold reset resets latch and both controllers before PIC/PIT reset. FDC owns its DRQ assertion/deassertion via the binding, with its own IRQ6 path. Generated firmware declares DMA POST and FDC consumer routes. | `DMA-GRANT:PORT`, `DMA-MODES`, `DMA-CHANNEL`, `DMA-PORT-PAGE`, `DMA-REQUEST-CASCADE`, `DMA-TRANSACTION-LIFECYCLE`, `DMA-BINDING-TOKEN`, and `DMA-RTC-AUTHORITY` pass. | Selected functional FDC-DMA2 binding, cancellation and terminal lifecycle exist. Transfer duration, HOLD/READY, bus grant and refresh interaction to T375; do not infer them from the logical transfer route. |
| RTC/CMOS | `rtc.{c,h}` owns selected register, calendar/NVRAM, SET, binary/BCD, alarm/periodic/update flags and register-C clear route. `machine.c` owns `70h/71h`, NMI-mask delegation and IRQ8 configuration. | Reset preserves configured NVRAM defaults while resetting clock/status state; RTC binds slave IRQ8. Firmware consumes CMOS POST and reads profile-selected floppy/memory equipment bytes. | `RTC-CMOS`, `CMOS-RTC-PORT`, `DMA-RTC-AUTHORITY`, and Model-339 composition/topology proof are retained. | Selected register/reset/IRQ acknowledgement routes exist. RTC rate/cadence, IRQ phase and board/NMI electrical behavior transfer to T375; independently unselected I/O-check NMI remains excluded. |

## Full Route Sweep

The audit searched `src/core/machine/{pic,pit,dma,rtc,machine}.{c,h}`,
`src/vm/profile/default_profile/pc_at_profile.{c,h}`,
`src/vm/composition/session/`, `tests/machine`, `tests/core`, and
`CMakeLists.txt` for controller names, selected port leaves, reset, IRQ0,
IRQ8, DMA2, `70h/71h`, initialization and focused-target registration.

- All selected port ownership and route bindings lead to the one core-machine
  controller owner; profile descriptors select leaves/routes but do not acquire
  controller state.
- DMA2's FDC binding is the only selected DMA peripheral consumer.  ATA/HDC,
  AUX and unselected PPI/speaker routes are outside this Model-339 scope.
- The only board-relevant PIT channel without a project consumer is channel 1
  refresh generation.  The lack of an observable refresh/PPI owner is a
  support-boundary transfer, not evidence that emulated memory refresh or a
  speaker has been implemented.
- Reset order is explicit in `core_machine_cold_reset()`: DMA, configured RTC,
  FDC/HDC, PIC and PIT state reset before the three deterministic timeline
  callbacks are rearmed.  It is a logical reset order, not an IBM reset-phase
  measurement.

## Replay And Discovered Regression

The local MinGW build rebuilt the listed targets.  Direct replay passed for
the following markers without ROM or guest-media input:

```text
M5:T216:S1:PIC-IRQ-LIFECYCLE:OK
M5:T349:S2:PIC-COMMAND-PRIORITY:OK
M5:T349:S3:PIC-OCW3:OK
M5:T349:S4:PIC-LIFECYCLE:OK
M5:T191:S2:PIT-READBACK:OK
M5:T222:S1:PIT-WAVEFORM:OK
M5:T350:S2:PIT-IRQ0:OK
M5:T269:S1:DMA-GRANT:PORT:OK
M5:T269:S4:DMA-MODES:OK
M5:T230:S3:DMA-CHANNEL:OK
M5:T348:S2:DMA-PORT-PAGE:OK
M5:T348:S3:DMA-REQUEST-CASCADE:OK
M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK
M5:T300:S4:DMA-BINDING-TOKEN:OK
M5:T296:S3:DMA-RTC-AUTHORITY:OK
M5:T350:S3:RTC-CMOS:OK
M5:T232:S1:CMOS-RTC-PORT:OK
```

`vm-pcat-composition-s4-smoke` rebuilt but exits `1`.  Static comparison
identifies the cause: its generic default-PC/AT leaf-direction loop still
requires reads at `3D4h`, `3D8h` and `3D9h`, while T374 S12 correctly made the
shared VADP CGA registration write-only and restored default EGA reads only
for Mode/Color.  The same source of truth therefore has a contradictory
default-EGA CRTC-index read claim.  This is not a PIC/PIT/DMA/RTC functional
failure, but it invalidates S12's prior assertion that the profile-leaf
directions were unrelated to handler direction.

## Receiver Decision

No platform-controller repair is selected by this audit: its named selected
functional routes have focused evidence, while every unmeasured phase and the
unselected PPI/refresh observation boundary stays transferred.  The discovered
default-EGA profile-leaf/handler-direction contradiction needs the next
bounded **T374 S16 corrective** repair.  It must reconcile the generic
descriptor's advertised VADP directions with actual configured handlers,
preserve Model-339's absent CGA reads, run both Model-339 and default-PC/AT
topology/composition proof, and not expand EGA/VGA capability or claim timing
or L3 closure.
