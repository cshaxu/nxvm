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

The first implementation slice is CPU/RAM/port authority only. It moves the
actual `vcpu`, `vram`, and `vport` storage behind the live machine handle while
retaining instruction, memory, and I/O semantics. Shared devices follow only
after that slice proves FDD DOS boot. VM-only devices and profile firmware bind
to that same handle later.

## Debugger Consequence

T23 may add pause request and acknowledgement to the current live execution
loop; it must not target the separate minimal model. T24 may expose a unified
debug backend only over the live machine handle. An adapter that merely mirrors
or redirects an unrelated `nxvm_core_machine` is prohibited.

## Stop Conditions

Stop for design review if the first CPU/RAM/port slice needs a second machine
allocation, changes POST/reset/refresh ordering, changes the retained Console
or debugger grammar, or loses the FDD DOS prompt checkpoint.
