# 80286 Protected-Mode Closure Program Context

## Purpose

This shared context defines the 80286 closure boundary. The Queue allocates
its implementation in two ordered candidates: descriptor/transfer mechanics,
then the `LOCK` matrix and profile-close reconciliation. It is not itself an
admission contract or a numeric task.

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

## Ordered Breakdown

1. [Descriptor-table and protected-transfer closure](m5-80286-descriptor-transfer-closure.md)
   resolves table/selector/gate and 16-bit transfer ownership first.
2. [80286 `LOCK` and profile closure](m5-80286-lock-profile-closure.md)
   consumes that proof, establishes the matrix, and performs the profile audit.

The tasks preserve true 16-bit TSS, gate, and frame layouts and transfer only
80386-only address, VM86, paging, debug, or register behavior forward.

## Admission and exit constraints

For a repeated protected-state mechanism, map descriptor/selector validation,
memory preflight, state materialization, commit, rollback, and all callers
before changing one form. An apparently common 16/32 implementation may share
only a private plan that selects the real architecture layout independently.
The two candidates together close only when the assigned ledger and 80286
`LOCK` matrix have no in-scope partial, missing, or unclassified row.
