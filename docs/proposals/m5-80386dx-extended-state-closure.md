# 80386DX Closure Program Context

## Purpose

This shared context defines the 80386DX boundary. The Queue breaks it into
form/width closure, system-state closure, and a final profile audit. It is not
itself an admission contract or a numeric task.

## Dependency and scope

The candidate consumes the audit and shared-state foundation, then splits into
bounded tasks by actual mechanism ownership and dependency. It preserves
architectural differences from 8086--80286. VME/PVI, 486+ instructions, x87
execution, timing, and device fidelity are explicit external boundaries.

This candidate is downstream of the shared, 8086/80186, and 80286 packages.
It may consume their documented contracts, but must not alter an earlier
profile disposition to make a 32-bit form convenient.

## Completion standard

Every assigned 80386DX form and state transition has focused evidence for its
mode, privilege, prefix, and fault contract, or is explicitly outside 80386.

## Ordered Breakdown

1. [Closed width, prefix, and integer-form proposal](../history/M5-T340-80386dx-width-integer-closure-proposal.md)
   closes residual form-family rows without obscuring older-profile results.
2. [System, VM86, paging, and debug closure](../history/M5-T341-80386dx-system-state-closure-proposal.md)
   owns residual privileged state composition and its form consumers.
3. [80386DX profile closure](m5-80386dx-profile-closure.md) reconciles the
   resulting form and state ledgers and returns a missing row to its owner.

Each admission begins with the vertical state owner (translation, debug,
VM86, descriptor/table state, or shared execution plan) and its callers,
validation, materialization, commit, rollback, and fault boundaries. It then
allocates the related opcode forms as one bounded proof matrix. This prevents
the remaining work from degrading into independent smoke-driven patches.

VME/PVI, later-CPU forms, persistent TLB models, and x87 execution stay outside
unless a separate owner decision expands the architecture boundary.
