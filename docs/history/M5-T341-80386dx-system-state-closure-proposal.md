# 80386DX System, VM86, Paging, And Debug Closure

## Objective

Close the residual 80386DX privileged state and form rows: CR/DR/TR behavior,
ordinary breakpoint/vector-1 composition, task and VM86 interactions, paging
and system-control consumers, and remaining descriptor/system forms.

## Dependency And Ownership

This candidate follows the 80286 profile closure and 80386DX form closure. It
groups rows by vertical state owner, not by isolated opcode: each admission
maps all callers, validation, materialization, preflight, commit, rollback,
and fault/delivery boundaries. Existing bounded paging, VM86-delivery, and
task-transition evidence is reused only after exact scope comparison.

The initial ledger divides work by state owner: VM86 entry/exit and frame
rules; task/TSS transition; paging translation and page-fault composition;
CR/DR/TR privilege and validation; and breakpoint/debug cause plus vector-1
delivery. It records all readers/writers, preflight/commit/rollback, frame and
stack selection, and cross-owner handoff. A caller-level symptom cannot close
a state-owner row without that complete construction audit.

## Required Matrix

Reconcile CR/DR/TR form and privilege tables; DR6/DR7 matching/cause and
vector-1 delivery; paging/task/VM86 compositions; residual VM86 instruction
and state rows; and outstanding system/table forms. Preserve true 16/32-bit
layouts and prove frame, stack, restart, nonpublication, and TSS-dependent
behavior where applicable.

## Admission Sequence

The state package is split only along real state-owner boundaries:

1. map all readers/writers and transitions for CR/DR/TR, table/system forms,
   VM86/task, paging, and debug delivery;
2. close privileged control/table and CR/DR/TR validation-to-commit rows;
3. close VM86/task/paging composition while retaining genuine 16/32-bit TSS,
   frame, and stack layouts; and
4. close ordinary breakpoint/debug cause and vector-1 composition, then audit
   every state transfer together.

Accepted paging, task-transition, or VM86-delivery evidence is reused only
after exact scope comparison. A new state subtask is admitted only for an
unproved composition or reproduced mechanism defect.

## Non-goals And Stop Conditions

VME/PVI, 486+ features, persistent TLB/test-register models, x87 numerical
execution, timing, and devices remain outside. Stop rather than add a generic
interrupt, paging, or task rewrite unless the complete affected state-owner
matrix is admitted.

## Exit Standard

All assigned state and privileged-form rows have an exact proof or an approved
outside-80386/TODO transfer. The result supplies a closed source ledger for
the 80386DX profile audit, with genuine 16/32-bit layout differences retained
as explicit contracts rather than flattened helper behavior.
