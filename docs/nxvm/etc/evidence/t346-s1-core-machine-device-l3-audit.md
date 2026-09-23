# M5 T346 S1: Core-Machine Device And L3 Audit

## Scope And Method

This is the whole-machine planning ledger for the next M5 program. It inspects
the 55 `src/core/machine` C/header units, relevant VM composition and platform
boundaries, 187 `tests/machine` smoke sources, current CMake gate
registrations, existing capability evidence, Queue, and TODO. It does not
change runtime behavior or claim x87 execution.

The local read-only reference surfaces were inspected structurally only:

| Reference | Module surface used to form questions | Not used as an oracle or import |
| --- | --- | --- |
| Bochs 2.6 compatibility tree | `iodev/{pit,pit82c54,pic,dma,cmos,keyboard,floppy,harddrv,vga,vgacore,speaker,devices}` | Source, device manager, firmware, host integration, and timer implementation. |
| PCjs PCx86 tree | `machines/pcx86/modules/v2/{bus,chipset,interrupts,fdc,hdc,keyboard,mouse,memory,rom,video}` | Source, machine configuration, ROM/media, and browser integration. |

Every later implementation must cite its primary hardware contract and a
project probe. These reference trees merely identify state, register, queue,
clock-domain, and ordering questions.

## Measured Timing Baseline

`core_machine_run()` determines instruction cost, advances
`machine->elapsed_ticks`, then calls `core_machine_advance_scheduler()`. That
function converts elapsed time independently for DMA, PIT, VADP, KBC,
RTC/provider clocks and advances them in the fixed order DMA, FDC/HDC refresh,
PIT, VADP, KBC, RTC/provider, PIC refresh. The core path is deterministic and
does not read wall time. It is L2 rather than L3:

* elapsed intervals are delivered in batches rather than ordered due events;
* equal-time order is implementation call order, not stated arbitration;
* memory and I/O have no bus-owner/duration transaction contract; and
* traces record coarse events, not event/transaction causality.

T346 L3 means deterministic event-and-bus timing: one monotonic machine clock,
explicit clock conversion, cancellable events, documented equal-time ordering,
and bounded memory/I/O/DMA transaction visibility. It does not require
prefetch queues, analog video, pin signals, or universal cycle accuracy.

## Capability Ledger

`Implemented` means a bounded focused behavior exists, not whole-chip parity.
`L2` means deterministic behavior without the L3 event/bus contract.

| Family and one owner | Current evidence | Current timing/status | Windows 3.x relevance | Receiver |
| --- | --- | --- | --- | --- |
| Machine run, instruction cost, clock ratios | `machine.c`, `clock.c`, timing and CPU smokes | Deterministic L2, fixed post-retirement order | Foundation | **T346 S2** owns one event/transaction contract. |
| Physical memory, A20, immutable ROM, port `92h` | `memory.c`; memory/A20/ROM smokes | Functional L2 mapping, no wait-state transaction | Boot, BIOS, DMA, video | **S2** owns transaction semantics; reset-image distinction remains TODO. |
| Port dispatcher/providers | `port.c`, `port_interface.c`; assembly/ownership/I/O smokes | Synchronous functional L2 | Driver programming | **S2** owns port transaction boundary. |
| Cascaded PIC | `pic.c`; IRQ lifecycle smoke | Source lifecycle, fixed refresh, L2 | IRQ0/1/6/8/12/14 | **S3** owns priority, EOI, equal-time, reset order. |
| PIT | `pit.c`; divider/readback/waveform smokes | Tick-loop waveform, L2 | BIOS ticks and scheduler | **S3** owns deadline/latch/IRQ ordering. |
| 8237 DMA and FDC binding | `dma.c`; channel/binding/authority smokes | One-unit grant loop and frozen binding, L2 | FDC and contention | **S3** owns arbitration, boundaries, abort/reset, bus visibility. |
| RTC/CMOS/NMI | `rtc.c`; RTC/CMOS/authority smokes | Provider-clock batches, L2 | BIOS time and IRQ8 | **S3** owns calendar/periodic/alarm/NMI order. |
| 8042 KBC/A20/reset/AUX | `kbc.c`; controller/AUX/guest keyboard smokes | Response/typematic countdowns, L2 | Keyboard, A20, warm reset | **S4** owns queued-input timing, IRQ order, reset handoff; advanced AUX stays TODO. |
| FDC and media | `fdc.c`; topology/media/read-track/boot smokes | Command-refresh model, L2 | Installer/floppy boot | **S3** owns ready, DMA/IRQ, seek/transfer, abort timing. |
| ATA/HDC PIO | `hdc.c`; HDC/boot/port smokes | Synchronous sector refresh, L2 | Disk boot/setup media | **S3** owns ready/DRQ/IRQ/service timing; backing TODO remains. |
| VADP CGA/EGA/display copy | `vadp.c`, `display.c`; CGA/EGA/text/display smokes | Batch VADP clock, copied output, L2 | Display driver/retrace | **S4** owns retrace/vblank, modes, presentation cadence. |
| Firmware/BDA/INT composition | VM firmware and boot smokes | Generated ROM/BDA reset consumers, L2 | Boot/BIOS/installers | **S3/S4** consume timing; ROM materialization remains TODO. |
| Host presentation/input/sleep/thread boundary | VM platform/session paths | Explicit host boundary | Must not define guest time | **S4** proves copied boundaries; host cadence is never L3 evidence. |
| Speaker/PPI `61h`, serial, parallel, game port | No core owner/smoke; comparable reference modules exist | Missing selected corpus | Low for initial Windows research | New bounded TODOs; corpus and hardware contract required. |
| Broad EGA/VGA/VBE and composite | Bounded CGA/EGA only; existing CGA TODO | Partial L2 coverage | Driver selection may need more modes | **S4** selects Windows-required digital modes; remaining breadth stays TODO. |

## Dependency Plan

```text
S2 deterministic time + due events + transaction boundary
  |- S3 PIC/PIT/RTC/DMA/FDC/ATA ordering and storage readiness
  |- S4 KBC/AUX/VADP/display/presentation ordering
  `- S5 PC/AT L3 reconciliation -> Windows 3.x readiness map
```

S2 is first: improving devices while retaining independent `advance()` loops
would recreate the present piecemeal ordering model. S3/S4 may start only once
S2 names their migration boundary. S5 transfers every unselected device and
non-L3 feature once before the Windows readiness map begins.

## Reconciliation And Deferred Boundaries

`current-capability-baseline.md` is refreshed by this S to use the accepted
T343 CPU closure and measured L2 timing wording. The TODOs for advanced AUX,
large/sparse backing, CGA breadth, composite output, reset policy, generated
ROM, speaker/PPI, and serial/parallel/game interfaces remain separate because
S1 has no probe proving each is a Windows-core requirement. x87/80287/80387
execution is excluded.

## Verification Record

The audit used `rg --files src/core/machine src/vm tests`, explicit searches
for device initialization/advance/port registration, CMake current targets,
TODO headings, host-time/thread indicators, and reference-tree module names.
No reference source, firmware, or guest media was copied, executed, or made a
dependency.

Promotion/retirement: retain through T346 S5, then merge final dispositions
into T346 history and the Windows readiness map.
