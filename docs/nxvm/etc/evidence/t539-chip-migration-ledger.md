# T539 S1: Finite Devices Inventory

Baseline 996a19a17. All 81 tracked files under `src/app-nxvm/devices` are listed
exactly once below. Entries remain **designed, not migrated**, except the
PIT row accepted by S3, RTC row accepted by S4, PIC row accepted by S5,
DMA row accepted by S7, AT keyboard chain accepted by S8 and XT PPI/keyboard
accepted by S9;
the inventory stays finite.
The [design review](../architecture/t539-independent-chip-design.md) supplies
dependency details, proposed contracts, regression ownership and decision gates.
This ledger is not a claim of completed chip semantics/timing qualification.

| Files | Proposed disposition and change |
| --- | --- |
| `cpu.c`, `cpu.h`, `cpu_instructions.c`, `cpu_instructions.h`, `cpu_interface.h`, `cpu_timing.c`, `cpu_timing.h`, `cpu_timing_model.c`, `cpu_trace.h` | Extract CPU execution/timing; remove machine/private peer dependencies; preserve tables; resolve firmware hook, bus, INTA and generated catalogs. |
| `fpu.c`, `fpu.h`, `fpu_interface.h` | Extract implemented FPU state/extension behavior; replace CPU-private coupling with explicit extension boundary. |
| `pic.c`, `pic.h`, `pic_interface.h` | S5 implementation: sole controller mechanism in `src/x86/devices/pic8259`; NXVM pic_bus owns ports, source counts and pair wiring. [Evidence](t539-s5-pic-extraction.md) records verification; Current owns acceptance. |
| `pit.c`, `pit.h` | S3 implementation: sole timer moved to `src/x86/devices/pit825x`; NXVM `pit_bus` owns port attachment, existing board/scheduler own wiring and clocks. Verification and acceptance are recorded in S3 evidence. |
| `dma.c`, `dma.h` | S7 implementation: sole controller in `src/x86/devices/dma8237`; NXVM dma_bus retains page/lane expansion, pair arbitration and physical cycle provider. [S7 evidence](t539-s7-dma-extraction.md) maps original tests and verification; Current owns acceptance. [S6](t539-s6-dma-first-service.md) retains the prior first-service repair evidence. |
| `rtc.c`, `rtc.h` | S4 accepted: sole mechanism moved to `src/x86/devices/rtc146818`; NXVM owns index/NMI, PIC signal binding, seed/checksum and clocks. Verification and actual-change acceptance are recorded in S4 evidence. |
| `kbc.c`, `kbc.h` | S8 implementation: opaque `kbc8042`, `keyboard` and `ps2mouse` in Shared; the original NXVM pair now owns only endpoint construction, ports, IRQ, A20/reset and time attachment. [Evidence](t539-s8-kbc-extraction.md) records order, test mapping and receivers; Current owns acceptance. |
| `xt_keyboard.c`, `xt_keyboard.h` | S9 implementation: sole serial/FIFO/BAT owner moved to `x86/devices/xtkeyboard`; old pair and concrete PPI dependency removed. [Evidence](t539-s9-xt-extraction.md) maps cases and verification; Current owns acceptance. |
| `xt_ppi_keyboard.c`, `xt_ppi_keyboard.h` | S9 implementation: qualified Mode-0 registers in `x86/devices/ppi8255`; NXVM pair retains DIP/NMI/speaker/receiving latch and IRQ wiring. No full-8255 claim. [Evidence](t539-s9-xt-extraction.md) records rollback and release proof. |
| `fdc.c`, `fdc.h`, `fdc_observation_interface.h` | S13 implementation: opaque command/PCN/cause owner in `x86/devices/fdc8272`; NXVM retains PC registers, physical drive/record provider, ports and PIC/DMA wiring. Copied diagnostics replace private caller access. [S13 evidence](t539-s13-fdc-extraction.md) maps original scenarios and verification; Current owns acceptance. S11/S12 qualification and the documented READ TRACK/timing limits are preserved, not promoted to complete silicon qualification. |
| `hdc.c`, `hdc.h` | Extract explicit controller personalities, not universal ATA; detach IRQ/DMA/board port routing. |
| `vadp.c`, `vadp.h` | Extract video mechanisms with one VRAM/frame owner; detach physical memory/port installation. |
| `controller_interface.h` | Split mixed chip variants/timing value types from board port/IRQ/DMA/topology configuration; no shared umbrella machine config. |
| `display.c`, `display_interface.h`, `guest_display_frame.h`, `presentation_interface.c`, `presentation_interface.h` | Split device-owned copied display values from machine display-provider slot and product presentation bridge; keep slot/bridge in board/adapter. |
| `device_support.h` | Audit macros by real owner; retain needed owner-local mechanics or existing Types equivalents; no new catch-all public utils component. |
| `clock.c`, `clock.h`, `timeline.c`, `timeline.h` | Retain board clock conversion/event ordering; chips receive documented time inputs, not machine timeline pointers. |
| `transaction.c`, `transaction.h` | Retain board transaction/arbitration owner; expose bounded bus effects to CPU/DMA without shared private latch pointers. |
| `memory.c`, `memory.h`, `memory_interface.c`, `memory_interface.h`, `port.c`, `port.h`, `port_interface.c`, `port_interface.h` | Retain physical storage/address decode/port routing; CPU translation stays CPU, video local state stays video. |
| `d4_memory.c`, `d4_memory.h` | Retain Model-40 board-specific memory mapping, not an independent chip. |
| `machine.c`, `machine.h`, `machine_board.c`, `machine_display.c`, `machine_firmware.c`, `machine_interface.h`, `machine_plan.c`, `machine_scheduler.c` | Retain machine composition/scheduling; replace embedded/private chip state and peer inspection with public connections; later ibmpc task classifies common board logic. |
| `debug.c`, `debug_interface.h`, `retirement_observation_interface.c`, `retirement_observation_interface.h`, `trace_interface.c`, `trace_interface.h` | Retain machine debug/observation adapter, consume copied CPU/device observations; not another shared Debug CLI. |
| `entry_plan_interface.c`, `entry_plan_interface.h`, `firmware_interface.h`, `rom_mapping_interface.c`, `rom_mapping_interface.h` | Retain entry/firmware/ROM board-service contracts; separately decide live CPU interception dependency. |
| `media_interface.c`, `media_interface.h` | Retain registry/lifetime at board; separate only necessary neutral per-device media operations; no host file/storage dependency in chip. |
| `execution_provider.h`, `guest_input_interface.h`, `lifecycle_interface.h` | Retain machine execution/input/lifecycle boundary; board maps input to actual attached device. |

