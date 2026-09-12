# M5 T528 S1 VM-Machine Owner Ledger

## Scope And Method

Baseline: `90127f67`. This ledger covers every tracked C/H production file
under `src/vm/machine/`; tests, generated files and sibling SoftPC source are
not production owners. Queries used:

```powershell
Get-ChildItem src\vm\machine -Recurse -File | Where-Object {
  $_.Extension -in '.c','.h'
}
rg -n 'vm_machine_(control|runner|waiting|lifecycle|executor_state|debug_|frame|bind_display|publish_result|print_)|common_machine_' src/vm src/common test/vm test/common CMakeLists.txt
$softpc_root = '<owner-managed SoftPC worktree>'
rg -n 'app_control|app_input_queue|app_runtime|softpc_machine_' "$softpc_root/src/app" "$softpc_root/src/host"
```

SoftPC was inspected read-only. It supplies semantic counterparts, not source
to import: `app/control` owns its total control queue; `app/input_queue` owns
per-run guest input retirement; `app/runtime` owns its executor loop; and
`host/machine` is its MVDM/CCPU adapter.

## Current Common Fact

`common/machine` already owns a bounded copied request FIFO, run binding,
wake event, one-request safe-point dispatch and a paused-debug lease. It does
not yet own a complete executor state machine, reset loop, completion facts or
worker lifetime. T528 may delete only paths genuinely duplicated by the
existing owner. The residual executor mechanism has the named receiver T529,
[Common-machine residual audit proposal companion](../../history/M5-T529-common-machine-residual-audit-proposal.md); it must not be copied or prematurely deleted.

## File Dispositions

