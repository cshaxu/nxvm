# M5 Platform Run-Handle Contract

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

## T138--T142 Sequence

| Task | Scope | Gate |
| --- | --- | --- |
| T138 | Approve this run-handle/lifetime contract and map current Win32/Linux owners. | design review; no code or artifact |
| T139 | Implement Win32 handle creation, request-stop, join, and sole teardown owner. | Console/window lifecycle and FDD DOS-prompt gates |
| T140 | Give Linux Console the equivalent contract and static-source checks. | source-level parity checks; native POSIX build deferred |
| T141 | Make core machine own its standard executor and shared-device lifecycle. | core/VM/VDM lifecycle and boot gates |
| T142 | Replace VM raw core alias maps with bounded composition/debug access. | two-session, debugger, Console, and FDD DOS-prompt gates |

Historical M5 T137 already completed the former generic execution-context
rename and move into `vm/composition`; it is not repeated in this sequence.

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
