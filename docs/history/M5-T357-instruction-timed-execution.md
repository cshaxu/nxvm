# M5 T357: Instruction-Timed Execution

## Task Record

T357 is the first physical-L3 prerequisite accepted after T356's M5-open
closure audit.  It replaces the retained synthetic instruction surcharge
mechanism with a source-backed, profile-aware CPU form-cost owner for an
explicitly finite corpus.  It is not a bus model or a cycle-exact CPU claim.

## Accepted S1 Progress

S1 was accepted at `8e0841b6`.  Its [source/form contract](../etc/evidence/t357-s1-instruction-timing-contract.md)
inventories the existing synthetic cadence and the sole retirement publication
owner, identifies primary profile references and their assumptions, allocates
the selected S2 corpus, and transfers physical timing dependencies precisely.

## Accepted S2--S4 Progress

S2 added the nonpublishing 80386 `Jcc` lexical-preview prerequisite. S3 added
the first selected 80386 source ledger at the sole retirement publisher. S4
added the selected 8086 ledger using Tables 2-20 and 2-21, including EA,
segment-override and odd-word additions, profile-local `REP MOVSB`, and an
explicit unsupported-form fallback. Its focused proof covers fault/stop,
reset, budget, overflow, and scheduler publication; 80186, 80286, and
unallocated 80386 rows remain separate source-ledger transfers.

S5 added the selected 80186 Table 1-16 ledger. It shares only the genuine
16-bit construction with 8086 while retaining independent source values for
`MOV`, moffs, Jcc, `REP MOVSB`, and `OUT`; 80286 remains the compatibility
profile until its own source-ledger packet.

S6 replaces that 80286 compatibility cadence with the selected formal
Appendix-B ledger, including `NOP`, `MOV`, `MOVSB`/`REP MOVSB`, short Jcc, and
port forms.  It retains a distinct 80286 EA/odd-word construction rather than
borrowing the different 8086/80186 rules; all profiles still use the one
successful-retirement elapsed-tick publisher.  The Appendix-B `NOP = 3`
allocation is authoritative for this ledger, while its conflict with manual
prose is an explicit later reconciliation debt.

## Intended Task Closure

T357 closes only after the selected corpus advances guest time from the single
CPU retirement owner with profile/form evidence, fault and reset publication
proof, and retained deterministic L3 regressions.  Memory/I/O wait states,
CPU/DMA physical ownership, prefetch, and cycle-exact behavior transfer to the
subsequent bus-timed and selected-profile candidates.
