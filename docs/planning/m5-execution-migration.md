# M5 Actual Execution Migration

## Baseline And Rule

The source baseline is NXVM commit
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, recorded in
`docs/provenance/m1-nxvm-baseline.md`. The immutable
`src/nxvm-baseline/` copy remains a regression reference. It must not supply
any source to the final `nxvm.exe` once M5 closes.

The retained Console is an exact compatibility surface: command grammar,
default workflow, help/info text, debugger entry without mounted media, and
debugger command/output behavior do not change without owner approval.

## Actual Execution Chain

```text
main -> consoleMain -> machineInit
     -> machineStart/machineResume -> platformStart
     -> Win32 kernel thread -> deviceStart -> vmachineRefresh
     -> device refreshes -> vcpuRefresh -> vcpuinsRefresh
```

`core/machine.c` currently provides lifecycle scaffolding only; it does not
execute guest instructions. `products/nxvm/default_profile.c` currently enters the chain
above through `adapters/nxvm_baseline/full_pc_profile.c`. That adapter is not a
completed migration.

## Source Ownership Map

| Baseline units | Final owner | Required change | Regression gate |
| --- | --- | --- | --- |
| `main.c`, `console.c`, `debug.c`, `utils.c`, `xasm32/*` | `products/nxvm` | Copy/derive the retained Console and hardware debugger as a product-owned surface; inject a session command boundary instead of changing its grammar. | Byte-level command-list markers; `DEBUG` with no media enters `-`, `q` returns to `Console>`. |
| `machine.c`, `device/device.c`, `device/vmachine.c`, `device/vglobal.h` | `core` and `runtime` | Replace process-global lifecycle with a session-owned PC/AT execution carrier. The carrier owns refresh order and the execution thread. | Reset vector, stop/reset/resume, no leaked thread, M1 fixture checkpoints. |
| `device/vcpu.c`, `device/vcpuins.c`, `device/vram.c`, `device/vport.c` | `core` | Move the real x86 executor, register state, real/linear memory and I/O dispatch into one machine instance. Existing minimal CPU/RAM/port scaffolds become supporting contracts, not a second executor. | CPU microprobes, normalized `#UD`, reset vector, bounded FDD/HDD progress. |
| `device/vpic.c`, `vpit.c`, `vdma.c`, `vkbc.c`, `qdx/*`, `vvadp.c` | `core` | Make PIC/PIT/DMA/keyboard/QDX/video state part of the PC/AT instance and retain refresh order. | IRQ/timer/input/display probes plus fixture boot. |
| `device/vfdc.c`, `vfdd.c`, `vhdc.c`, `vhdd.c` | `core` plus `products/nxvm` media policy | Keep controllers generic; pass block providers from the product composition. No host-path policy in core. | Frozen FDD/HDD identities, create/remove behavior, bounded boot checkpoints. |
| `device/vbios.c`, `vcmos.c` | `firmware/default_profile` | Move ROM/BDA/POST/interrupt table/CMOS behavior into the built-in firmware provider. | Reset image, POST order, boot selection, BIOS interrupt checkpoints. |
| `platform/platform.c`, `platform/win32/*` | `platform/win32` | Replace direct device globals with input queues and copied display snapshots while retaining Console and window behavior. | Console text output, window input/paint, close-stop lifecycle. |
| `platform/linux/*` | `platform/linux` | Preserve as a source migration companion; it is not a Windows M5 acceptance target. | Compile-only until a later Linux run gate. |

## Dependency Breaks

The imported execution code has module-wide globals (`vcpu`, `vram`, `vport`,
`device`, and device-specific globals) plus callbacks that carry no instance
context. The migration may use a short-lived PC/AT execution-context binding on
the single machine thread while files are moved, but it may not leave a
process-global final execution path. Each migrated group must either receive an
explicit `nxvm_core_machine`/PC-AT state pointer or be isolated behind an
instance-owned callback table. Platform threads never access that state
directly.

The product Console and debugger may keep their original parser and output, but
their old `deviceConnect*` calls must be routed through the synchronized debug
and command registry before the baseline adapter is removed.

## Bounded Migration Order

1. **T9 S1: default-profile naming.** Rename the built-in profile implementation
   to `default_profile`; the emulated PC/AT identity remains factual, while
   future machine profiles receive their own names.
2. **T9 S2: retained Console source lift.** Move the original Console, debugger,
   assembler/disassembler and support code into `products/nxvm`; link it to the
   still-baseline execution engine only as a temporary compatibility step.
3. **T10: core execution carrier.** Move the actual CPU, memory, port and
   machine/device loop into `core`; prove the executor, not a lifecycle stub,
   advances guest instructions under a finite budget.
4. **T11: PC/AT devices and firmware.** Move controller, storage, QDX,
   keyboard, CMOS and BIOS/POST code to `core` or `firmware/default_profile`, preserving
   refresh and reset order.
5. **T12: platform migration.** Move Win32 and retained Linux presentation/
   input code to `platform`, replacing direct globals with queues/snapshots
   without changing visible NXVM behavior.
6. **T13: composition cutover.** Wire `products/nxvm` through `runtime` to
   migrated owners, remove baseline sources from the final target, and run the
   retained FDD/HDD and Console/debugger gates. The baseline remains a separate
   reference target only.

Every implementation task produces its task-level `nxvm-m5_t<task>.exe` after
its focused gates pass. A failed fixture, changed Console/debugger transcript,
unbounded recorder run, or need to change a visible command stops that task.
