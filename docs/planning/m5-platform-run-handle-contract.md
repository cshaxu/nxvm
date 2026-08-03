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

T141 removes the remaining historical choice of core-machine shape. It is not
permitted to introduce a VM core type, a VDM core type, or a second executor.

- Delete `CORE_MACHINE_PROFILE_*` and the `profile` member of
  `core_machine_config`.
- Delete the public `core_machine_enable_executor()` selection layer and every
  caller. `core_machine_create()` always constructs the one standard executor.
- `core_machine` owns prepare, reset, and finalize for that executor and every
  shared core device: PIC, PIT, DMA, KBC, and video.
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
