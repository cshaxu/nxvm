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

## T336 Audit-Derived Breakdown

Admission divides the retained rows by mechanism rather than opcode smoke:

1. primary 32-bit operand/address and prefix combinations not already proven
   by the semantic-class closure;
2. all metadata-valid `0F` data/control/segment forms, including complete
   CR/DR/TR privilege and state classification;
3. residual VM86, paging/task interaction, debug/breakpoint, and system-state
   rows after the shared-delivery and 80286 candidates close; and
4. a 80386DX profile-close audit against the complete form ledger.

VME/PVI, later-CPU forms, persistent TLB models, and x87 execution stay outside
unless a separate owner decision expands the architecture boundary.
