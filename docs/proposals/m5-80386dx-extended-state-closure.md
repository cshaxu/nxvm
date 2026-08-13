# 80386DX Extended Instruction And System-State Closure

## Objective

Close the audit-assigned 80386DX-only forms and state: 32-bit operand/address
variants, 0F extensions, FS/GS and debug state, paging, VM86, and 32-bit
system/descriptor behavior.

## Dependency and scope

The candidate consumes the audit and shared-state foundation, then splits into
bounded tasks by actual mechanism ownership and dependency. It preserves
architectural differences from 8086--80286. VME/PVI, 486+ instructions, x87
execution, timing, and device fidelity are explicit external boundaries.

## Completion standard

Every assigned 80386DX form and state transition has focused evidence for its
mode, privilege, prefix, and fault contract, or is explicitly outside 80386.
