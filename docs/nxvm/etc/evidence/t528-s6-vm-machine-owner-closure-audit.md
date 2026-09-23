# M5 T528 S6 VM-Machine Owner Closure Audit

Baseline: `36d51a29` after accepted S5. This audit covers every tracked
production C/header below `src/vm/machine`, its direct owner includes and CMake
source rows. It does not claim a shared executor worker: NXVM runs bounded Core
quanta; SoftPC retains its CCPU/timer rendezvous runtime.

## Removed Paths

| Former path | Final disposition | Proof of non-duplication |
| --- | --- | --- |
| `vm/machine/request_interface.h` | `vm/request_interface.h` | The immutable copied YAML/catalog-to-app request is used by both product catalog and app composition, but never by machine. VM-root value contract avoids peer dependency in either direction. |
| `runtime/execution.c`, `runtime/execution.h` | Deleted | Its only state was a `vm_machine *`, two fixed callbacks and a command-boundary callback. The runner now directly retires the one `common_machine` FIFO at its own safe points; reset/debug helpers are owner-local `control` operations. |
| `vm-execution-context-smoke` | Renamed `vm-control-lifecycle-smoke` | The retained external-media scenario proves control start/reset/stop behavior, not a deleted context object. The direct diagnostic probe likewise retires the Common FIFO directly. |

## Exhaustive Retained Corpus

| Files | Single responsibility | Boundary / reason retained |
| --- | --- | --- |
| `debug.c`, `debug.h` | Core execution-plan completion and copied instruction observation. | NXVM Core target state; Common Debug owns grammar and Common machine owns the paused lease. |
| `runtime/control.*`, `runtime/executor_state.*`, `runtime/runner.*` | One NXVM bounded-quantum Core runner and its local execution state. | The runner alone calls `core_machine_run()` and consumes the Common FIFO only at command boundaries. This is not SoftPC's continuous CCPU/timer loop and has no second Core path. |
| `runtime/lifecycle.*` | Binds the Common driver, starts/stops the one NXVM host task, maps Core guest input, and publishes copied lifecycle facts. | Common owns copied requests/generation/wake/lease; NXVM owns the Core task and its concrete safe points. |
| `runtime/waiting.*` | Core guest-time deadline advancement and Standard pacing. | Core-time-specific policy; generic host waits remain only in Lib host. |
| `runtime/debug_adapter.*` | Bounded Common paused-Debug operations to Core debug API. | Sole NXVM target adapter; it exposes neither a Core pointer nor a second Debug grammar. |
| `runtime/display.*`, `runtime/frame.*`, `runtime/event_interface.h` | Core snapshot capture and copied result/frame conversion. | Common UI/Lib own presentation/mailboxes; this is the one Core-to-copied-value adapter. |
| `runtime/fault.*`, `runtime/machine_info.c` | Core fault/information to copied VM facts. | Product formats text; machine only publishes values. |
| `runtime/machine.*`, `runtime/machine_interface.h`, `runtime/machine_private.h` | Frozen profile/assets/media assembly into one Core plan and VM adapter ABI. | Sole Core construction/asset-copy transaction; no YAML parser, native UI or separate provider route remains. |
| `runtime/machine_devices.*` | Generic FDD/HDD provider binding and plan materialization. | Device composition owns one media registry route; FDD/HDD mechanics are in `vm/media`. |
| `runtime/model40_composition.*` | Model 40's state-coupled FDD/HDD/D4 plan materialization. | It mutates the one `vm_machine` plan/state; moving to profile would create a profile-to-machine reverse dependency. |

## Similar-Issue Sweep

The audit searched all retained sources, tests, CMake source rows and direct
project includes for former execution-context, command-boundary, request-value,
provider-lifecycle, media, firmware, presentation/mailbox and peer-owner
paths. The only production hits were the two removed categories above. Core
CPU execution-context terms are a distinct Core CPU implementation and are
outside this VM facade class.

## Verification

- Focused: `vm-machine-executor-state-smoke` and
  `vm-keyboard-host-ingress-smoke` passed.
- `verify-dependency-dag` passed after removal of the now-stale
  `vm/product/catalog.h|vm/machine` allowlist edge.
- Full repository-only unit suite: **299/299 passed** with `ctest --parallel 8`.

The task-level external integration, dual-architecture artifact and final
closure audit remain required before T528 can close; this S establishes the
complete post-S5 owner corpus and no generic executor receiver.
