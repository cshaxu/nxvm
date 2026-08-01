# M5 T10: Core Execution Carrier Migration

## Decision

The actual full-PC execution carrier is the baseline chain below, not the M3
lifecycle scaffold:

```text
machineStart/reset/resume
  -> platformStart
  -> deviceStart
  -> vmachineRefresh
  -> vcpuRefresh
  -> vcpuinsRefresh
```

T10 moves this carrier by `git mv` in small buildable slices. It preserves the
original ordering, exported Console/debugger behavior, reset vector, and
single-machine execution semantics. No new executor is written alongside it.

## Source Slices

| Subtask | Exact source move | Temporary boundary | Gate |
| --- | --- | --- | --- |
| S2 | `machine.c/.h` to `machine/vm`; `device.c/.h` and `vmachine.c/.h` to `machine/vm` | Baseline device headers may be forwarding includes only while unmoved callers still use their old paths. The user-facing target compiles the moved implementations. | GCC build; no-media `help/info/exit`; `debug/q/exit`; reset-vector adapter smoke with supplied fixture arguments. |
| S3 | `vcpu.c/.h`, `vcpuins.c/.h`, `vram.c/.h`, `vport.c/.h`, and `vglobal.h` to `machine/core` | Preserve the legacy globals only as a documented single-session binding while dependent controllers remain unmoved. No duplicated CPU/RAM/port implementation. | Existing CPU probes; finite one-instruction adapter probe; reset vector; no-media Console/debugger checks. |
| S4 | Replace the temporary binding at the moved carrier boundary with an explicit session-owned PC/AT execution context and callback table. | Remaining controller code may use compatibility accessors only; no platform thread obtains guest-state access. | Two sequential session reset/probe runs; bounded stop/reset/resume; no leaked execution thread; CPU probes. |
| S5 | Remove T10 carrier use of baseline implementations and make `nxvm-m5_t10.exe` use the moved lifecycle, loop, CPU, RAM, and port sources. | Remaining baseline device/controller sources are callers or providers only, pending T11; no baseline source owns the loop or executor. | All S2-S4 gates plus retained Console/debugger regression and recorded FDD/HDD checkpoint run when local fixtures are available. |

`src/core` is the historical M3 contract scaffold. It is not a second executor.
Before S3 completes, its contract headers and the moved implementation headers
must be reconciled under `src/machine/core`; the task may not leave two active
CPU, RAM, port, or lifecycle implementations.

## State And Threading Boundary

The moved implementations initially retain their exact NXVM logic and ordering.
During S3, a short-lived single-session binding is permitted only inside the
execution thread so unmoved controllers can reach the moved state. S4 replaces
that bridge with the session-owned context. This bridge is neither a platform
API nor a final global execution path, and it must be deleted before M5 closes.

`platform` continues to create or join the execution thread according to the
retained NXVM Console/window behavior. It may request lifecycle actions but
cannot read or mutate CPU, memory, ports, or controller state directly.

## Regression And Stop Rules

- Every source slice must compile the user-facing `nxvm-m5_t10.exe` target from
  the moved source paths; CMake source-set inspection is evidence.
- Scripted no-media checks retain `help`, `info`, `exit`, and `DEBUG -> - -> q
  -> Console>` output markers. Any difference stops for owner approval.
- CPU probes remain finite. Recorder traces are prohibited unless a later
  subtask declares the execution-policy budgets and cleanup owner.
- FDD/HDD runs use only the recorded local fixture paths and budgets. Missing
  local fixtures block that fixture gate, not the no-media source-move gate.
- A move that requires a Console/window/input/media/boot behavior change stops
  for explicit owner approval before implementation.

## T10 Exit

T10 completes only when the user-facing T10 executable's lifecycle, refresh
loop, CPU instruction dispatch, RAM, and port sources all live in
`machine/vm` or `machine/core`; the baseline adapter cannot own any of those
implementations. T11 owns controller and firmware movement.
