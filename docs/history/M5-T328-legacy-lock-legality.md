# M5 T328: Legacy LOCK-Prefix Legality

## Scope And Result

T328 S1 closes the retained 8086/80186/80286 `LOCK`-prefix policy boundary.
The local `PREFIX_LOCK` path now keeps the historical bus-prefix treatment for
8086 and 80186, and for 80286 in real mode or protected mode when `CPL <=
IOPL`. Only protected 80286 with `CPL > IOPL` raises `#GP(0)` before the
following opcode can publish state. The later 80386 memory-modifying opcode
whitelist remains unchanged.

The owner smoke proves transparent register, memory, REP-string, and port-I/O
forms; preserved undefined-opcode behavior; protected 80286 allow/reject
ordering and restart frame; and the retained 80386 legal-memory versus
illegal-register distinction. It does not claim an observable host LOCK-pin,
bus arbitration, or timing model.

## Verification And Artifact

Fresh GCC configuration, the target-local strict command audit, focused marker
`M5:T328:S1:LEGACY-LOCK:OK`, retained S64 prefix regression, documentation
governance, and `git diff --check` passed. The full current gate passed
211/211 tests.

Commit `e5aa9d97` produced current developer artifact `vm-0-5-0328` at
`build/output/nxvm_0_5_0328.exe`. Its SHA-256 is
`52D81668FE747C5A1083EE5F9A5C33A5C71F41C5383B299B1728EAD8F523DFEA`.

## Transfers

VME/PVI, physical bus arbitration/timing, task state, paging, and x87
execution remain in their independently named Queue or TODO boundaries. No
legacy LOCK-prefix debt remains in `TODO.md`.

## Current Crosswalk Reconciliation

T328 S2 reconciles the current [80386DX closure map](../etc/evidence/80386-closure-map.md)
and ordinary-execution matrix with this accepted result. The completed
8086/80186/80286 `LOCK` boundary is no longer a Deferred or TODO destination.
Older task histories retain their contemporaneous transfer statements as
historical records; they are not current allocation authorities.
