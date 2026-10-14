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

Coordinator reviewed cab60e0ac actual proposal/ledger/packet/queue/history
changes against the owner request. All applicable CPU rows remain open;
source-sensitive silicon decisions are explicit rather than invented.
The state-owner design includes implicit CR3 writes and descriptor paging,
and the grouped plan does not substitute transfer for completion. S1 is
accepted and closed. S2 admits the base reset/control/paging/descriptor batch;
Shared and MyNES remain outside its change authority.

## S2 Base-System Delivery

The [base-system evidence](../etc/evidence/t560-s2-base-system.md) records
reset state/fetch, CR0/CR3, WP permissions, linear descriptor tables and
conditional Accessed publication, with actual guest-instruction regressions.
Complete repository units pass 597/597 per width; all four PC integration
groups pass on both widths and eight verified 0560 EXEs replace 0559.
The convergence ledger distinguishes these base subcases from still-open
cache/TLB, AC/RMW, VME/SMM, inherited semantics, timing and Core admission.

On 2026-10-14 the owner permanently approved routine task-version updates
to build targets, presets and release configuration without repeated approval
requests. S2 therefore includes three root preset target replacements as a
separate Shared P; the CPU, tests, evidence and artifacts remain an NXVM P.
This approval does not broaden public-component functionality/API authority
or authorize unrelated changes. Neither Shared-six nor MyNES source/test or
MyNES artifacts changed. Executor review and verification are recorded in
the evidence; coordinator acceptance follows the complete pushed delivery.
