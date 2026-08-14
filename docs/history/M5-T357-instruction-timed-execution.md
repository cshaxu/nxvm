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

## Intended Task Closure

T357 closes only after the selected corpus advances guest time from the single
CPU retirement owner with profile/form evidence, fault and reset publication
proof, and retained deterministic L3 regressions.  Memory/I/O wait states,
CPU/DMA physical ownership, prefetch, and cycle-exact behavior transfer to the
subsequent bus-timed and selected-profile candidates.
