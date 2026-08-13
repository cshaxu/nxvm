# Task-Switch Transition Unification

Resolve the explicit
[16-bit/32-bit task-transition construction debt](../etc/evidence/task-switch-transition-unification-debt.md)
before declaring the task-system family closed. Map all current task-entry
callers and TSS/descriptor writes; align the reviewable preflight, planning,
and commit semantics for `JMP`, `CALL`, and task-gate entry while preserving
width-specific Intel TSS layouts; and rerun the accepted direct-JMP plus new
task-entry fault matrices. Nested return, IDT task gates, LDT, paging, and
debug state remain separately assigned rather than being silently absorbed.

> Historical proposal retained with T330. T330 closed its stated construction
> debt; this file is not a current Queue candidate or task contract.
