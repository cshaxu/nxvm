# Optimized Artifact and Trace Audit

## Purpose

Apply the owner's 2026-08-24 corrective scope to the latest closed task:
publish its existing executable only as an optimized build with debug
information, and remove only inactive internal trace work from the normal
optimized runtime.

## Boundary

The current artifact identity is unchanged. Preserve the Debug current-gate
route and every production debugger capability, including its trace command,
pause, step, break/watch, recording, and product fault diagnostics. Audit Core
trace and retirement-observation consumers before changing any hot path. Do
not change guest timing, device behavior, public ABI, or introduce a second
artifact route.

## Completion

One RelWithDebInfo publication route must rebuild the current artifact; a
non-optimized build must be unable to copy it to `build/output`; every
trace/diagnostic mechanism must have a consumer disposition; focused release
debugger tests, Debug current gate, and documentation governance must pass.
