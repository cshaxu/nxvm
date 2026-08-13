# M5 T332: VM Session Construction Transaction

## Scope

T332 is the owner-approved VM-composition construction-drift package admitted
from [its active proposal](../proposals/m5-vm-session-construction-transaction.md).
It has three bounded subtasks: configuration materialization, lifecycle failure
atomicity, and test-fixture lifecycle convergence. Detailed completion evidence
is appended as each accepted subtask is committed.

## Boundaries

The task preserves the existing `vm_session_config` API, core/device ownership,
Console/debugger behavior, and local instruction/device fixture state. It does
not start mantle or alter CPU/device behavior except for a reproduced
composition-lifecycle defect within the admitted transaction boundary.

## S1: Configuration Materialization

S1 replaced the two default/configured `vm_session_create` branches with one
private profile materialization followed by one explicit memory/CPU/FPU override
step. The retained session initialization atomicity smoke now proves default
and explicit configuration images, profile-derived timing/clock/KBC state, and
the actual constructed core's memory/CPU/FPU values. It retains the existing
failure/recovery evidence and emits `M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK`.

The T332 developer artifact is `vm-0-5-0332`, SHA-256
`1DF13B58B316026C233127B9407A03003C8B997221B431BB321F01E4D0C54A7E`.