| Files | Current responsibility | Disposition and receiver |
| --- | --- | --- |
| `debug.c`, `debug.h` | NXVM-local execution-plan/observer state around Core debug. | T528 S3: reduce to the Core target bridge or relocate NXVM-only recorder observation to `vm/product`; Common Debug keeps grammar and Common machine keeps lease. No Common move. |
| `fdd.c`, `fdd.h`, `fdd_private.h` | FDD geometry and Core-media-to-Lib-storage adapter. | T528 S4: relocate to NXVM `vm/media` or profile-local owner; never Common. |
| `hdd.c`, `hdd.h`, `hdd_private.h` | HDD geometry and Core-media-to-Lib-storage adapter. | T528 S4: relocate to NXVM `vm/media` or profile-local owner; never Common. |
| `request_factory.c`, `request_factory.h`, `request_interface.h` | NXVM session/profile request to machine construction. | Retain NXVM, with placement reviewed in S4; no SoftPC-common semantic. |
| `runtime/control.c`, `runtime/control.h` | Old VM pause/reset/resume/stop, events, step state and command-boundary binding. | T528 S2 deletes or bypasses only request ingress/state already duplicated by Common FIFO/lease. Its executor-state/completion remainder transfers explicitly to T529; no parallel path remains after T529. |
| `runtime/debug_adapter.c`, `runtime/debug_adapter.h` | Bounded `common/machine` debug request to Core debug API translation. | Retain as the one NXVM Core target adapter; simplify S3 if it has duplicate state. SoftPC needs its own adapter, not this file. |
| `runtime/display.c`, `runtime/display.h` | Copies Core display facts and publishes NXVM result records. | Retain as NXVM Core-display adapter. `common/ui` owns presentation surfaces/mailboxes, not Core snapshot conversion. |
| `runtime/event_interface.h` | Copied NXVM machine result/input ABI. | Retain as NXVM adapter ABI; verify no duplicate lifecycle reducer in S2/S3. |
| `runtime/execution.c`, `runtime/execution.h` | Core execution-context enter/leave and callback binding. | Retain as NXVM Core executor adapter, but T529 receives only its neutral worker mechanics. |
| `runtime/executor_state.c`, `runtime/executor_state.h` | Old VM active/paused/reset state. | T528 S2 inventory receiver: Common has request intent but not actual executor state; T529 must absorb it before deletion. |
| `runtime/fault.c`, `runtime/fault.h` | Converts Core outcome into copied NXVM fault result; also formats fault text. | Retain copied Core outcome mapping; move printing to `vm/product` in T528 S3. |
| `runtime/frame.c`, `runtime/frame.h` | Copies a Core display snapshot into `lib/ux` frame ABI. | Retain as NXVM Core-frame adapter, consistent with architecture; no presenter ownership. |
| `runtime/lifecycle.c`, `runtime/lifecycle.h` | Mixed Common request submission, Core reset/start/stop, lifecycle facts, initialization and legacy control. | Split in T528 S2: retain Core adapter hooks and copied fact publication; delete duplicated request submission/state ownership. T529 receives neutral actual-lifecycle/completion mechanics. |
| `runtime/machine.c`, `runtime/machine_interface.h`, `runtime/machine_private.h` | Mixed Core construction, Common driver, profile/media operations, input mapping, speed, public adapter ABI and legacy state. | T528 S2-S4 split by owner: Core assembly remains VM machine; profile/media moves NXVM-local; Common dispatch glue remains thin; legacy control fields/routes are removed. No direct Common migration of profile/Core fields. |
| `runtime/machine_devices.c`, `runtime/machine_devices.h` | Materializes Core machine device/media plan. | Retain NXVM composition, relocate with media/profile ownership in S4 if needed; no Common consumer. |
| `runtime/machine_info.c`, `runtime/machine_info.h` | Formats machine/BIOS/status text. | T528 S3 move to `vm/product` monitor policy; delete VM-machine printing API. |
| `runtime/media.c`, `runtime/media.h` | Binds the NXVM FDD/HDD adapters to Core media. | T528 S4 relocate alongside `vm/media`; no Common consumer. |
| `runtime/model40_composition.c`, `runtime/model40_composition.h` | DeskPro Model 40-specific topology and media startup. | T528 S4 move to `vm/profile/model40`; never Common. |
| `runtime/provider_lifecycle.c`, `runtime/provider_lifecycle.h` | Initializes/resets/finalizes NXVM media/device providers. | T528 S4 fold into the owner-local media/profile composition transaction; no generic lifecycle route. |
| `runtime/rom/external_pc_at.c`, `runtime/rom/external_pc_at.h` | External PC/AT firmware mapping into Core. | T528 S4 move to NXVM profile/BYOB composition; never Common. |
| `runtime/runner.c`, `runtime/runner.h` | VM-specific Core run loop, pause/reset checks, debug plan, idle/time advance and completion. | T529 receiver: its generic executor/wake/actual completion mechanics are required by SoftPC `app/runtime`, but Core run/clock/debug callbacks remain NXVM adapter code. Do not delete in T528 S1. |
| `runtime/waiting.c`, `runtime/waiting.h` | Standard pacing and HLT/deadline advancement against Core time. | Retain NXVM Core-time adapter; only generic event wait belongs to Lib/Common. T529 must not absorb guest-time/profile pacing. |

## Required Cutover Order

1. T528 S2 removes duplicate request/state entry routes while leaving one
   explicit Core adapter and records every retained executor dependency.
2. T528 S3 removes VM-owned product output/debug state while retaining only
   copied Core result and debug target adaptation.
3. T528 S4 moves profile/media/firmware composition to NXVM-local owners and
   leaves the residual neutral executor mechanism as T529's finite receiver.
4. T529 first proves both NXVM and SoftPC consumers, then migrates that residual
   executor mechanism and deletes the old VM implementation in one cutover.

## Similar-Issue Sweep Result

The source query found one legacy VM control cluster (`control`,
`executor_state`, `lifecycle`, `runner`) and one Common FIFO/lease cluster.
No second Common or Lib presentation/mailbox implementation exists; Core-frame
copying is an NXVM-specific adapter, not a duplicate. No profile/media/ROM
code has a SoftPC-equivalent Common claim. Existing tests that reach private
`session->control` or `session->executor` are migration consumers and must be
updated or deleted with the production-path cutover; they cannot preserve a
second ownership route.
