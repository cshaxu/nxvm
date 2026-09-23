# M5 T341: 80386DX System, VM86, Paging, And Debug Closure

## Task Record

T341 is the Queue-ordered successor to the closed 80386DX form package. It
owns the residual vertical state composition required for a truthful 80386DX
profile claim: privileged control and table state, VM86, task/TSS, paging, and
debug/breakpoint delivery. It preserves genuine 16-bit and 32-bit layouts and
reuses earlier evidence only after a state-owner comparison.

The active packet in [CURRENT](../states/CURRENT.md) owns each admitted S.
The linked proposal remains current until T341 closes.

## S1 State Allocation

S1 creates the complete state-owner ledger before implementation. It maps CR,
DR, TR/LDT/table, VM86, task/TSS, paging, and vector-1 boundaries through all
readers/writers and validation-to-commit paths. It may reuse accepted T320,
T321, T325, T326, T329, and T337 evidence only where their exact frame,
privilege, mode, and publication boundaries match the T341 row.

The indexed allocation ledger determines the later bounded mechanism S work.
It cannot close a runtime row or introduce a new execution path.

S1 was accepted through source, evidence, and focused-owner review. Its
implementation delivery is `b4d4251c`; S2 now owns only the CR/DR/TR/LDT/table
reconciliation defined by that ledger.

## S2 Control, Debug-Register, And Table-State Reconciliation

The indexed [S2 reconciliation](../etc/evidence/t341-s2-control-debug-table-reconciliation.md)
maps every assigned form to its one decoder and mutable-state owner, retaining
the exact prior proof where its state and delivery contract match. It confirms
that DR4/DR5 are Intel-reserved on the 80386 rather than later-CPU aliases and
that TR6/TR7 require the explicitly deferred persistent-cache model.

S2 transfers only VM86/task/paging composition to S3 and ordinary breakpoint
cause/vector-1 delivery to S4. It makes no production change.

The accepted S2 delivery is `3f4f3cb2`. S3 now owns the VM86, task/TSS, and
paging composition graph, retaining genuine TSS and frame layout differences.

## S3 VM86, Task, And Paging Composition

The indexed [S3 composition graph](../etc/evidence/t341-s3-vm86-task-paging-composition.md)
ties VM86 entry and return to the same page-walk, TSS stack, frame, and task
plan/commit contracts. It adds mapped VM86 source delivery and mapped IRET
return probes without creating a second executor, page walker, or TSS format.

The accepted S3 delivery is `cf43c5b8`. S4 now owns ordinary DR6/DR7
breakpoint matching, cause publication, priority, and vector-1 delivery as one
debug state-machine mechanism.

## S4 Ordinary Debug And Vector-1 Composition

The indexed [S4 debug graph](../etc/evidence/t341-s4-ordinary-debug-vector1.md)
binds instruction-breakpoint faults, post-access data traps, sampled TF/RF,
incoming-TSS task debug, DR6 cause publication, DR7 local/global task
semantics, and real/protected/VM86 vector-1 delivery to one execution path.
It also corrects the earlier TF/PIC ordering statement: an ordinary sampled TF
trap is selected before a pending maskable IRQ; an interrupt-gate handler then
has its ordinary IF-clearing behavior.

## S5 Closure Audit

The [S5 closure audit](../etc/evidence/t341-s5-80386dx-system-state-closure-audit.md)
reconciles every S1 allocation exactly once with its S2/S3/S4 owner and focused
proof. T341 therefore closes the 80386DX system-state package and transfers
only explicit later-CPU, x87, timing/device, and Windows boundaries.
