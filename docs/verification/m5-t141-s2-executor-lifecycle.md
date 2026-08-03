# M5 T141 S2: Core Standard-Executor Lifecycle

## Result

`core_machine_create()` now initializes CPU execution state, port state and
port-provider table, RAM, and RAM-to-port registration before exposing the
machine. `core_machine_destroy()` remains their single finalizer. VM provider
assembly, VDM minimal creation, and the shared executor fixture no longer
prepare or finalize those core objects directly.

## Verification

Windows GCC built and ran the focused core lifecycle, executor-run, VDM
minimal, two-session, retained Console, and FDD DOS-prompt gates. Observed
markers:

```text
M5:T83:S3:CORE-EXECUTOR-RUN:OK
M3:T2:S1:LIFECYCLE:OK
M5:T94:S1:VDM-SESSION-ISOLATION:OK
M5:T73:S1:TWO-SESSION-ISOLATION:OK
M5:T96:S1:CONSOLE-LIFECYCLE:OK
M5:T70:S2:DOS-PROMPT:OK
```

The source scan finds no `core_machine_prepare_executor_*` or direct
CPU/RAM/port finalizer call in VM or VDM production source. PIC/PIT/DMA/KBC/
VADP ownership remains the next bounded S3 operation.
