# M5 Live Machine Authority Migration

## Decision

The bootable NXVM product has one live machine authority. It currently consists
of the inherited global machine state driven by `vmachineInit`,
`vmachineReset`, and `vmachineRefresh`. `nxvm_core_machine` is presently a
separate minimal CPU/RAM/port model and is not permitted to become a parallel
product machine. It is test/session infrastructure until the real state moves
into it.

No product path may allocate, reset, or debug two independent CPU, RAM, or
port states for one NXVM session. A temporary compatibility accessor may refer
to the live object, but it must not cache, mirror, or synchronize a copy.

## Current Live Authority

| Live source | Current responsibility | Target owner |
| --- | --- | --- |
| `vcpu`, `vcpuins` | x86 state and instruction execution | `core/machine` instance state |
| `vram` | guest RAM and A20 behavior | `core/machine` instance state |
| `vport` | I/O dispatch | `core/machine` instance state |
| `vpic`, `vpit`, `vdma`, `vkbc`, `vvadp` | shared PC-compatible devices | `core/machine` instance device set |
| `vcmos`, `vfdc`, `vfdd`, `vhdc`, `vhdd`, `vdebug` | bootable-VM-only devices and debug instrumentation | `vm/machine` providers bound to the instance |
| `vbios`, `qdx`, `qdcga`, `qdkeyb`, `qddisk` | default-profile firmware and firmware services | `vm/profile/default_profile` providers |
| `vmachine*`, composition loop | product assembly, order, execution ownership | `vm/` composition |

## Migration Rules

1. `nxvm_core_machine` becomes the container for the live state; its current
   small CPU/RAM/port fields do not coexist with migrated product state.
2. A migration moves storage and all reads/writes for one authority at a time.
   It may leave a temporary accessor, but never a second allocation or copied
   state.
3. Initialization, reset, and refresh order in `vmachine*` are behavior
   contracts. Moving a unit does not reorder calls.
4. Each migrated authority requires GCC, dependency-DAG, retained Console and
   debugger gates, then an FDD DOS boot to the recorded prompt checkpoint.
5. `nxvm_core_machine` session tests remain useful but cannot substitute for a
   full-PC boot regression.

## Required Staging

Before a storage move, introduce one composition-owned live machine handle that
identifies the real CPU/RAM/port/device set. It may expose compatibility
accessors to inherited callers, but it has no duplicate CPU, RAM, or port
storage. Product code must not construct the current minimal
`nxvm_core_machine` as an additional machine for a bootable session.

After T23, CPU, RAM/A20, and port authority move in separate tasks. Each keeps
instruction, memory, and I/O semantics unchanged and proves FDD DOS boot before
the next authority moves. Shared devices follow, then VM-only devices and
profile firmware bind to that same handle.

## Controlled Execution Method

T23 is a no-behavior-change staging task. VM root composition creates one
private live-machine handle. That handle is an identity and a set of typed,
non-owning references to the existing live CPU, instruction decoder, RAM, port
dispatcher, and later device objects. It owns no guest storage, calls no
device initializer, and does not replace `vmachineInit`, `vmachineReset`, or
`vmachineRefresh`.

The handle is bound once during the existing full-PC composition setup, before
the retained machine lifecycle begins. Its references point directly at the
already-existing objects. T23 may add a focused binding check, but no retained
Console, firmware, device, or execution caller may be redirected through the
handle yet. The current minimal `nxvm_core_machine_create` path remains a
separate test/session path and is not constructed by full-PC composition.

Each later device task uses this exact sequence:

1. Map every definition, read, write, initializer, finalizer, and callback for
   the one authority.
2. Bind the pre-existing object through the live handle and add a focused
   identity check. The check proves both paths name the same object.
3. Move the authority's storage and its direct callers to the selected instance
   representation. A temporary compatibility access path may dereference that
   same storage, but may not allocate, cache, copy, or synchronize it.
