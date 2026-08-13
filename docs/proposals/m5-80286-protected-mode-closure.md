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

## T336 Audit-Derived Breakdown

The expected sequence is: first reconcile every `0F 00`, `0F 01`, `0F 02/03`,
and `0F 06` form and its descriptor/table/privilege disposition; then close
the 16-bit protected transfer, gate, stack, and task rows that remain after
T323/T329; finally perform an 80286 profile-close audit. The tasks preserve
true 16-bit TSS, gate, and frame layouts and transfer only 80386-only address,
VM86, paging, debug, or register behavior forward.
