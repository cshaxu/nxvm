# M5 T138--T142: Unique-Owner Closure Audit

## Scope And Exit Result

This is the final audit required by the M5 unique-owner exit standard in
`docs/planning/m5-platform-run-handle-contract.md`. It confirms the following
for the active NXVM path:

| Exit condition | Result | Evidence |
| --- | --- | --- |
| One standard core-machine creation path per product session | Pass | `src/vm/composition/session.c:60`; `src/vdm/machine/dos_minimal.c:114`; `src/core/machine/machine.c:85` |
| One live platform-run owner | Pass | `src/vm/composition/session.h` embeds `vm_platform_run_handle_storage`; T139 evidence and run-handle smoke |
| No TLS/current-object execution selection | Pass | final source scan below; explicit invocation work is recorded through T129--T132 |
| Core has no VM/VDM dependency | Pass | final source scan below |
| No VM raw alias map for core internals | Pass | `src/vm/composition/machine_access.h`; final source scan below |
| No active second VM session/machine execution path | Pass | T118 removal record; real-session tests and final two-session smoke |

"One creation path" means that both product forms call the same unconditional
`core_machine_create()` contract. VM and VDM correctly create distinct machine
instances for distinct sessions; it does not require them to share an instance.

## Creator, Worker, And Destroyer Map

| Resource | Creator | Worker/reporting role | Sole finalizer/destroyer |
| --- | --- | --- | --- |
| Core CPU/RAM/port and PIC/PIT/DMA/KBC/VADP | `core_machine_create()` | `core_machine_run()` drives the standard executor | `core_machine_destroy()` |
| VM `core_machine` | `vm_session_storage_initialize()` | VM execution uses bounded composition access | `vm_session_storage_finalize()` then `core_machine_destroy()` |
| VM platform run handle | `vm_platform_start()` for a session-owned embedded handle | kernel/display report completion or stop only | outer session lifecycle requests stop, joins workers, finalizes host surface, then finalizes handle |
| Console/debug context | VM session storage | command boundary / debugger target | VM session finalization after no live run handle remains |
| VDM minimal machine | `vdm_machine_dos_minimal_initialize()` | no runnable DOS path is claimed | VDM minimal finalization |

The VM access record in `src/vm/composition/machine_access.c` owns no guest
storage and exposes no lifecycle operation. It is the only VM source that
borrows core executor/shared-device objects; each accessor returns a fresh
non-owning borrow.

## Final Source Audit

The following scans ran against the active source tree after the T142 build:

| Prohibited pattern | Result | Notes |
| --- | --- | --- |
| `#include "vm/"` or `#include "vdm/"` beneath `src/core` | Empty | Core does not depend on product forms. |
| TLS current session/machine/active selection | Empty | No `_Thread_local` execution-selection facade remains. |
| VM session raw CPU/RAM/port/PIC/PIT/DMA/KBC/VADP access | Empty | VM reaches core through the bounded access record. |
| VM/VDM direct core executor lifecycle calls | Empty | Core owns prepare/reset/refresh/finalize. |
| Detached platform workers or legacy `start_machine` lifecycle | Empty | Run-handle ownership is the active platform model. |
| Profile-selected executor / `core_machine_enable_executor()` | Empty | `core_machine_create()` always creates the standard executor. |

`CMakeLists.txt` still names `tests/products/nxvm_full_pc_session_smoke.c`.
This is a retained test filename only; the former full-PC wrapper implementation
and product route were removed by T118. It is not a second runtime path.

## Windows GCC Verification

`cmake --preset mingw-gcc-x64` and the following focused executables built
successfully after the T142 change:

```text
nxvm-0-5-0142
nxvm-vm-platform-run-handle-smoke
nxvm-core-executor-run-smoke
nxvm-core-machine-lifecycle-smoke
ntvdm64-vdm-minimal-session-smoke
nxvm-vm-core-executor-storage-smoke
nxvm-vm-two-session-isolation-smoke
nxvm-vm-debug-target-smoke
nxvm-vm-debug-pause-boundary-smoke
nxvm-vm-dos-prompt-smoke
nxvm-product-console-lifecycle-gate
```

The runtime suite passed with these markers:

```text
M5:T139:S1:RUN-HANDLE:OK
M5:T83:S3:CORE-EXECUTOR-RUN:OK
M3:T2:S1:LIFECYCLE:OK
M5:T94:S1:VDM-SESSION-ISOLATION:OK
M5:T83:S2:CORE-EXECUTOR-STORAGE:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T14:S3:VM-DEBUG-TARGET:OK
M5:T45:S1:PAUSE-BOUNDARY:OK
M5:T70:S2:DOS-PROMPT:OK
```

The task artifact is `build/output/nxvm_0_5_0142.exe`, banner `0.5.0142`,
SHA-256 `A8617DA8E203D048DEE0BE907649C92B91815CE1AFF11DEFF7E18606B4895A5C`.

## Environment Limit

T140 provides Linux source-level run-handle parity. Native POSIX compilation
and runtime verification remain intentionally deferred to the designated POSIX
environment. This is the only platform-specific verification limit in this
closure; it is not a claim that Linux behavior has run on Windows.

## Conclusion

T138--T142 satisfy the M5 unique-owner closure standard. This closes the
identified historical split between core storage and lifecycle, platform worker
and teardown ownership, and VM raw aliases to core state without changing the
retained NXVM Console, debugger, startup flow, or FDD DOS-prompt behavior.
