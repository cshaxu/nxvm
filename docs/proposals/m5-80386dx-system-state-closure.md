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

## Required Matrix

Reconcile CR/DR/TR form and privilege tables; DR6/DR7 matching/cause and
vector-1 delivery; paging/task/VM86 compositions; residual VM86 instruction
and state rows; and outstanding system/table forms. Preserve true 16/32-bit
layouts and prove frame, stack, restart, nonpublication, and TSS-dependent
behavior where applicable.

## Non-goals And Stop Conditions

VME/PVI, 486+ features, persistent TLB/test-register models, x87 numerical
execution, timing, and devices remain outside. Stop rather than add a generic
interrupt, paging, or task rewrite unless the complete affected state-owner
matrix is admitted.

## Exit Standard

All assigned state and privileged-form rows have an exact proof or an approved
outside-80386/TODO transfer. The result supplies a closed source ledger for
the 80386DX profile audit.
