# M5 T560 Complete 486SX CPU

## Admission

Owner explicitly approved T559 closure and the next task completing all
remaining 486SX CPU gaps. T559 closure is fb8021291. This task consumes the
system proposal and all remaining CPU obligations; it cannot close by moving
applicable unimplemented CPU semantics into another T.

## S1 Planning Delivery

The [convergence ledger](../etc/evidence/t560-s1-cpu-convergence.md) records
C01--C17, source anchors and unresolved exact-silicon applicability, actual
control/paging/descriptor/cache/TLB/timing gaps and existing proof limits.
It includes inherited RMW ordering and real Core signal/construction routes,
not only new opcodes. Planned S2--S10 batches have distinct state owners,
estimated changes and direct exit tests. Sequential admission remains required.

Read-only inspection found no reason to change Shared-six or MyNES. Exact
SL-enhanced SX package applicability remains a named C01 proof obligation;
base-family implementation can proceed without pretending it is settled.
S1 changes only NXVM documentation: production/test added 0, removed 0, net 0.
No new API/state/helper, no generated source or firmware import. The verified
0559 artifacts and T559 S7 unit/external qualification remain unchanged.

Documentation governance and whitespace checks pass. Actual review must
confirm the packet, ordered queue, task transfer, coverage dimensions and
honest open-row status before S1 acceptance. No runtime tests are repeated
for documentation-only planning; no claim of CPU implementation accompanies it.
