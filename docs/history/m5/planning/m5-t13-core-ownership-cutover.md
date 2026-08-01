# M5 T13 Core Ownership Cutover

This is the required bounded follow-up to the T13 S1 audit. Every slice keeps
the retained full-PC Console, debugger, boot order, keyboard mapping, and local
FDD/HDD gates unchanged. No slice starts before its predecessor passes.

## Supersession

T13 S6 through S8 are complete. The remaining S9 through S11 source-level
fixes are superseded by `docs/planning/m5-dependency-governance.md`: a local
callback replacement is insufficient unless the complete source and CMake
dependency graph reaches the product-composition model in
`docs/architecture/module-layout.md`.

| Slice | Change | Boundary | Gates |
| --- | --- | --- | --- |
| S6 | Move the platform text snapshot type to `core/machine/presentation.h`; let VDM copy only text cells/attributes from its DOS-minimal snapshot. PIT/IRQ remain VDM-private. | `core/platform` stops including `vdm/machine`. | presentation smoke, full GCC build. |
| S7 | Define the product-session split: core retains only a generic profile descriptor; VM and VDM own their static descriptors and session compositions. | No source behavior change. | replacement API review and test migration map. |
| S8 | Split `core/product/runtime/session` by product owner. VM full-PC session composition moves to `vm/product`; VDM minimal composition moves to `vdm/product`; core carries neither product enum nor product include. | `core/product` stops selecting VM/VDM profiles. | session/profile smokes, CPU, FDD/HDD, Console/debugger. |
| S9 | Replace `utilsSleep -> vm/platform` with a core host-sleep capability callback, bound by VM product composition and rejected when unbound. | `core/product` has no VM platform include. | Console/debugger delays, CPU, FDD/HDD. |
| S10 | Replace `vcpuins -> deviceStop` with a core execution-stop callback bound by VM lifecycle. | `core/machine` has no VM device include. | CPU probe, finite stop/reset lifecycle, Console/debugger. |
| S11 | Replace PIC/PIT/DMA/KBC direct BIOS calls with a core firmware-service callback/registry binding supplied by the default VM profile. | `core/machine` has no default-profile firmware include. | reset/POST order, keyboard input, FDD/HDD reset vector, bounded boot checkpoints. |

S6 is a type-only source move. S7 through S10 each require a separate active
subtask and must be stopped if their callback changes ordering, lifetime, or
the observable Console/debugger behavior. The current `nxvm-baseline-runtime`
CMake name is a historical aggregate name only; it may be renamed after its
source ownership is resolved, not before.
