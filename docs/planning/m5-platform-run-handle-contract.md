# M5 Platform Run-Handle Contract

## Status And Authority

This is the normative M5 design and closure reference for a VM platform run.
T138 defined the contract; T139 implemented the Win32 shape and T140 aligned
the Linux Console source shape. The remaining ledger item is not permission to
introduce another lifecycle model: it requires continuing evidence that every
current or newly admitted backend obeys this contract. `TODO.md` links here for
the exact remaining verification scope.

## Purpose

`win32app`, `win32con`, and their Linux equivalents must not let a kernel or
display thread release the shared platform context. A platform run is one
session-owned resource with a single teardown authority.

## Ownership

`vm_session` owns one `vm_platform_run_handle`. `vm_platform_start` creates
and starts it for an already initialized `vm_platform_run_context`, but does
not transfer ownership to either worker thread. The handle owns:

- the kernel and display thread handles;
- the stable `vm_platform_run_context` reference used by those threads;
- renderer and native window/Console lifecycle state; and
- cancellation and completion state.

The run context remains valid from successful handle creation until the outer
session lifecycle destroys the handle after both workers have joined.

## Worker Rules

- The display worker may report window close or request cancellation. It never
  frees a renderer, host-surface handle, run context, or run handle.
- The kernel worker may report guest stop, fault, or completion. It never
  frees a renderer, host-surface handle, run context, or run handle.
- Neither worker finalizes `vm_session`, providers, firmware, or `core_machine`.
- The outer lifecycle is the only joiner and destroyer.

## Cancellation And Teardown

All stopping initiators first enter the same idempotent lower-level sequence:

```text
request stop -> wait kernel exit -> wait display exit
             -> destroy renderer/window or Console binding
             -> release run handle -> permit session finalization
```

The initiator determines the product outcome after that sequence:

- Console `STOP` returns the retained Console to its stopped/idle state.
- Console `EXIT` proceeds to session destruction after stop/join.
- Window close requests controlled cancellation, then proceeds to session
  destruction and its defined result.
- Guest completion follows the same join path before returning control.

No caller may invoke `vm_session_finalize` while a run handle is live. Failed
thread creation follows the same cleanup order for every successfully created
worker and renderer resource.

## Historical T138 Baseline

The following table records the pre-T139 ownership problems that motivated the
contract. It is historical evidence, not a description of the permitted
current implementation.

| Path | T138 creator | T138 worker behavior | T138 join / destroyer | T139/T140 correction |
| --- | --- | --- | --- | --- |
| `win32app` | `vm_platform_win32app_start_machine` heap-allocates `win32app_run_context`. | Kernel starts guest; display creates and destroys the window renderer, stops guest, clears the shared run context, and frees the shared run context. | No retained thread handles or outer joiner. | Move both thread handles and all shared state into the session-owned run handle; display only reports close/stop. |
| `win32con` | `vm_platform_win32con_start_machine` heap-allocates `win32con_run_context`. | Kernel starts guest; display paints until stop. | Backend itself waits, closes thread handles, destroys renderer, frees context, and releases lease. | Let the run handle/lifecycle own join, teardown, and lease release; backend workers only report completion. |
| `linuxcon` | `lnxcStartMachine` uses the embedded run context directly. | Both pthreads are detached; display finalizes curses and releases terminal lease. | No joiner; partial display-start failure releases the lease while detached kernel can still run. | Add handle-owned joinable pthreads; move curses/lease teardown to the outer destroyer. |
| `vm_session_finalize` | Console target may call it directly. | It detaches providers and destroys session storage without a platform-run guard. | Session lifecycle is the current finalizer but cannot prove workers stopped. | Require run-handle stop/join/destroy before finalization. |

## Implemented T138--T142 Sequence

| Task | Scope | Gate |
| --- | --- | --- |
| T138 | Define this run-handle/lifetime contract and map the former Win32/Linux owners. | Complete: design review; no artifact |
| T139 | Implement Win32 handle creation, request-stop, join, and sole teardown owner. | Complete: Console/window lifecycle and FDD DOS-prompt gates |
| T140 | Give Linux Console the equivalent contract and static-source checks. | Complete: source-level parity checks; native POSIX build deferred |
| T141 | Make core machine own its standard executor and shared-device lifecycle. | core/VM/VDM lifecycle and boot gates |
| T142 | Replace VM raw core alias maps with bounded composition/debug access. | two-session, debugger, Console, and FDD DOS-prompt gates |

Historical M5 T137 already completed the former generic execution-context
rename and move into `vm/composition`; it is not repeated in this sequence.

## Task Exit Standards And Continuing Closure

The open ledger item is satisfied only when the following checks remain true
for every supported backend. A future backend must complete this table before
it is called supported; it may not invent a backend-local teardown sequence.

| Concern | Required rule | Evidence / failure cases |
| --- | --- | --- |
| Handle ownership | The session creates one live run handle and is its only destroyer. A failed creation returns no live handle. | Inspect creator/destroyer paths; fail kernel creation, display creation, renderer/window initialization, and host-surface lease acquisition. |
| Stop request | Console stop/exit, window close, guest completion/fault, and external cancellation reach one idempotent request-stop operation. | Repeat each initiator; a second request must observe the first operation rather than release or restart resources. |
| Worker boundary | Kernel and display workers only report completion, guest result, or stop request. They never free the context, handle, renderer, lease, or session. | Source scan plus normal completion and close-window tests. |
| Join barrier | The outer lifecycle waits for every successfully created worker before any shared context, renderer/window, lease, or session object is released. | Exercise partial startup failure, guest stop, window close, Console exit, and race-order completion. |
| Finalization guard | Session finalize/destroy rejects or first completes a live handle; it cannot detach machine providers while a worker can observe them. | Attempt finalization during a live run and during a failed startup cleanup. |
| Win32 proof | Win32 Console and window preserve existing NXVM Console, debugger, boot, and DOS-prompt behavior. | Focused lifecycle gates, two-session host-surface contention, and FDD DOS-prompt regression. |
| Linux proof | Linux Console/window follows the same source shape and receives native POSIX compilation and runtime evidence before support is claimed. | Native compile, modifier/extended-key input probe, normal stop, close/cancel, and partial-start failure probe. |

