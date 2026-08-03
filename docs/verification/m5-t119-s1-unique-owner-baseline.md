# M5 T119 S1: Unique Owner Baseline

## Confirmed Baseline

`src/vm/main.c` constructs one `vm_session`, installs its Console target, and
enters the retained Console. The guest execution route is the session runner
to `core_machine_run` and then the executor CPU context. Test-only CPU probe,
VDM-minimal presentation, profile registry checks, and alternate product
facades are not formal product routes.

## Required Closure

- VM Console must not initialize an already initialized session a second time.
- `core_machine` must not expose minimal and executor state as equal
  production run models.
- Debug, wait, and Console command work must not rely on an implicit current
  object at their formal command boundary.
- VDM minimal APIs must be VDM-owned and presentation must use `vdm_session`.
- Runtime profile descriptors/registry must either be consumed by composition
  or leave production source.

## Gates

The active tasks use the focused GCC target suite from `CMakePresets.json`,
plus static scans for forbidden core-to-product dependencies, TLS current
object facades, direct VDM-minimal presentation access, and obsolete wrapper
exports. T119 changes documentation only and creates no artifact.
