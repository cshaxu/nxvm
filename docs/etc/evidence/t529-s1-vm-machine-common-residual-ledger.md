# M5 T529 S1 VM-Machine/Common Residual Ledger

Baseline: `cab545e8`. This is a source-only, two-consumer audit. It covers all
29 tracked C/header files in `src/vm/machine`, the current
`src/common/machine` contract, and the read-only SoftPC corpus at
`O:/repos.hobby/softpc`. No SoftPC source is imported and no NXVM runtime code
changes in this subtask.

## Frozen Comparison Rules

A row may enter Common only if NXVM and SoftPC need the same neutral state,
value contract, lifecycle, failure boundary and safe-point rule. Similar names
or both products using a thread, queue, event or frame are insufficient.

The permitted dispositions are:

- **Existing Common route:** NXVM already uses the sole Common capability.
- **Distinct NXVM adapter:** Core/profile/asset/timing behavior that cannot
  enter Common.
- **No shared consumer:** SoftPC has no corresponding capability or has a
  different execution contract.
- **Local removal candidate:** dead or forwarding-only NXVM code; it is not a
  reason to expand Common.

## Complete NXVM Corpus

| Files | Owned responsibility | Common / SoftPC comparison | Disposition |
| --- | --- | --- | --- |
| `debug.c`, `debug.h` | Core instruction observation and execution-plan completion. | Common carries only the copied Debug request; SoftPC has no paused-Debug consumer. | Distinct NXVM adapter; no shared consumer. |
| `runtime/control.c`, `control.h` | Core runner's active/reset/pause/step acknowledgement and completion signal. | Common FIFO delivers requests but does not own actual executor state. SoftPC's `app_runtime` has CCPU-specific start/reset/heartbeat state rather than this state machine. | Distinct NXVM adapter, except the two local removals below. |
| `runtime/executor_state.c`, `executor_state.h` | Atomic state read by the bounded Core runner at its own safe points. | SoftPC's runtime state includes CCPU worker/callback rules; Common intentionally has no executor state. | Distinct NXVM adapter. |
| `runtime/lifecycle.c`, `lifecycle.h` | One Core host task; Common-driver bind; request-to-Core mapping; lifecycle facts. | It already submits copied requests to Common. SoftPC must instead enter CCPU TLS, bind its timer callback and heartbeat. | Existing Common route plus distinct NXVM adapter. |
| `runtime/runner.c`, `runner.h` | Bounded Core quantum, Core result/fault/deadline handling and Core-safe request retirement. | SoftPC enters continuous `c_cpu_simulate()` and reaches safety through its timer/executor callback. | Distinct NXVM adapter; no common executor candidate. |
| `runtime/waiting.c`, `waiting.h` | Core time observation, source-qualified deadline advance and Standard pacing. | SoftPC runtime does not consume NXVM Core time observations. | Distinct NXVM timing adapter. |
| `runtime/debug_adapter.c`, `debug_adapter.h` | Bounded Common Debug lease operations mapped to Core's opaque Debug API. | SoftPC currently has no Debug target. | Existing Common lease route plus distinct Core adapter. |
| `runtime/display.c`, `display.h`, `frame.c`, `frame.h`, `event_interface.h` | Core display/input values converted to copied Common/UI facts. | Common UI owns presentation; SoftPC's copied frame is produced from original C-VID surfaces with different state and capture rules. | Distinct NXVM Core adapter. |
| `runtime/fault.c`, `fault.h`, `machine_info.c` | Core fault and machine information become copied VM results. | Common carries generic facts but does not know Core fault/info layout; SoftPC has its own runtime outcome. | Distinct NXVM Core adapter. |
| `runtime/machine.c`, `machine_interface.h`, `machine_private.h` | Frozen profile/assets/media transaction, Core plan construction and Common driver binding. | SoftPC builds original MVDM/CCPU hardware and cannot consume this Core plan. | Distinct NXVM composition adapter. |
| `runtime/machine_devices.c`, `machine_devices.h` | Frozen FDD/HDD Core-plan and media-registry assembly. | SoftPC attaches original FDC/HDD mechanisms and owns a different media binding. | Distinct NXVM composition adapter. |
| `runtime/model40_composition.c`, `model40_composition.h` | Selected DeskPro board topology, D4, FDC/HDC and ROM-specific assembly. | No SoftPC counterpart and forbidden profile-to-Common dependency. | Distinct NXVM board adapter. |

## Shared-Mechanism Reconciliation

| Mechanism | NXVM evidence | SoftPC evidence | Result |
| --- | --- | --- | --- |
| Copied request ingress, run identity, wake and safe-point delivery | `common_machine_submit`, `bind_run`, `observe_safe_point`, `wait` in `common/machine`; `vm_machine` binds the Core driver. | `app_input_queue` carries only `ui_event`; `app_runtime` separately owns lifecycle commands and asks CCPU for another callback. | NXVM is already on the sole Common route. SoftPC's split input/control queues and CCPU callback rule are not the same contract; no duplicate in `vm/machine`. |
| Actual executor lifecycle | `vm_machine_executor_state` plus `runner` owns bounded Core `run()` quanta and pause acknowledgement. | `app_runtime` owns worker, CCPU TLS entry/leave, heartbeat, timer callback, media event and continuous `c_cpu_simulate()`. | Different execution owners; Common must not acquire a generic worker. |
| Paused Debug lease | `common_machine_debug_acquire`/`execute_with_lease` dispatches to `debug_adapter`, then Core. | No SoftPC Debug target/consumer. | Existing Common contract with one current product adapter; not a two-consumer expansion candidate. |
| Frame publication | NXVM copies Core VADP snapshots before reporting a fact to Common Session/UI. | SoftPC copies original C-VID text/DIB buffers under a double-buffer lock and publishes a separate sequence. | Both are product adapters, not Common-machine state. |
| Host events and tasks | NXVM uses `lib/host` directly for its Core task and completion observation. | SoftPC also uses Lib host synchronization for its worker/event set. | Already shared by Lib; not a Common-machine duplicate. |

## Proven Local Cleanup Batch

The complete caller sweep found two residual dead controls, both confined to
`runtime/control.*`:

| Item | Evidence | Required receiver |
| --- | --- | --- |
| `control_changed` | Created, signalled, destroyed and nulled, but has no wait/read caller anywhere in `src/` or `test/`. | Remove the field and its lifecycle calls in a bounded NXVM-local S2. |
| `vm_machine_control_wait_for_completion()` | Declared and implemented, but has no caller outside its declaration/definition. | Remove the API and implementation in the same NXVM-local S2. |

`completion_ready`, pause acknowledgement, step state and pause reason remain
live: the runner, lifecycle and focused Debug/firmware tests use them. They are
not a second Common queue or lifecycle reducer.

## Completion Predicate

All 29 tracked VM-machine files are classified. No existing VM-machine/Common
duplicate and no proven two-consumer Common expansion batch exists. The only
S2 batch is the two-item local-dead-control deletion above; it must not add a
Common API or change the NXVM/SoftPC executor boundary.
