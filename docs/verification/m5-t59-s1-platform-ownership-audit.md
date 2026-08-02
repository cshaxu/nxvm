# M5 T59 S1 Platform Ownership Audit

## Decision

`win32app`, `win32con`, `linuxcon`, and the future `linuxapp` remain the
preferred symmetric host-surface names. T59 does not move or rename platform
source files. Ownership is decided by dependencies and policy content, not by
filename style.

## Current Shared Core Mechanisms

| File | Owner | Reason |
| --- | --- | --- |
| `src/core/platform/display_frame.*` | `core/platform` | Product-neutral copied display snapshot. Producers publish and host adapters capture; it contains no VM Console, execution, or profile policy. |
| `src/core/platform/sleep.h` | `core/platform` | Product-neutral wait interface. |
| `src/core/platform/win32/sleep.c` | `core/platform/win32` | Win32 implementation of the shared wait interface. |
| `src/core/platform/linux/sleep.c` | `core/platform/linux` | Linux implementation of the shared wait interface. |
| `src/core/platform/presentation.h` | `core/platform` | Product-neutral presentation/input event type placeholder. Its public names still need future naming cleanup if it grows. |

## VM Platform Ownership Map

| File | Current owner | Classification |
| --- | --- | --- |
| `src/vm/platform/platform.*` | `vm/platform` | VM platform hub. It chooses console/window mode, initializes the shared display snapshot, and dispatches to the active VM host surface. |
| `src/vm/platform/execution.*` | `vm/platform` | VM execution callback boundary. It binds host loops to VM composition-owned run/stop/flip behavior. |
| `src/vm/platform/input.*` | `vm/platform` | VM keyboard callback boundary. It translates host input into VM keyboard state and stop requests. |
| `src/vm/platform/request_bridge.*` | `vm/platform` | VM request FIFO. Its payload is VM keyboard and stop semantics. |
| `src/vm/platform/vm_request_transport.*` | `vm/platform` | VM execution-boundary transport. It observes VM run boundaries and dispatches VM request payloads. |
| `src/vm/platform/win32/win32.*` | `vm/platform/win32` | Win32 VM adapter hub. It maps Win32 keyboard state into VM keyboard modifiers and dispatches display/start by VM surface mode. |
| `src/vm/platform/win32/win32app.*` | `vm/platform/win32` | Win32 window host surface for VM. It owns window/thread policy and calls VM execution/input/display adapters. |
| `src/vm/platform/win32/win32con.*` | `vm/platform/win32` | Win32 console host surface for VM. It owns console polling/thread policy and calls VM execution/input/display adapters. |
| `src/vm/platform/win32/w32adisp.*` | `vm/platform/win32` | Win32 window renderer currently bound to the VM window surface globals and the copied display snapshot. |
| `src/vm/platform/win32/w32cdisp.*` | `vm/platform/win32` | Win32 console renderer currently bound to the VM console surface and the copied display snapshot. |
| `src/vm/platform/linux/linux.*` | `vm/platform/linux` | Linux VM adapter hub. It dispatches to the retained Linux console surface. |
| `src/vm/platform/linux/linuxcon.*` | `vm/platform/linux` | Linux console host surface for VM. It owns ncurses rendering, input polling, pthread startup, and VM execution/input callbacks. |

## T60 Eligibility

No entire VM platform file is safe to migrate in T60. The only proven
policy-free mechanisms are already in `core/platform`: sleep and copied display
snapshot storage.

Potential future extractions require a design task before source movement:

- Win32 keyboard state probing could become `core/platform/win32` only after
  its VM key-code mapping and stop semantics are separated.
- Win32 console and window rendering helpers could become
  `core/platform/win32/{win32con,win32app}` only after they receive explicit
  host-surface context instead of using VM window/console globals.
- Linux ncurses rendering and polling could become `core/platform/linux` only
  after VM execution/input callbacks are separated from host-loop mechanics.

## Verification

- Source audit used `rg` over `src/vm/platform` and `src/core/platform`.
- No platform source file was moved or renamed in T59.
- Follow-up build and source-DAG evidence is recorded in the T59 tracking file.
