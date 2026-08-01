# M5 T10 S4 P1 Execution Context Carrier

The session-owned execution-context carrier is bound and unbound at the
existing device lifecycle points without changing refresh order. `nxvm-m5-t10`
compiled the new source and the finite real CPU probe retained its established
`#UD` and success marker. Controller state remains unchanged in this first step.

P2 binds the context to the existing CPU, RAM, port, and device objects through
opaque slots at `deviceInit`. The project GCC build and finite real CPU probe
passed after this binding; legacy anonymous state typedefs remain unchanged.

P3 exposes the context CPU slot to retained consumers without changing the
legacy anonymous CPU type. P4 makes the finite CPU-probe adapter that consumer:
it samples the bound `t_cpu` through the active context rather than through
the old device connector accessors. Reset, RAM writes, and instruction
execution remain on their retained paths. GCC builds, the established `#UD`
CPU-probe marker, and the no-media Console/debugger interaction regression
passed.

P5 extends the finite CPU-probe smoke to create, probe, finalize, and recreate
the retained PC/AT session. It verifies that the current context is cleared at
each finalization and that the second session retains the established CPU
result. The sequential probe and no-media Console/debugger regression passed.

P6 changes only the moved `machine/vm/machine.c` lifecycle implementation to
include its owned `machine/vm/device.h`, rather than the temporary baseline
forwarding path. GCC build, CPU probe, no-media Console, and debugger-prompt
regressions passed.

P7 changes only `machine/vm/vmachine.c` to include the migrated CPU, RAM, and
port headers from `machine/core`; its controller and firmware includes remain
unchanged for T11. GCC build, CPU probe, no-media Console, and debugger-prompt
regressions passed.
