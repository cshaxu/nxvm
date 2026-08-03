# M5 T127 S1: Unique Owner And Execution-path Audit

## Scope And Rule

This is an independent source/CMake audit after T119--T126. A runtime state
has one owner; production startup and execution have one route; test helpers
do not define a product route; and an unconsumed descriptor, registry,
selector, or metadata wrapper is debt, not an architecture feature. Evidence
below is source-level. Thread and host-window scheduling still require runtime
validation on each supported host.

| Module | State; create/destroy | Run/reset/refresh; providers | Static/TLS; extra path/wrapper | Verdict and evidence |
| --- | --- | --- | --- | --- |
| `core/machine` | One heap `core_machine`; `core_machine_create/destroy` | `core_machine_reset/run`; execution provider refreshes it | `test_*` fields are test-profile-only and `run` rejects non-executor | P2: one production executor, but two structural representations remain by declared test mode. [machine.c](../../src/core/machine/machine.c#L139), [machine.c](../../src/core/machine/machine.c#L242), [machine.h](../../src/core/machine/machine.h#L52) |
| `vm/machine` | Device storage belongs to `vm_session`; core objects are borrowed, not copied | session lifecycle initializes/resets/finalizes providers | no module-global live device state | No issue. [session.c](../../src/vm/composition/session.c#L48), [providers.c](../../src/vm/composition/providers.c#L49) |
| `vdm/machine` | `vdm_machine_dos_minimal` owns one test-minimal `core_machine` | create/reset; no production run loop | no static mutable state; only VDM tests call it | P2: intentional skeleton, not a second NXVM route. [dos_minimal.c](../../src/vdm/machine/dos_minimal.c#L100), [dos_minimal.c](../../src/vdm/machine/dos_minimal.c#L147) |
| `core/platform` | Callers own surface contexts; lease type stores only an atomic owner token | no product runner | no mutable singleton in core platform | No issue. [host_surface_interface.c](../../src/core/platform/host_surface_interface.c#L14) |
| `vm/platform` | `vm_platform_run_context` is embedded in `vm_session` | `vm_platform_start` receives that context; transports are callbacks into session | one static OS Console/terminal lease per backend | P2: permitted physical-host lease, but two-session contention needs host runtime coverage. [session.h](../../src/vm/composition/session.h#L82), [win32con.c](../../src/vm/platform/win32/win32con.c#L17), [linuxcon.c](../../src/vm/platform/linux/linuxcon.c#L212) |
| `vdm/platform` | none | none | no source module or product path | P2: absent by design before M8/M9, not a hidden path. [CMakeLists.txt](../../CMakeLists.txt#L519) |
| `core/product` | debug contexts are caller-owned; execution context is in VM control | explicit debug target operations; wait scope selected at platform run | TLS debug parser, assembler/disassembler parser, and wait scope | P1: no public current-target API remains, but runtime helper bodies still rely on TLS. [debug.c](../../src/core/product/debug/debug.c#L18), [debug.c](../../src/core/product/debug/debug.c#L1928), [wait.c](../../src/core/product/wait.c#L7) |
| `vm/product` | Console context is embedded in `vm_session` | `main` enters one Console with explicit context/target | TLS `consoleContext` routes the body | P1: restore-safe but still an implementation current-object dependency. [console.c](../../src/vm/product/console.c#L17), [console.c](../../src/vm/product/console.c#L402) |
| `vdm/product` | none | none | no source module or product entry | P2: intentionally unimplemented, not a duplicate UI. [CMakeLists.txt](../../CMakeLists.txt#L522) |
| `vm/composition` | `vm_session` owns core, VM devices, profile context, transports, platform, debug and Console contexts | `vm_session_create/initialize/reset/resume`; runner alone calls `core_machine_run` | no forwarding `full_pc/default_profile/session_model` objects remain | No issue. [session.c](../../src/vm/composition/session.c#L176), [session_lifecycle.c](../../src/vm/composition/session_lifecycle.c#L191), [session_runner.c](../../src/vm/composition/session_runner.c#L11) |
| `vdm/composition` | `vdm_session` owns exactly one minimal DOS object | reset and presentation only delegate through session | no descriptors/registry fields | No issue within skeleton. [session.c](../../src/vdm/composition/session.c#L10), [presentation.c](../../src/vdm/composition/presentation.c#L23) |
| `vm/profile`, `vdm/profile` | VM default firmware storage is in `vm_session`; VDM has no runtime profile | provider callbacks receive profile context | immutable default firmware descriptor only; T125 deleted unused profile registry/descriptors | No issue. [session.c](../../src/vm/composition/session.c#L97), [default_profile.c](../../src/vm/profile/default_profile/firmware/default_profile.c#L11) |
| CMake, mains, tests | NXVM has one `src/vm/main.c` executable route | `main -> vm_session_create -> Console`; guest execution is runner -> core run | test binaries have `main`, but all are CMake smoke targets; no VDM executable exists | P2: VDM must add exactly one entry only when its product begins. [main.c](../../src/vm/main.c#L26), [CMakeLists.txt](../../CMakeLists.txt#L553), [CMakeLists.txt](../../CMakeLists.txt#L531) |

## Paths

```text
NXVM product: src/vm/main.c
  -> vm_session_create / vm_session_initialize
  -> vm_product_console_main
  -> START/RESUME callback -> vm_platform_start
  -> VM platform kernel callback -> vm_session_runner_run
  -> core_machine_run -> executor CPU context

VDM skeleton: ntvdm64-vdm-minimal-session-smoke
  -> vdm_session_create -> vdm_machine_dos_minimal_create
  -> test-minimal core_machine; reset/input/text snapshot only

Test-only: core contract/executor smokes, VM CPU probe support, VDM presentation
smokes. None is linked as the NXVM product executable.
```

## Findings

**Confirmed facts.** T125 removed the descriptor/registry layer and its sole
tests; no production composition source includes it. T126 removed empty VM
lifecycle facades. The VM formal graph has one session root and its runner is
the only production caller of `core_machine_run`.

**P1 architecture debt.** `core_product_debug_main`,
`vm_product_console_main`, and `core_product_wait_milliseconds` select their
otherwise explicit caller-owned context through TLS. Nesting restores the
previous pointer, and no selected VM/session pointer is exposed publicly, but
the implementation is still non-reentrant within a thread and violates the
strict no-TLS runtime-dependency goal. A follow-up must pass an invocation
state through debug/Console helpers and make wait a supplied operation rather
than ambient scope. It must retain Console grammar, debugger behavior, and
platform timing.

**P2 gaps.** A VDM minimal object intentionally uses the core test profile and
has no executable product route; it must not become M6/M7's production
executor path without an explicit design decision. The per-backend host lease
is correctly atomic and process-exclusive, but the current gates do not drive
two interactive host surfaces concurrently. Verify rejection/release on Win32
and POSIX before treating it as fully closed.

**Only naming/history.** Historical M0--M4 documents retain old
descriptor/registry terminology. They are history, not active source policy;
T125's removal is the current behavior.

## Required Follow-up Order

1. Design explicit invocation-state propagation for debug, Console, and wait;
   add nested and parallel-session regression gates before modifying code.
2. Remove the TLS selections one component at a time; preserve Console,
   debugger, startup, and DOS-prompt gates after each change.
3. Add Windows and POSIX host-surface contention/release observations.
4. Before M6, decide whether VDM replaces its test core with the production
   executor or remains a non-executing contract fixture until M7.

Verification: `cmake --build --preset nxvm-current-gates-gcc` is the source
and behavior gate. The artifact is `build/output/nxvm_0_5_0127.exe`
(`DA276BCDDAB140BCA2BADB42C11AFD8C253E963865BC1A20834EFBA2F7486F6B`).