4. Preserve the exact existing init, reset, refresh, finalization, IRQ/DMA, and
   firmware callback order. Do not combine a device move with a functional fix.
5. Build the task artifact, run the static dependency gates and retained
   Console/debugger smokes, then boot the FDD fixture to the DOS prompt before
   opening the next task.

The compatibility path is transitional and must be listed by the task that
introduces it. It is removed only when all of that authority's callers use the
live instance. It must remain a direct alias or accessor to the sole object;
mirrored structs, copy-in/copy-out bridges, dual reset paths, and background
synchronization are forbidden.

Stop immediately for design review if a move requires a second allocation,
changes observable boot timing or device order, requires a core-to-VM include,
changes retained Console/debugger grammar, or fails the FDD DOS-prompt gate.
The failed task remains the rollback boundary; no later device task begins.

## Debugger Consequence

Debugger migration begins only after this authority convergence. It must target
the single live machine handle. An adapter that mirrors or redirects an
unrelated `nxvm_core_machine` is prohibited.

## Task Breakdown

Every implementation task below changes one primary authority only, compiles a
task-level `nxvm-m5_t<N>.exe`, and retains the full-PC FDD DOS-prompt gate.
The temporary compatibility surface may be reduced only after the corresponding
device task proves that no second storage remains.

| Task | Primary authority | Scope |
| --- | --- | --- |
| T23 | live-machine carrier | Create the one composition-owned live machine handle and bind existing state by reference only; no duplicate CPU/RAM/port allocation. |
| T24 | CPU/executor | Move `vcpu` and `vcpuins` state and execution into the live machine instance. |
| T25 | RAM/A20 | Move `vram` storage, allocation, translation, and A20 state into the instance. |
| T26 | port dispatcher | Move `vport` ownership and I/O dispatch table into the instance. |
| T27 | PIC | Move `vpic` state and instance binding. |
| T28 | PIT | Move `vpit` state and instance binding. |
| T29 | DMA | Move `vdma` state and instance binding. |
| T30 | keyboard controller | Move `vkbc` state and instance binding. |
| T31 | video adapter | Move `vvadp` state and instance binding. |
| T32 | CMOS/RTC | Bind the VM-only `vcmos` provider to the instance. |
| T33 | floppy media | Bind `vfdd` media state to the instance. |
| T34 | floppy controller | Bind `vfdc` controller state and DMA/IRQ links to the instance. |
| T35 | hard-disk media | Bind `vhdd` media state to the instance. |
| T36 | hard-disk controller | Bind `vhdc` controller state and BIOS service links to the instance. |
| T37 | BIOS/POST | Bind `vbios` firmware image, POST, and interrupt registrations to the instance while preserving default-profile ownership. |
| T38 | QDX service | Bind the profile video interrupt service through the instance. |
| T39 | CGA text service | Bind `qdcga` profile text state and display snapshot provider through the instance. |
| T40 | keyboard firmware service | Bind `qdkeyb` through the instance keyboard controller and platform ingress. |
| T41 | disk firmware service | Bind `qddisk` INT 13 services through the instance media/controller providers. |
| T42 | debug instrumentation | Bind `vdebug` break, watch, bounded trace, and stop-reason state to the instance. |
| T43 | full-PC closure | Remove obsolete global compatibility storage, prove one authority across init/reset/refresh, and freeze the migrated full-PC baseline. |
| T44 | debugger pause boundary | Add request, acknowledgement, pause reason, step, and continue to the converged live execution loop. |
| T45 | unified debugger backend | Route the retained debugger UI and future VDM debugger through the single live-machine backend. |

T23 is structural preparation, not a second machine model. T24 through T42 are
one-device tasks; T43 is the required composition closure. T44 and T45 remain
inactive until T43 proves a single authority.

## Stop Conditions

Stop for design review if the first CPU/RAM/port slice needs a second machine
allocation, changes POST/reset/refresh ordering, changes the retained Console
or debugger grammar, or loses the FDD DOS prompt checkpoint.
