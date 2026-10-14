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

Coordinator actual-change review accepts Shared e7c3f4e78 and NXVM ddb260095.
Review checked the complete CPU/header/test changes, control and descriptor
caller sweep, build/preset identities, manifest deltas, artifact hashes and
evidence against the original full-CPU request and bounded S2 packet. The
source changes retain one execution path and explicit older-family behavior;
the tests drive actual CPU instructions and valid exception handlers.
Both 597-case unit suites and all eight integration groups passed. No manual
RDP or production 486 admission is inferred. S2 closes with no unresolved
base-batch delivery item; remaining CPU rows stay in T560. No new S has yet
been admitted by this closure-only governance commit.

## S3 Alignment And Read/Modify/Write

S3 is automatically admitted after accepted S2, under the owner's sequential
Core authority. The [batch evidence](../etc/evidence/t560-s3-alignment-rmw.md)
records C05/C09 source reconciliation, the finite instruction-family caller
sweep, actual guest exception/provider regressions and production decisions.
Alignment uses the existing logical-access owner and exception path; modifying
operands use one write-preflight/read entry. Composite/string accesses check
their full operands before data effects, and masked-zero scalar shifts avoid
unnecessary writes. Read-only commands and old-family scope remain explicit.

Whole repository units pass 599/599 per width, with Core manifests and docs
gates passing. Production adds 184 and removes 78 lines; tests add 677 lines.
All eight final PC deployment/integration groups pass and eight verified
0560 binaries are deployed. The Windows integration race is corrected by
reading/checking paused BDA before stop can clear memory; production lifecycle
is unchanged. The additional test adds 5/removes 4 lines. INI and MyNES
artifact hashes are unchanged. Commit/push and coordinator acceptance remain
required before S3 closes.
No complete 486 admission, cache/TLB or remaining T560 row is claimed here.

Coordinator actual-change review accepts cb52586a9. Review re-read the pushed
Core helper/handler changes, new and extended tests, Core AC vocabulary,
manifest membership, Windows paused-observation correction, task records
and all eight artifact identities. The direct instruction matrix and final
599/599 unit suites per width plus eight integration groups meet the ordinary
access/RMW batch's exit. Four private helpers perform necessary decisions;
there is no additional state, interpreter, recovery loop or Shared change.
INI/MyNES hashes remain unchanged. S3 is accepted and closed.

The original all-486 objective is still open: C06/C07 real TLB/cache, selected
SL/VME/SMM applicability, C13 exception-entry/nesting, inherited whole-opcode
and timing matrices, and final Core admission retain their planned T560
owners. C05 whole-row acceptance is reconciled with C13 in that later matrix;
normal alignment delivery is not substituted for complete CPU qualification.
This closure-only P admits no next implementation packet.

## S4 Translation Cache Admission

After revalidating clean bbc8b85c6 and S3 closure, the coordinator admits S4
under the owner's automatic sequential Core authority. The CURRENT packet
consumes the complete C06 batch: real CPU translation entries, test-register
commands, INVLPG and all invalidation/preview paths. Source review precedes
production edits; this admission is not implementation or runtime evidence.
Shared-six, MyNES and owner inputs remain excluded. Eight S3 0560 PC EXEs
remain current until a verified S4 replacement is deployed.

S4 implementation and final qualification are delivered. The
[batch record](../etc/evidence/t560-s4-translation-cache.md) maps real TLB,
TR6/TR7, INVLPG, reset/control/task/debug publication and preview to original
sources, explicit L2 internal choices and actual guest-command tests.
Production changes +237/-9; tests +671/-2. Both whole 600-case unit suites
and all eight integration groups pass; the final V=0 test-only addition
passes its affected recheck per width without changing production inputs.
Eight verified 0560 PC artifacts are deployed; MyNES and owner inputs remain
unchanged. Complete P push and coordinator review remain before acceptance.
Full page-walk bus-cycle/timing, cache, remaining CPU rows and production
admission stay in T560; this is not full CPU or Windows GUI qualification.

Coordinator actual-change review accepts pushed c32103a24. Review inspected
the production state/translation/control/handler diffs, the complete new CPU
test, all extended permission/task/test-register tests and registration,
manifest membership, eight PE/version/hash identities and task/source records.
The ordinary/test commands share one private TLB; debug invalidation occurs
only on committed candidates and preview leaves live state unchanged.
Current test inputs have proof from both 600-case full runs plus the final
affected-test recheck; all eight integration/deployment groups pass. The
explicit internal L2 choices are not relabelled L3, and C04/C16 physical-bus
work remains mandatory before full CPU admission. Shared-six, MyNES and owner
inputs are unchanged. S4 is accepted and closed, T560 remains open; no next
implementation packet is admitted by this closure-only P.
