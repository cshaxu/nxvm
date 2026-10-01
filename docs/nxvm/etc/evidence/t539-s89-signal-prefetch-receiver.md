# M5 T539 S89 CPU signal/prefetch receiver

## Scope

S89 moves exactly two CPU-only static functions from
`test/app-nxvm/unit/core/devices/cpu_execution_context_smoke.c` to the sole
Shared receiver `test/x86/devices/cpu/cpu_execution_signal_prefetch_smoke.c`:

- `cpu_signal_case()` — NMI mask, pending-edge and delivery behavior;
- `cpu_prefetch_case()` — 8088/other prefetch reservation and reset behavior.

Both reuse the existing CPU-local `cpu_bus_fixture`; no production source,
public API, board adapter, firmware, asset, INI or executable input changes.
The NXVM residual retains only debug, S90 paging/INVLPG and S91 fault/event
functions.

## Verification

- Focused receiver and residual x64/x86 executables: passed.
- `verify-t332-cpu-fixture-lifecycle`: passed on x64 and x86 (44 owners).
- CPU/PIC authority, Shared test manifest/corpus, documentation governance and
  `git diff --check`: passed.
- Detached repository-only unit suites: x64 **442/442**, exit `0`, 75.79 s;
  x86 **442/442**, exit `0`, 75.74 s.

## Artifact determination

This is test/CMake/documentation-only ownership work. No executable input
changed, so no NXVM binary rebuild is required.
