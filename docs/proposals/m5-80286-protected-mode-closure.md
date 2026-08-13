# 80286 Protected-Mode And Descriptor Closure

## Objective

Close audit-assigned 80286 protected-mode instruction and state rows:
descriptor validation, privilege, GDT/LDT/IDT and gate behavior, 16-bit task
state, and their required failure and return contracts.

## Dependency and scope

This candidate relies on the shared delivery foundation and preserves genuine
Intel layout differences instead of flattening 16-bit and 32-bit state. 80386
extensions, paging, VM86-specific state, and x87 execution remain outside
unless the ledger assigns a shared prerequisite.

## Completion standard

All assigned 80286 rows have a form-and-mode proof or an explicit transfer;
no accidental reuse of a 386 construction path is presented as a 286 contract.
