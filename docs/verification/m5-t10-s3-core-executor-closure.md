# M5 T10 S3 Core Executor Closure

The user-facing executor now compiles its real NXVM port, RAM, CPU control,
and 8086+ dispatcher sources only from `src/machine/core/`. CMake contains no
baseline source entry for those implementations. `src/core` contains no C
implementation; its remaining headers are compatibility forwarders to the
non-executing contract subtree under `src/machine/core/contract/`.

Focused CPU probes and the retained no-media Console/debugger regressions from
P1 through P7 passed. The contract relocation evidence P8 through P18 confirms
that it is not a second executor. S4 owns replacing the remaining temporary
single-session global binding with an explicit execution context.
