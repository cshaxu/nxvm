# T539 S87 Execution-Bus Receiver Evidence

## Allocation

Shared owns the former `cpu_bus_cases()` body from
`test/app-nxvm/unit/core/devices/cpu_execution_context_smoke.c`, now as
`test/x86/devices/cpu/cpu_execution_bus_smoke.c`.  It covers CPU linear-bus
width, reset-fetch addressing, synchronous write observation, scalar port
effects, copied retirement observation, rejected transfers, and interrupt
acknowledge/rollback.

`test/x86/devices/cpu/support/cpu_bus_fixture.h` is the one CPU-local fixture
used by that receiver.  The NXVM residual and its FLAGS-local smoke include the
same Shared fixture directly; the old App copy is deleted.

The NXVM residual keeps, exactly once: timing, NMI/signal, debug, context
reset, instance lifecycle, prefetch, 80186 LGDT gate, paging controls/INVLPG,
interrupt preparation, UD cache preservation, and pending-interrupt rollback.
Those rows remain allocated to S88 and S89.

## Verification

- x64/x86 `shared-x86-tests` build: pass.
- x64/x86 `x86.cpu_execution_bus`: pass.
- x64/x86 `core-machine-cpu-context-smoke` and
  `core-machine-eflags-local-smoke`: pass.
- x64/x86 T332 CPU fixture-lifecycle gate: pass (44 owners).
- Shared test manifest, x86 corpus and CPU/PIC authority gates: pass.

The terminal-hosted full-unit invocations initially misreported
`unit.cpu-bus-boundary-negative` as failed even though its direct CMake command
exited zero and printed all three successful groups (72 CPU, five board, and
96 migrated-board negatives). Detached process-owned CTest runs remove that
terminal artifact: x64 **440/440** and x86 **440/440** pass with exit code zero.
S87 is therefore accepted.

No production source, API, firmware, asset, INI or executable input changed.
