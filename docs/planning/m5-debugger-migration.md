# M5 Debugger Migration

## T22 Audit

The retained NXVM Console sends `debug` directly to `debugMain()`. That UI
preserves the historical command grammar and reaches the real bootable VM
through `core_product_debug_target`, whose VM composition adapter reads and
writes the live NXVM global `vcpu`, `vram`, `vport`, and `vdebug` state.

The execution loop calls `vdebugRefresh()` before each machine refresh.
Breakpoints and trace counts request a stop by clearing the loop's run flag.
This is not an acknowledged pause protocol: Console entry, state access, and
resume are not serialized through one command boundary. The newer
`core/machine/debug.*` API instead controls a separate minimal
`nxvm_core_machine` owned by the session model; it is tested and initialized
but is not the authority that boots the current full PC. The authority
convergence rules are in [Live Machine Authority Migration](m5-machine-authority-migration.md).

## Target Contract

The real VM has these externally visible states:

`stopped -> running -> pause_requested -> paused -> debug_boundary -> running`

The execution owner alone changes `running`, `pause_requested`, or `paused`.
It acknowledges a pause only at the synchronized boundary between guest
instructions. Product UI may inspect or mutate guest state only while the
acknowledged `debug_boundary` is open. A breakpoint, watchpoint, trace count,
or explicit Console request all use the same pause request and report one stop
reason. `step` runs exactly one instruction through that owner and returns to
the boundary; `continue` closes the boundary and resumes. The retained command
grammar and printed output remain stable.

## T45: Pause Boundary

T45 adds a composition-owned debugger control state and synchronized request /
acknowledgement transport. The Console `debug` path requests pause and waits
for acknowledgement before calling the retained UI. `vdebug` converts its
breakpoint and trace detections into the same request. The execution loop
acknowledges at its existing command boundary, exposes the pause reason, and
does not execute another guest instruction until `continue` or `step` is
accepted. Existing `stop`, `reset`, window, input, boot, and recorder behavior
are regression gates; no raw recorder runs without a separately budgeted
experiment.

## T46: Unified Backend

T46 introduces one product-neutral debug backend interface for register,
memory, port, run-control, break/watch, disassembly inputs, and stop-reason
access. It binds only to the single live machine authority defined by the
authority migration plan. A future VDM supplies its owned-DOS machine adapter.
The small `core/machine/debug.*` API becomes a native implementation only after
it owns the real full-PC state; it must not be mistaken for that authority
before convergence. The retained debugger UI consumes the backend, removing
direct global-state access only after command, output, and DOS-boot regressions
pass.

## Exit Evidence

T45 requires a bounded breakpoint/pause/continue/step interaction against the
FDD fixture and no-media Console, plus lifecycle and input regressions. T46
requires the same interactions through the unified backend, parity checks for
the retained debugger commands, and explicit proof that it observes the live
full-PC state rather than the session-model machine.
