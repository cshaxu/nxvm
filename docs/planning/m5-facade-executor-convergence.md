# M5 Facade And Executor Convergence

## Status

Story 1, T75 S1 through T81 S1, is complete. Story 2 is planned but not active.

## Story 1: Remove Residual Global Facades

T64--T73 removed selected-live-object aliases from the inherited executor, but
some real NXVM paths still use process-global keyboard, display, debugger,
input, execution, wait, or platform bindings. This is a correctness and object
ownership problem even when the product exposes only one VM session. It is not
a multi-session product feature.

The story goal is that every host input, display notification, debug target,
and platform call reaches its owning VM session through an explicit object or
provider slot. It preserves the retained NXVM Console, debugger grammar,
window/Console behavior, boot sequence, and FDD DOS-prompt gate.

| Task | Scope | Completion gate |
| --- | --- | --- |
| T75 | Inventory every process-global facade, binding site, production caller, and replacement owner; add a static no-new-facade gate and correct the T73 closure scope. | Approved ledger and focused regression plan; no runtime change. |
| T76 | Replace the old core keyboard facade in the full-PC request path with the owning keyboard provider slot. | Session-targeted keyboard regression, retained Console/input/FDD gates. |
| T77 | Replace global display mode notification and generation state with session/profile-owned display context. | Two-session display-target regression, snapshot and FDD display gates. |
| T78 | Replace the global core debug target with a composition-owned debug target while retaining the existing debugger UI. | Debugger command/output regression and full-PC gate. |
| T79 | Map keyboard, execution, wait, display mailbox, and host-surface ownership before runtime changes. | Approved C1--C6 order, entry-point inventory, and focused gates; no artifact. |
| T80 | Remove platform-state facades in ordered cutovers: C1 Console threads; C2 window threads; C3 Linux thread shape; C4 frame mailbox; C5 mode and handles; C6 scoped wait provider. | Each cutover has its own subtask and regression. C1 requires Console command, keyboard transport, and FDD DOS-prompt gates. |
| T81 | Delete every legacy facade proven to have no production caller; close the facade scan. | Static closure scan, all Story 1 regressions, `nxvm_0_5_0081.exe`. |

## Story 2: Make Core Machine The True Executor

The current `core_machine_create/run` path is a minimal deterministic scaffold;
the real NXVM CPU/BIOS/device executor is still stored in
`vm_composition_live_machine`. This is the wrong direction for the dual-product
architecture.

The target shape is:

```text
vm/main
  -> VM composition creates vm_session
       -> creates one core_machine
       -> creates VM-only devices and default-profile firmware
       -> binds their providers/registries into core_machine
       -> freezes configuration
       -> creates Console/window/platform adapters

  -> VM outer loop
       -> drain host input and Console commands
       -> core_machine_run(core, bounded budget)
       -> handle stop/pause/reset/guest event
       -> capture display snapshot to platform frame
       -> pacing, host wait, and product shutdown
```

`core_machine` owns real CPU/decoder/RAM/port execution, shared guest devices
and deterministic guest-domain timing. It remains synchronous and platform
independent. VM owns profile selection, VM-only devices, BIOS/POST providers,
NXVM Console/window policy, host threading, media commands, and the outer
session loop. VM binds providers into core; it does not copy VM-only devices
into core or execute guest instructions itself.

| Task | Scope | Completion gate |
| --- | --- | --- |
| T82 | Define the real executor convergence interface: `core_machine` storage, reset/run order, provider registration, configuration freeze, and stop/event result. | Approved contract and bounded T83--T86 migration map; no code move. |
| T83 | Move the real NXVM CPU, decoder, RAM, and port-bus executor into one `core_machine` instance. | No duplicate core storage; CPU/Console/FDD-HDD regressions pass. |
| T84 | Move PIC/PIT/DMA/KBC/VADP and shared guest-domain scheduling into that core instance; bind VM-only/profile services as providers. | Device ordering and BIOS/FDD display regressions pass. |
| T85 | Cut NXVM over: the VM outer loop repeatedly calls real `core_machine_run()` and the old `vmachineRefresh`/independent instruction loop is deleted. | FDD boot, BIOS, Console, debugger, pause/reset, and window behavior remain unchanged. |
| T86 | Delete minimal-scaffold and parallel-machine remnants; prove the only guest execution path is `core_machine_run()`. | Source closure scan, full GCC regression, FDD/HDD and three-second DOS-prompt/display gates. |

## Non-Goals

- No session manager, multi-session CLI, session enumeration, or multi-window
  product feature.
- No owned DOS backend, VDM CLI, host-drive mapping, Win16 work, or Microsoft
  component integration.
- No CPU capability expansion, MS-DOS `MEM` triage, or instruction repair in
  this sequence.

## Stop Conditions

Stop for owner review if a task requires copied guest state, a second executor,
a core-to-VM/VDM dependency, changed NXVM Console/debugger behavior, changed
boot/reset timing, or a failed FDD DOS-prompt/display gate.
