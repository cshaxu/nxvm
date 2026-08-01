# M5 Actual Execution Migration

## Baseline And Rule

The source baseline is NXVM commit
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, recorded in
`docs/provenance/m1-nxvm-baseline.md`. The immutable
`src/nxvm-baseline/` copy remains a migration reference. It must not supply
any source to the final `nxvm.exe` once M5 closes.

The retained Console is an exact compatibility surface: command grammar,
default workflow, help/info text, debugger entry without mounted media, and
debugger command/output behavior do not change without owner approval.

## M5 Compatibility Gate

M5 is a structural migration, not a product redesign. It must preserve NXVM
functionality and user experience: the Console and debugger command surface,
output, default workflow, Console/window presentation, input behavior, media
operations, whole-machine debugging, and established boot behavior. A source
move or internal interface change is not permission to alter any of them.

If a change is genuinely unavoidable, stop before implementing it and obtain
explicit owner approval. The approval record must name the affected behavior,
reason no compatible implementation exists, user-visible before/after result,
and focused regression/acceptance plan. Silence, an internal cleanup benefit,
or a build failure is not approval.

Final destination names and ownership in this plan are interpreted through
`docs/architecture/module-layout.md`: `core/{machine,platform,product}` and
`{vm,vdm}/{machine,platform,product,profile}` replace the earlier horizontal
layout. `vm/main.c` and `vdm/main.c` are the product entry points. `adapter`
and all other migration roots are removed. Only `core`, `vm`, and `vdm` are
permitted final source roots; the baseline is deleted after its VM-owned
sources move and its formal-target dependency ends.

## Actual Execution Chain

```text
main -> consoleMain -> machineInit
     -> machineStart/machineResume -> platformStart
     -> Win32 kernel thread -> deviceStart -> vmachineRefresh
     -> device refreshes -> vcpuRefresh -> vcpuinsRefresh
```

`core/machine/machine.c` currently provides lifecycle scaffolding only; it does
not execute guest instructions. The staged NXVM profile currently enters the
chain above through a temporary bridge now under `adapters/nxvm_baseline/full_pc_profile.c`. That bridge is
not a completed migration.

## Source Ownership Map

| Baseline units | Final owner | Required change | Regression gate |
| --- | --- | --- | --- |
| `main.c`, `console.c`, `debug.c`, `xasm32/*` | `vm/main.c`, `vm/product`, and `core/product` | Move the retained Console and debug tooling by `git mv`; preserve grammar. `utils.c` remains with its device/platform consumers until their own slice. | Byte-level command-list markers; `DEBUG` with no media enters `-`, `q` returns to `Console>`. |
| `machine.c`, `device/device.c`, `device/vmachine.c`, `device/vglobal.h` | `core/machine` and `vm/machine` | Replace process-global lifecycle with a session-owned PC/AT execution carrier. The platform-neutral carrier itself belongs in `core/product/runtime`; VM lifecycle and refresh policy remain in `vm/machine`. | Reset vector, stop/reset/resume, no leaked thread, M1 fixture checkpoints. |
| `device/vcpu.c`, `device/vcpuins.c`, `device/vram.c`, `device/vport.c` | `core/machine` | Move the real x86 executor, register state, real/linear memory and I/O dispatch into one machine instance. Existing minimal CPU/RAM/port scaffolds become supporting contracts, not a second executor. | CPU microprobes, normalized `#UD`, reset vector, bounded FDD/HDD progress. |
| `device/vpic.c`, `vpit.c`, `vdma.c`, `vkbc.c` | `core/machine` | Make shared controller state part of the PC/AT instance and retain refresh order. | IRQ/timer/input/display probes plus fixture boot. |
| `device/vvadp.c`, `vcmos.c` | `vm/machine` | Retain the existing BIOS/POST-coupled video and CMOS behavior without presenting it as shared state before its firmware dependencies are separated. | IRQ/timer/input/display probes plus fixture boot. |
| `device/qdx/*` | `vm/profile/default_profile/firmware` | QDX is legacy built-in BIOS-service dispatch, not a machine-neutral device. Move its dispatcher and handlers only as one firmware slice after the display-mode request no longer calls the concrete platform directly. | BIOS INT 09h/10h/13h/16h, input/display probes, and fixture boot. |
| `device/vfdc.c`, `vfdd.c`, `vhdc.c`, `vhdd.c` | `vm/machine` plus `vm/product` media policy | Keep this stack VM-local until independent reuse is proved. | Frozen FDD/HDD identities, create/remove behavior, bounded boot checkpoints. |
| `device/vbios.c` | `vm/profile/default_profile/firmware` | Move ROM/BDA/POST/interrupt table into the profile. A later approved slice may separate CMOS defaults from the retained VM-machine implementation. | Reset image, POST order, boot selection, BIOS interrupt checkpoints. |
| `platform/platform.c`, `platform/win32/*` | `vm/platform` | Replace direct device globals with input queues and copied display snapshots while retaining Console and window behavior. | Console text output, window input/paint, close-stop lifecycle. |
| `platform/linux/*` | `vm/platform/linux` | Preserve as a source migration companion; it is not a Windows M5 acceptance target. | Compile-only until a later Linux run gate. |

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
2. **T9 S2: retained Console source lift.** Move the original Console and
   debugger into `vm/product`, and assembler/disassembler into `core/product`;
   retain `utils` with the still-baseline device/platform sources. Link the
   moved units to the still-baseline execution engine only as a temporary
   compatibility step.
3. **T10: core execution carrier.** Follow the bounded source slices in
   `docs/planning/m5-t10-execution-carrier.md`: move the actual full-PC
   lifecycle and loop into `vm/machine`, then CPU, memory, port, and instruction
   sources into `core/machine`; prove the executor, not a lifecycle stub,
   advances guest instructions under a finite budget.
4. **T11: PC/AT devices and firmware.** Move controller, storage, QDX,
   keyboard, CMOS and BIOS/POST code to `core/machine`, `vm/machine`, and
   `vm/profile/default_profile`, preserving
   refresh and reset order.
5. **T12: platform migration.** First converge all sources into the canonical
   roots, then move Win32 and retained Linux presentation/input code to
   `vm/platform`, replacing direct globals with queues/snapshots
   without changing visible NXVM behavior.
6. **T13: composition cutover.** Wire `vm/product` through `core/product` to
   migrated owners, remove baseline sources from the final target, delete the
   transition adapter, and run the retained FDD/HDD and Console/debugger gates.
   The imported baseline sources have moved to their VM owners and its root is
   deleted after the retained regression evidence is recorded.

Every implementation task produces its task-level `nxvm-m5_t<task>.exe` after
its focused gates pass. A failed fixture, changed Console/debugger transcript,
unbounded recorder run, or need to change a visible command stops that task.