The current Windows GCC baseline is evidence for the implemented owner shape;
it is not evidence that Linux runtime behavior has been validated. A failure in
any row reopens the platform run-handle ledger item and must be repaired before
new platform lifecycle features are admitted.

### T138: Run-Handle Design

T138 exits only when the contract names the run-handle owner, creator, worker
event reporters, cancellation requester, joiner, renderer/window destroyer,
and session finalizer; covers normal guest stop, Console `STOP`, Console
`EXIT`, window close, and partial thread-start failure; and preserves the
distinction between Console idle-after-stop and session destruction. It
produces no runnable artifact.

### T139: Win32 Run Handle

T139 exits only when a live run has exactly one session-owned handle; kernel
and display workers free no shared state; every Win32 completion/cancellation
route reaches request-stop then joins both workers before host-surface teardown;
and no session finalizer can run while that handle is live. Gates cover Console
lifecycle, normal guest stop, window close, repeated cancellation, failed
second-thread creation, two-session host-surface contention, and FDD DOS
prompt. It produces a task artifact.

### T140: Linux Parity

T140 exits only when Linux Console source has the same explicit handle,
worker-reporting, request-stop, join, and sole-destroyer structure as Win32;
no Linux worker frees shared run/session state; and static source checks map
every T139 lifecycle event to its Linux equivalent. Native POSIX compilation
and runtime verification are deferred to the designated POSIX environment.
This task produces no Windows artifact unless it changes a runnable Windows
path.

### T141: Core Lifecycle Closure

T141 proceeds in small gates: remove profile/enable selection; establish
unconditional standard core creation; then move core device lifecycle one
device family at a time. It exits only when no `CORE_MACHINE_PROFILE_*`,
`core_machine_enable_executor()`, VM/VDM core-type branch, or VM direct
core-device prepare/reset/refresh/finalize call remains; both products use the
same `core_machine_create -> bind -> freeze -> reset -> run -> destroy` path;
and core executor lifecycle, VDM skeleton, two-session, Console/debugger, and
FDD DOS-prompt gates pass. It produces a task artifact.

### T142: Access Boundary Closure

T142 proceeds by CPU/RAM/port first, then shared devices, then debugger access.
It exits only when `vm_session` has no raw alias map for core internals; VM
composition, firmware, platform, and debugger reach core only through bounded
composition, provider, or debug access contracts; no cached duplicate state or
direct lifecycle bypass remains; and two-session, debugger, Console, and FDD
DOS-prompt gates pass. It produces a task artifact.

### M5 Unique-Owner Closure

M5 may claim its unique-owner objective only after T139 through T142 complete
and a final source-and-runtime audit confirms one core-machine creation path,
one live platform-run owner, no TLS/current-object selection dependency, no
core-to-VM/VDM dependency, no raw core alias map, and no second session or
machine execution path. The final audit records its source scans, creator /
joiner / destroyer map, full focused gate set, and any platform-specific test
environment limits.

## T141 Core Lifecycle Constraint

Earlier M5 work moved live CPU/RAM/port and shared-device storage into
`core_machine` while intentionally retaining device initialization order in VM
composition to protect DOS boot during small migrations. That was a bounded
transition, not a completed boundary: storage ownership without lifecycle
ownership is no longer acceptable after T141.

T141 removes the remaining historical choice of core-machine shape and this
split lifecycle. It is not permitted to introduce a VM core type, a VDM core
type, a second executor, or a new transitional lifecycle facade.

### Core Responsibility

`core_machine` owns:

- CPU, RAM, port, PIC, PIT, DMA, KBC, and VADP storage; and
- their prepare, reset, refresh, and finalize order.

It exposes frozen provider-registration points for product/profile bindings;
it does not acquire VM/VDM policy or firmware implementation.

### VM Composition Responsibility

VM composition only:

- creates one `core_machine`;
- creates CMOS, FDD, FDC, HDD, debug state, and default-profile firmware;
- binds VM providers and profile hooks; and
- freezes configuration before reset/run.

It must never directly prepare, initialize, reset, refresh, or finalize a
core-owned CPU, RAM, port, PIC, PIT, DMA, KBC, or VADP object.

VDM follows the same core creation and binding contract, differing only in its
own provider/profile bindings.

- Delete `CORE_MACHINE_PROFILE_*` and the `profile` member of
  `core_machine_config`.
- Delete the public `core_machine_enable_executor()` selection layer and every
  caller. `core_machine_create()` always constructs the one standard executor.
- `core_machine` owns prepare, reset, refresh, and finalize for its standard
  executor and every shared core device.
- VM and VDM both call the same core creation path. Their only later
  differences are provider registration, firmware/profile callback binding,
  and VM-only or VDM-only device binding.
- The required lifecycle is:

```text
core_machine_create -> bind product providers/firmware -> freeze bindings
                    -> core_machine_reset -> run -> core_machine_destroy
```

- T141 must prove no profile enum or executor-enable API remains, and retain
  core executor lifecycle, VDM skeleton, VM two-session, Console, and FDD
  DOS-prompt regression gates before T142 removes raw aliases.
