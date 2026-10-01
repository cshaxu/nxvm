# T539 S88 Execution-Lifecycle Receiver Evidence

## Allocation

Shared now owns `cpu_execution_lifecycle_smoke.c`, receiving the former
`cpu_timing_case`, `cpu_execution_context_reset_case`, `cpu_instance_case` and
unnamed two-context reset/request-isolation rows from the mixed execution
context test. It retains the same five CPU profiles, reset-vector checks,
prepared-entry commit/reject checks, two-instance independence and REP timing
phase checks.

The NXVM residual keeps, exactly once: debug-state access, NMI signal handling,
prefetch, 80186 LGDT gate, paging controls and INVLPG, UD cache preservation,
and interrupt pending/rollback. Those rows are reserved for S89 or later; no
board wiring, production source, fixture API or public ABI changed.

## Verification

- x64/x86 focused Shared lifecycle and NXVM residual tests: pass.
- x64/x86 complete repository-only unit suites: **441/441**, exit code zero.
- x64/x86 T332 CPU fixture-lifecycle gate: pass (44 owners).
- Shared manifest/corpus, CPU/PIC authority, documentation governance and diff
  checks: pass.

No production source, firmware, asset, INI or executable input changed, so no
EXE rebuild is required.
