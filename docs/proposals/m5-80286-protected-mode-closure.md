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

This candidate follows shared delivery and the 8086/80186 legacy baseline.
It owns the 80286-specific protected transition and descriptor mechanisms;
it does not borrow a 32-bit construction merely because the later-profile
route happens to be nearby.

## Completion standard

All assigned 80286 rows have a form-and-mode proof or an explicit transfer;
no accidental reuse of a 386 construction path is presented as a 286 contract.

## T336 Audit-Derived Breakdown

The expected sequence is:

1. reconcile every `0F 00`, `0F 01`, `0F 02/03`, and `0F 06` form and its
   descriptor/table/privilege disposition;
2. close the 16-bit protected transfer, gate, stack, and task rows that remain
   after T323/T329;
3. establish the 80286 `LOCK` legality matrix for every allocated
   memory-capable form, including legal atomic forms, profile-invalid forms,
   and prefix/fault publication behavior; and
4. perform an 80286 profile-close audit.

The tasks preserve true 16-bit TSS, gate, and frame layouts and transfer only
80386-only address, VM86, paging, debug, or register behavior forward.

## Admission and exit constraints

For a repeated protected-state mechanism, map descriptor/selector validation,
memory preflight, state materialization, commit, rollback, and all callers
before changing one form. An apparently common 16/32 implementation may share
only a private plan that selects the real architecture layout independently.
The candidate closes only when its assigned ledger and 80286 `LOCK` matrix
have no in-scope partial, missing, or unclassified row.