## Inspected Boundary Evidence

Source anchors use symbol names to survive subsequent line moves:

- CPU: execution context connections in `cpu_instructions.h`; PIC acknowledgement
  in `cpu_instructions.c`; machine-private timing in `cpu_timing_model.c`;
  `core_machine_firmware_handle_software_interrupt` and its machine binding.
- PIC: paired initialization/acknowledgement and cascade fields. DMA: paired
  initialization/advance, page/lane address construction, provider validation
  and binding token allocation. PIT: local waveforms/output and port binding.
- KBC: `core_machine_kbc_apply_output_port`, explicit reset and serial/BAT paths.
  XT PPI header's Mode-0 limitation; XT keyboard concrete PPI delivery.
- FDC: `core_machine_fdc_complete_unready_read`, drive seek and PC adapter ports.
  HDC: personality dispatch, DMA provider and PIC source connection.
- RTC: PIC binding/IRQ and machine timing includes. Video: physical mapping,
  observers and copied frame generation. Scheduler: private FDC phase inspection,
  DMA pair inspection and clock-domain deadline conversion.
- Build: `cmake/nxvm/NxvmProduct.cmake` executor/observable source ownership and
  generated timing catalogs; `src/x86/CMakeLists.txt` and `verify_corpus.cmake`
  at the S1 baseline supported only xasm32/debug, with Common assembled
  unconditionally. S3 adds a Types-only PIT target and a tools-off standalone
  build that does not assemble Common.
- Tests: `test/app-nxvm/unit/core/devices` and `unit/core/machine` contain mixed
  chip and board assertions; profile and integration tests remain product-owned.
  Relevant families include PIC priority/phase/lifecycle, PIT waveform/readback,
  DMA channel/token/Model-40, KBC serial/AUX, FDC topology/media, HDC personalities,
  RTC calendar/CMOS, video text/planar geometry, CPU timing/exception/FPU ledgers.

## Verification Scope

Tracked-file set equality checks the inventory against Git, not an inferred
directory count. Design-only edits require Markdown/reference and documentation
governance verification; no executable or runtime-test result is claimed here.
Baseline T538 runtime acceptance remains historical evidence, not proof of this
future architecture. S1 changed no source, tests, manifests, binary or INI.
For the PIT cutover, see [S3 evidence](t539-s3-pit-extraction.md).
For the RTC batch, see [S4 evidence](t539-s4-rtc-extraction.md).
