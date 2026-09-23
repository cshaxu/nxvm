# NES Debugger Workflow

Uses [shared context](../etc/m2-execution-management-context.md). Entry: accepted
[complete CPU/bus](../history/M2-T6-cpu-bus-completion-proposal.md) and its
runnable product/regressions.

## Outcome

A user can inspect registers/memory, disassemble, step, modify RAM, set a PC
breakpoint, run to it and inspect the stopped machine entirely from the cooked
Console. Normal execution and debugging use the same CPU and Common executor.

## Coverage And Implementation

Deliver all nine designed protocol operations: OBSERVE/PEEK/POKE/STEP,
BREAK_SET/BREAK_LIST/WARM_RESET, and explicit unsupported OUTPUT_SET/BIND_SET in
M2. Implement regs/mem/poke/disasm/step/break/delete/breaks and the designed App
count defaults. Implement the currently accepted reset-soft command contract;
any owner revision must update the contract before implementation, not silently
change it. Ordinary lifecycle/media behavior remains from the first package.

Use bounded copied values and fresh paused leases; prevalidate byte layouts and
response capacity before mutations. Guest stops report partial progress distinctly
from transport failure. Breakpoint bypass/rearm, BRK preservation, RAM-only poke,
side-effect-free peek, guest-trap inspection and correct disassembly are in scope.

## Acceptance And Transfer

Through real Common, insert a fixture, inspect and disassemble its entry, step a
known count, modify RAM, run to a breakpoint and verify copied PC/register/memory
facts, remove the breakpoint and resume. Test every opcode metadata class in
disassembly, all nine message contracts, invalid lengths/versions/capacities,
step limits and partial traps, lease invalidation, bounded breakpoint capacity
and illegal paused/running usage. Include actual command parsing/output checks.

Close DBG and debug-command APP obligations, maintaining both architecture builds
and prior regressions. Reliability receives cross-operation/fault-injection
matrices, not omitted debug correctness. Cycle refinement receives the same step
path as an additional timing consumer. No assembler or separate debug CPU.
