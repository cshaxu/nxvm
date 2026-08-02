# M5 Instance Authority Plan

## Objective

Replace every selected-session global and retained live-object alias with
explicit session context. The end state is one `core_machine` session graph per
VM or VDM composition, with no `*_current()`, `*_bind_live()`, `*_unbind_live()`,
or legacy `v*` alias selecting mutable guest state.

This is an implementation plan, not a multi-session product feature. NXVM
Console grammar, debugger UX, boot/reset order, media semantics, and FDD DOS
prompt behavior are compatibility constraints throughout.

## Ordered Tasks

| Task | Authority family | Required result | Artifact |
| --- | --- | --- | --- |
| T65 | root session/composition | Replace the process-global live-machine carrier with an explicitly owned VM session object; make composition control and command targets receive that session. Do not yet remove device aliases. | `nxvm_0_5_0065.exe` |
| T66 | CPU/executor | Pass one explicit core execution context through CPU decode, instruction helpers, probes, and debugger adapters. Retained CPU aliases are compatibility-only until T73 removes the shared selection layer. | `nxvm_0_5_0066.exe` |
| T67 | RAM and port bus | Pass explicit memory/port context through real-mode helpers, CPU I/O, debugger, and direct bus operations. Device callback bodies migrate in T68/T70; retained aliases are compatibility-only until T73 removes the selection layer. | `nxvm_0_5_0067.exe` |
| T68 | core devices | Context-bind PIC, PIT, DMA, KBC, and VADP, including their live lifecycle and core execution edges. Record VM/profile/diagnostic compatibility callers; T73 deletes their aliases only after those owners migrate. | `nxvm_0_5_0068.exe` |
| T69 | core registries | Convert block, keyboard, display, trace, and firmware to session-owned registries/provider slots with configuration/freeze/teardown rules; eliminate their static provider bindings. | `nxvm_0_5_0069.exe` |
| T70 | VM machine providers | Context-bind VM-only CMOS/FDD/FDC/HDD/debug state and callbacks; remove `vcmos`, `vfdd`, `vfdc`, `vhdd`, and `vdebug`. | `nxvm_0_5_0070.exe` |
| T71 | profile firmware | Bind BIOS/QDX/CGA/keyboard/disk firmware through the selected session/provider contexts; remove `vbios`, `qdxTable`, and profile current-object accessors. | `nxvm_0_5_0071.exe` |
| T72 | product/debug/runtime | Remove remaining selected execution-context globals and adapt retained Console/debugger commands to their composition-owned session target. | `nxvm_0_5_0072.exe` |
| T73 | closure | Delete transitional live-access APIs, add a two-session isolation smoke, run full source scans and compatibility gates, and record the resulting instance graph. | `nxvm_0_5_0073.exe` |

## Per-Task Method

1. Map every read, write, callback, initializer, reset, refresh, finalizer,
   and test for the chosen family.
2. Add an explicit context parameter or session-owned provider object. Context
   ownership and teardown are fixed before callers move.
3. Convert direct callers, including firmware and debugger paths, without
   changing guest-visible operation order.
4. Delete the chosen aliases and selected-session accessor functions in the
   same task. A new compatibility alias is prohibited.
5. Build the numbered NXVM artifact; run GCC, dependency-DAG, relevant
   authority smokes, retained Console/debugger smoke, and the FDD DOS-prompt
   observation before opening the next task.

## Required Migration Shape

Every authority-family task follows the same four implementation parts. A
part is a review and recovery boundary, not a separately closed task or a
separate executable artifact.

1. **Object graph:** introduce the session-owned context or registry, record
   its owner, child references, initialization, reset, refresh, and teardown
   order. It contains the real objects; it must not mirror guest state.
2. **Internal ABI:** change internal dispatch and callback signatures to take
   that context explicitly. A function pointer table, provider callback, or
   observer callback may not discover a session through a global.
3. **Callers:** move every direct consumer, including profile firmware,
   debugger, product composition, and smoke code, to the explicit object
   graph. Preserve each existing lifecycle and guest-visible operation order.
4. **Deletion readiness:** remove the family from every newly migrated path
   and add a focused isolation gate proving two independently constructed
   contexts do not cross-write. Retain a legacy entry only when an unmigrated
   later family still calls it; record each remaining caller and its owning
   task.

T66--T72 are explicit-context conversion tasks. T73 is the only task allowed
to delete the shared legacy selection layer wholesale, after its source scan
proves that no caller remains. This is necessary because the current CPU
aliases are consumed by device and profile callbacks that belong to T68--T71;
deleting them in T66 would force an uncontrolled cross-family rewrite. No new
caller may use a legacy alias after its owner task begins.

### T66 CPU/Executor Parts

| Part | Work | Deletion gate |
| --- | --- | --- |
| P1 | Add a composition-owned CPU execution context referencing the one CPU and decoder storage objects, including stop/reset requests. | No new current-object accessor exists. |
| P2 | Replace the no-argument instruction-dispatch ABI with a typed handler receiving that context; thread it through all decoder helpers and execution paths. | `cpu.c` and `cpu_instructions.c` contain no selected-session pointer. |
| P3 | Move VM composition, probes, and core debug adapter to the explicit context; publish the typed instruction-handler ABI for later device/profile injection. | No T66-owned caller resolves CPU state through a selected-session lookup. |
| P4 | Add two-context CPU reset/execution isolation smoke; record any remaining device/profile legacy callers and prohibit new ones. | T66-owned executor and composition paths have no selected-session CPU lookup. Global CPU alias deletion is a T73 gate. |

## Hard Stop Conditions

Stop and obtain owner review if a task requires copied guest state, a second
executor, changed Console grammar, changed boot/reset/refresh order, a
core-to-VM/VDM dependency, or a compatibility-gate failure. The failed task is
the rollback boundary; no later family starts.

## Closure Scan

T73 must find no source declaration, macro, or call of:

```text
vcpu vcpuins vram vport vpic1 vpic2 vpit vdma1 vdma2 vlatch vkbc vvadp
vcmos vfdd vfdc vhdd vdebug vbios qdxTable
*_current() *_bind_live() *_unbind_live()
```

The scan excludes only immutable utility lookups explicitly approved in the
T73 verification record. No mutable machine, provider, profile, product, or
execution state may be selected implicitly.
