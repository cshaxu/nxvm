# M5 Real Executor Migration

## T82 Contract

`core_machine` becomes the sole owner of guest execution storage and the only
module that advances guest instructions. VM owns the outer host loop, profile
selection, VM-only media controllers, BIOS/POST providers, Console/debugger
adaptation, and display publication. It never calls a CPU refresh function.

`core_machine_run(machine, budget, result)` is synchronous. It runs at most
the requested instruction budget, checks stop/reset/pause requests at an
instruction boundary, advances core-owned guest devices in their documented
order, and returns a copied result. It has no host wait, host thread, window,
Console, or VM-profile dependency. A halted CPU returns a waiting result; VM
owns pacing and host sleep.

Configuration is mutable only before `core_machine_freeze`. VM composition
creates the core object, registers VM/profile callbacks, then freezes it.
Reset preserves the frozen topology and resets core storage before invoking
registered provider reset callbacks. A topology change requires construction
of a new core machine.

## Current-To-Target Map

| Current owner | Target owner | Migration task | Rule |
| --- | --- | --- | --- |
| `vm_composition_live_machine.cpu`, `cpuins`, `ram`, `port`, `cpu_execution` | `core_machine` | T83 | One object instance; VM keeps only a borrowed accessor while callers migrate. |
| `vmachineRefresh()` CPU dispatch | `core_machine_run()` | T83/T85 | No CPU refresh wrapper remains in VM after T85. |
| PIC, PIT, DMA, KBC, VADP | `core_machine` | T84 | Core owns storage and guest-domain refresh/reset order. |
| CMOS, FDD/FDC, HDD/HDC | VM machine/profile providers | T84 | VM owns controller/media policy and registers reset/refresh/port/IRQ callbacks. |
| BIOS, QDX, POST, interrupt tables | VM default-profile providers | T84 | VM profile state is borrowed by provider callbacks; core never includes VM headers. |
| control pause/reset/command boundary | VM outer loop | T85 | VM applies commands between `core_machine_run()` calls only. |
| display snapshot and mailbox | VM composition/platform | T85 | Capture only after a returned core quantum. |

## Lifecycle Order

1. VM allocates a `core_machine` and VM/profile-owned provider objects.
2. VM registers all provider callbacks into the configuring core object.
3. VM calls `core_machine_freeze`, then `core_machine_reset`.
4. The VM execution thread drains its command boundary, calls bounded
   `core_machine_run`, processes its result, captures display, and decides
   whether to run another quantum.
5. VM requests stop only through `core_machine_request_stop`; it waits or
   joins only as host policy.
6. VM tears down UI/platform first, then provider objects, then core.

## Migration Tasks

### T83: CPU, RAM, Port, And Decoder

Move the existing `t_cpu`, `t_cpuins`, `t_ram`, `t_port`, and
`core_machine_cpu_execution_context` storage into `core_machine`; expose only
borrowed internal accessors required by existing core instruction code. Bind
the existing decoder to that same storage. Add a bounded core run adapter that
executes one existing CPU refresh per instruction budget unit. VM retains its
old device loop temporarily but has no duplicate CPU/RAM/port storage.

Gate: CPU and memory focused smokes, retained Console/debugger smoke, and FDD
and HDD boot gates. Stop if pointer identity cannot be preserved.

### T84: Shared Devices And Providers

Move PIC, PIT, DMA, KBC, and VADP storage plus their reset/refresh sequence to
the core machine. Introduce frozen callback slots for VM-only controllers and
default-profile firmware. The provider sequence replaces direct calls from
`vmachineInit`, `vmachineReset`, and `vmachineRefresh` without embedding VM
types in core.

Gate: device-order test, BIOS interrupt/POST checks, FDD display and boot.

### T85: NXVM Cutover

Replace the guest portion of `vm_composition_control_start` with repeated
bounded `core_machine_run` calls. Retain VM command draining, pause policy,
host waits, display capture, and console behavior. Remove the independent
`vmachineRefresh` instruction path.

Gate: Console/debugger, pause/reset, window, FDD/HDD, and three-second DOS
prompt/display gates.

### T86: Closure

Delete the minimal core scaffold and all parallel executor storage/wrappers.
Add a static closure verifier that rejects CPU execution advancement outside
`core_machine_run` and rejects VM ownership of CPU/RAM/port/core-device
storage. Run the full GCC and fixture regression set.

## Stop Conditions

Stop for owner review on copied guest state, a second CPU loop, any
core-to-VM dependency, changed Console/debugger grammar, changed boot/reset
timing, or a failed FDD DOS-prompt/display gate.
