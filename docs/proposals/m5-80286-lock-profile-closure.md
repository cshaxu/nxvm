# 80286 LOCK And Profile Closure

## Objective

Close the 80286 profile after descriptor/transfer proof: establish the Intel
legal-versus-illegal `LOCK` matrix for every allocated memory-capable 8086,
80186, and 80286 form, then reconcile all remaining inherited and 80286-only
form/mode rows.

## Dependency And Ownership

This candidate consumes the 8086/80186 legacy matrix and the preceding 80286
descriptor/transfer ledger. It owns 80286 classification, not a new decoder
policy. Audit prefix ordering, whitelist/handler callers, memory preflight,
publication, fault/restart, and legal atomic behavior as one mechanism.

## Required Matrix

For each allocated memory-capable form, record legal locked memory execution,
illegal register or unsupported forms, profile-invalid cases, prefix ordering,
operand/address attributes, and fault/nonpublication behavior. Reconcile all
80286 inherited form rows across real and protected modes, including
Intel-required 16-bit stack/frame and selector differences.

## Non-goals And Stop Conditions

Do not alter the established 8086/80186 legacy `LOCK` policy without a
reproduced cross-profile defect. Do not implement 80386 operand/address,
VM86, paging, debug, or x87 behavior. Stop if an apparent 80286 issue is owned
by a shared delivery mechanism not already covered by its contract.

## Exit Standard

The 80286 ledger and `LOCK` matrix have no in-scope partial, missing, or
unclassified row. Every remaining 80386-only row is transferred once to the
80386DX candidates.
