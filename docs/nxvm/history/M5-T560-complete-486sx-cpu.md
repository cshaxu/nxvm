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

## S5 Cache And Physical-Bus Admission

The coordinator revalidated clean 5560979f9 and admits S5 under the owner's
automatic sequential Core and interface authority. The active packet consumes
C07 and the relevant carried C04/C16 source obligations. Cache tags/lines and
TR buffers remain CPU-owned; physical attributes, external effects and
coherence must connect to actual Core routing. Original-source and exact
contract review precede edits. This admission changes no production, tests,
Shared-six, MyNES or artifacts and claims no S5 runtime qualification.

The executor inspected the original PRM cache test-register diagram and
developer cache-control, page-cacheability, A20M and test-buffer contracts.
The [source/route evidence](../etc/evidence/t560-s5-cache-bus.md) records why
CD is not a cache-hit bypass, NW cache-as-RAM behavior must survive, test and
ordinary accesses share circuitry, and the Core-only A20 mask cannot remain
below an unmasked CPU cache. Actual route and write publication inspection
also found the narrow-provider span and observer-coverage constraints.
These are design inputs, not completed cache semantics or new runtime proof.

The first CPU-owned implementation now supplies cache lines/LRU and fill/read
buffers, real MOV TR3--5 commands, TR5 read rejection and reset ownership.
Focused cache/TLB/TR regressions pass on x64 and x86 in the original cached
graphs; Core manifests, dependency and documentation gates pass. Ordinary
cache access and physical-bus integration are not implemented yet, so this is
ongoing S5 work, not a complete P, C07 acceptance or artifact release. Actual
interim counts and proof boundaries are recorded in the S5 evidence.

Ordinary CPU access now consumes the Core cache query and one shared cache:
real hit/fill/no-allocation write, write-through/cache-as-RAM, per-access
CR3/PDE/PTE attributes and independent split-page reads. External transfers
share one bus-publication owner across old and selected profiles. Direct
failure checks preserve cache/backing data, and actual rebuilt dual-width
focused selections pass 4/4 with a final test-only recheck. Original manuals,
code/model limits and the diagnosed null-DS fixture error are recorded in S5
evidence. External commands, snoops, full C07 matrix/integration and final
whole-unit/artifact qualification remain open; no complete P or release yet.

INVD/WBINVD now enter through the existing selected-family metadata/dispatch
gate, invalidate real internal state and deliver ordered typed external
operations. Core executes the frozen attachment callback within its existing
transaction, preserving failure/cancellation without replay. Focused dual-width
proof covers mode/prefix and populated/empty cases, actual CPL/VM86 GP frames,
failed fetch, missing implementation and failure at each WBINVD stage; Core
directly proves no-cache transaction completion and attachment cancellation
and reuse. CPU-owned protected-exception setup is shared with retained TR
tests without dropping assertions. S5/T560 remain open for snoop, remaining
attribute/matrix/integration work and complete qualification; artifacts remain
the verified S4 baseline.

CPU range invalidation is connected to the existing Core physical-write
publication owner. Actual CPU-write transactions suppress self-snoop; DMA,
debug and firmware writes use one borrowed private callback without consuming
board observer slots. Real Core CPU instructions prove RAM/alias/DMA visibility,
write-through residency and NW preservation; tests also cover provider write
failure, accepted provider publication and binding lifetime through RAM
replacement. The final focused four-test selection passes on both widths,
with a changed-test-only x64 recheck after the final provider assertion.
Core dual-address alias coherence is recorded as L2 rather than physical pin
qualification. Remaining page-control, complete sweep/matrix and whole-S
qualification stay active; no partial P or artifact release is made.

Further S5 review removes repeated fill-route probing from cache hits and
writes through one typed Core query. Refreshed aggregate unit-consumer builds
complete on both widths; reset, saturating snoop ranges and all replacement
states have direct focused proof. The selected 486 page walk now omits writes
when A/D requirements are already satisfied; a 32-case 486/386 matrix and
failed-update checks pass with TLB/alignment/RMW regressions, 4/4 per width.
The original second-read/locked-update rule remains unimplemented and explicit,
not discharged by this smaller correction. S5/T560 stay open; no complete P,
whole-S qualification or new release is claimed.

The next S5 increment connects second reads to the paired Core transaction
boundary. Changed directory/table values select actual new targets and
attributes; begin/read/write failure completes the acquired pair. Real PF
frames prove second-value presence and supervisor-WP checks without target
access. Six focused cases per width pass, with the final cache test rebuilt
per width after correcting exception-delivery fixture counts. Full privilege,
cached-entry, split-redirection and actual Core paged execution proof remains
open. This is not full C04/S5 qualification or a new release.

Further S5 sweep confirms stale-cache memory XCHG through a one-shot ignored
diagnostic. The unpublished page-specific pair is then replaced by one memory
LOCK signal boundary shared by page updates and RMW. Implicit XCHG preserves
lexical-prefix identity; one real private CPU signal controls cached-read
bypass, forced external writes and fault release. Core retains its existing
arbiter and scopes self-snoop only to each actual write. Real CPU/Core alias
and DMA exclusion tests plus the eleven-case related cohort pass per width.
The prior whole x64 baseline was 599/601 with two aliases of an outdated
metadata table; its exact expectations and retained-386 UD cases are corrected
and rechecked per width, not called a new full pass. Final whole qualification,
accepted LOCK form/fault review and eight releases remain open in S5/T560.

The final LOCK-fixed snapshot passes 601/601 repository units on both x64
and x86 in the existing release-ccache graphs. Actual handler review confirms
all accepted memory LOCK forms use the common RMW entry, with implicit XCHG
separate from lexical-prefix identity. The sequential eight profile/width
release and external-integration groups are now running. This is not yet a
complete S5 delivery; C16 and remaining CPU batches retain their T560 owners.

Final S5 executor qualification passes all eight profile/width integration
groups (35/15/17/17 per width) and rebuilds/deploys eight verified 0560 PC
EXEs. The [S5 evidence](../etc/evidence/t560-s5-cache-bus.md) records actual
diff (+828/-140 source, +2068/-427 tests), source/model grades, all final logs
and artifact hashes. Core manifests/dependency and documentation checks pass;
Shared-six, MyNES and owner inputs remain unchanged. Complete implementation
P and coordinator acceptance are required before bounded S5 closure. T560
and production 486 admission remain open.

Coordinator accepts complete pushed 33cb2efe1 after actual production,
test, interface, manifest, document and eight-artifact review against the
packet. S5 closes in its governance P. Final proof is 601/601 units per width
and eight passing final integration groups; the original graphs are restored.
The common memory LOCK replaces the unpublished page-only boundary, with
no second cache, arbiter or Shared change. Source/model grades and C16 plus
S6--S10 remain explicit in T560; production 486 admission remains closed.
