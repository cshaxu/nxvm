# M5 T13 Core Ownership Cutover

This is the required bounded follow-up to the T13 S1 audit. Every slice keeps
the retained full-PC Console, debugger, boot order, keyboard mapping, and local
FDD/HDD gates unchanged. No slice starts before its predecessor passes.

| Slice | Change | Boundary | Gates |
| --- | --- | --- | --- |
| S6 | Move the platform text snapshot type to `core/machine/presentation.h`; let VDM copy only text cells/attributes from its DOS-minimal snapshot. PIT/IRQ remain VDM-private. | `core/platform` stops including `vdm/machine`. | presentation smoke, full GCC build. |
| S7 | Split `core/product/runtime/session` by product owner. VM full-PC session composition moves to `vm/product`; VDM minimal composition moves to `vdm/product`; any core lifecycle helper is callback-only and has no profile enum or product include. | `core/product` stops selecting VM/VDM profiles. | session/profile smokes, CPU, FDD/HDD, Console/debugger. |
| S8 | Replace `utilsSleep -> vm/platform` with a core host-sleep capability callback, bound by VM product composition and rejected when unbound. | `core/product` has no VM platform include. | Console/debugger delays, CPU, FDD/HDD. |
| S9 | Replace `vcpuins -> deviceStop` with a core execution-stop callback bound by VM lifecycle. | `core/machine` has no VM device include. | CPU probe, finite stop/reset lifecycle, Console/debugger. |
| S10 | Replace PIC/PIT/DMA/KBC direct BIOS calls with a core firmware-service callback/registry binding supplied by the default VM profile. | `core/machine` has no default-profile firmware include. | reset/POST order, keyboard input, FDD/HDD reset vector, bounded boot checkpoints. |

S6 is a type-only source move. S7 through S10 each require a separate active
subtask and must be stopped if their callback changes ordering, lifetime, or
the observable Console/debugger behavior. The current `nxvm-baseline-runtime`
CMake name is a historical aggregate name only; it may be renamed after its
source ownership is resolved, not before.
