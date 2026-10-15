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

S6 is admitted after clean a14f4420b under the owner's existing automatic
sequential Core authority. It consumes C01/C10/C13/C14 and S3's carried
exception-entry alignment/nesting matrix. Source and current implementation
review precede production changes; the complete feature/exception batch,
dual-width qualification and affected eight releases remain its exit.
Shared-six, MyNES and owner inputs remain excluded. No partial P is created
for admission or source investigation.

S6 initial audit confirms that guest CR4/VME and GD remain missing and that
SX ESC with EM/TS clear falls through to the old FPU path. The original PRM
requires NM for absent-FPU SX. One existing-handler condition (+2/-1 source)
repairs this; the expanded existing FPU test (+139/-0) proves 12,288 actual
mode/control/prefix/register-memory cases per width, including real NM frames
and VM86 saved state. The test corrects its IVT-versus-operand counter and RF
fault-frame expectations without production compensation. WAIT/fetch/LOCK,
Core binding, VME/debug/exception and full S6 qualification remain open.
Public Intel 253669-087US is archived externally for VME research; no NDA
supplement or third-party source is imported. No partial P or new release.

S6 continues with actual CR4/VME flags and focused dual-width proof. Source
review of public Intel 325383-078US corrects an investigation hypothesis:
VME does not admit low-IOPL IRET. Existing return owners preserve VIF/VIP
where required; selected-SX low-IOPL GP retains the instruction-start
checkpoint instead of overwriting it after fetch. Eight preservation and
384 actual GP cases pass; the latest related cohort is 6/6 per width.
Core manifests, dependency boundaries, documentation and whitespace checks
pass. Software INT redirection, GD/exception, WAIT and whole-S6
qualification remain open. No partial P, artifact or Windows support claim.

S6 adds software INT imm8 redirection at its existing CPU owner, with one
private TSS/IVT/frame helper (+56/-1 production) and actual-path test expansion
(+111/-0). All 256 vectors across flags/IOPL/prefix/code forms yield 32,768
normal cases; lower-IOPL set-bit GP and protected INT/INT3/INTO frame proofs
also pass. Related cohorts are 7/7 on both widths. Invalid-map and paged/failing
access ordering, hardware interrupt admission and the remaining S6 rows stay
open. No Shared, MyNES, owner-input or asset mutation; no partial P.

S6 implements selected-SX GD protection at common MOV DR decoding and GD
clearing at the actual DB exception entry (+11/-1 production). Deferred
debug traps stop borrowing IRQ delivery and share exception entry. The
existing debug test grows +109/-0 for 384 actual fault forms and two trap
cases; related cohorts pass 5/5 per width, including retained earlier debug
cases. A test-only code/IVT overlap and nonexistent array macro are corrected
without production compensation. Full debug priority/failure, INT failure
and paging, WAIT/Core and whole-S6 qualification remain open. No partial P
or new artifact; full T560 scope and production admission gate are retained.

S6 confines external TEST/pending-FPU WAIT to earlier personalities after the
unchanged TS-and-MP NM check (+7/-8 source). Existing FPU tests add 768 SX
WAIT cases (+63/-0); debug tests add 72 LOCK/privilege competing-fault cases
(+50/-0). Both widths pass the four-test cohort including actual Core's
retained early-family TEST/ESC wait. That does not prove SX Core binding.
The original-versus-modern MOV mod-field wording is explicitly retained for
form review rather than silently altering decode. Remaining S6 rows, full
qualification and releases remain open; no partial P or Shared mutation.

S6 qualifies SX absent-FPU through actual Core using the existing pending-CPU
test binding pattern without opening production admission. The FPU Core test
adds +55/-6 for parameterized NM proof, 128 SX ESC and 16 WAIT control cases.
The CPU test adds 6,144 illegal-LOCK ESC cases, retaining actual UD frames and
no FPU publication. Both widths pass the five-test cohort including the closed
SX production gate. The full-S6 test estimate is revised to +1000--1500
/-20--80 before remaining failure/paging/exception work; original scope and
all final qualification/artifact obligations remain unchanged.

S6 extends the existing decode-admission fixture and table (+95/-2 test,
no production edit) for 8,192 real-mode failed-byte cases across its feature
instruction group. An explicit prefetch-injection control fixes the initial
test's failure to reject bytes supplied by actual SX prefetch. Full CPU
rollback and absence of data/FPU/port publication remain strict; old cases
keep their original injection. Related cohorts pass 4/4 per width. Paging,
protected/VM86 priority and remaining exception proof are not inferred.
No partial P, full qualification or new artifact is claimed.

S6 adds 96 paged VME INT cases (+119 test lines), exposing and correcting
the new helper's whole-frame paging/early-IVT order (+5/-5 production).
The existing segment-only frame owner and scalar pushes now follow Intel's
FLAGS/CS/IP-before-IVT contract. Tests prove supervisor TSS versus user
IVT/stack permissions, exact PF frames/CR2, all accepted stack writes on IVT
failure, and only the FLAGS prefix on later cross-page failure. Related
cohorts pass 5/5 per width, then final GPR/nine-field checks pass per width.
Remaining malformed-map/provider/segment and exception obligations stay
explicit. No new state, Shared mutation, partial P or release is claimed.

S6 audits all flags/CR4 writers and removes a selected-SX debugger bypass.
Private canonical mask/normalization is shared with instruction loading
(+27/-19 production); the old local mask owner is deleted. VME tests add
68 debugger samples and include EFLAGS in atomic invalid-CR4 patches
(+27 test lines). Both widths pass seven-test cohorts with retained task
regressions. Actual selected-SX task and whole failure/exception qualification
remain open; no partial P, new public interface or artifact is claimed.

S6 expands the existing task-completion table (+23/-7 test only) rather than
duplicating TSS setup. The older matrix already covered SX RF/TF/AC/ID;
new VIF/VIP, reserved input, outgoing-save and CR4-retention assertions bring
the actual count to 2,560 SX plus 80 retained 386 cases, zero failures on
both widths. Related cohorts pass 3/3 per width. Complete failure/exception
and final S6 qualification remain open; no blanket alignment exception,
production change, partial P or artifact is added in this phase.

S6 adds same-CPL entry proof in the existing alignment fixture (+76 test
lines, no production edit): 128 INT/UD, word/dword gate, residue, AC and
prefix cases. Valid kernel AC delivery proves fault IP/RF and nested-entry
handling. A wrong new physical-write-count assertion is replaced by exact
frame/guard-byte proof; zero guest writes remains strict on AC. Both widths
pass four-test cohorts. No blanket AC exemption or framework is introduced;
remaining IRQ/failure/nesting and final S6 qualification stay open.

S6 adds 96 VME provider failures at six TSS/frame/IVT stages (+101 test
lines, no production edit). Each failed transfer is attempted once, full CPU
state rolls back, and only accepted stack prefixes remain without replay.
Both widths pass three-test cohorts. The finite remaining R1--R7 qualification
inventory is explicit in evidence, retaining all admitted feature/exception
requirements and final release duties. S7--S10 are not narrowed or discharged.

S6 refines R1/R6 direct forms: 512 normal CR4 and 1,152 LOCK/privilege
rejections plus valid debugger/reset readback, with a shared complete fault
checker (net +60 VME test lines). The original FPU LOCK table now covers
13,056 WAIT/register/memory ESC combinations (+16/-4 test lines), including
denied DS operands without a FPU callback. Both widths pass the final
four-test form/Core cohort. Production remains unchanged in this phase;
failure/priority/source review and final whole-S6 release qualification remain.

S6 resolves DR4/5 through the archived Intel retrospective historical alias
contract: three production mapping lines, 512 alias read/write forms and
4,096 full-GPR GD cases. Both widths pass four-test related cohorts.
R5 reuses existing exception tests (+28/-15 test, no production): 1,321
classification decisions plus actual SX GP/PF serial/DF delivery with exact
CR2 and frames. Initial inherited 386 expectations were inapplicable to two
SX missing-descriptor-page cases; expectations now follow PRM 9.9.8 and the
actual absent descriptor page, without changing production or qualifying the
inherited 386 implementation. Both widths pass four-test cohorts. Manifests,
Core dependencies, documentation and whitespace gates pass. Whole S6 and
T560 remain open; no partial P, new EXE or desktop qualification is claimed.

S6 reuses existing debug matrices for SX RF/TF/breakpoint/shadow/IRET proof
(+39/-34 test lines), retains fixed SX DR6 reset bits in assertions, and
extends existing entry-provider/external-error tests (+20/-14). Another 64
actual NMI/INTR VME cases (+73 test lines) prove IDT routing independent of
software redirection and VIF; full VM86 frames and physical-IF gating pass.
Both widths pass final four-test cohorts in the original cached graphs.
No production change or new public boundary is added in that proof phase.
The following source reconciliation restores T559's established POPF/RF
preservation contract: original 486 PRM and selected embedded developer manual
exempt POPF from clearing; later generic wording is not selected-SX evidence.
Remove S6's unsupported two-line clear and mirrored expectation; existing
POPF RF/TF tests now include SX (16 RF/32 TF total), alongside the VME matrix.
Remaining S6 and T560 duties are not discharged by these focused tests.

S6 adds six-mode fetch proof (+20 existing test lines), 256 valid map-edge
and 160 stack-edge VME cases. Nonzero guest ESP high bits expose a real SX
32-bit interrupt-entry publication bug; the existing owner now loads full
TSS ESP rather than preserving stale high bits when the target stack B bit
is clear. Original Intel486 VM86 INT algorithm supplies the contract. No API,
helper or state is added. Both widths pass final four-test cohorts. Same-shape
call-gate/outer-return publishers remain explicit R2 obligations, not silently
qualified by the IRQ case. Full S6 qualification/release is still pending.

S6 reconciles original eSP notation before changing those publishers: original
CALL/outer RET/IRET select SP or ESP by B, unlike full-ESP VM86 interrupt entry.
SX outer returns now have direct retained-fixture proof. Another 32 call-gate
cases expose wrong operand-width dispatch; SX now chooses width from the
already-read gate descriptor (+3/-1 source), using the same existing handlers.
The outer test grows +78/-9 lines without another fixture. A 512-case protected
IRET VIF/VIP permission matrix passes with no flags production change. Both
widths pass six final related tests. Remaining source/failure/task and complete
S6 release obligations stay open; no partial P or new release is claimed.

S6 extends existing word-task proof (+25/-6 test lines): 1,024 SX plus 48
retained-family flag cases, including stale high bits, reserved/fixed bits,
CR4 retention and exact outgoing word FLAGS. Another 64 NOP cases qualify
the VIF/VIP instruction-boundary GP, while hardware routes add valid VIP
states (+35 VME test lines). A mistaken partial-source reading briefly removed
the mandatory boundary locally; the next Intel paragraph resolved the error
and the original production guard was restored before delivery. Both widths
pass seven final tests. External references are read-only, hash-pinned research,
not imported source or proof of every malformed-map classification. Remaining
source/entry and whole-S6 release duties are unchanged.

S6 adds 320 malformed-map guard cases using the existing VME failure fixture
(+46 test lines, zero production): direct GP delivery and no guest stack/IVT
effects, even with competing stack or provider failure. Both widths pass seven
related tests. Evidence explicitly separates reference-supported L2 short/bounds
guards from a conservative low-base guard without fitted-silicon proof. Neither
a passing test nor a reference implementation is treated as a hardware upgrade;
remaining source/entry reconciliation and whole-S6 delivery remain open.

S6 reuses the alignment table for actual same-CPL INTR/NMI, bringing it to
256 entry cases (+25/-1 test lines). Both widths pass eight related tests.
Actual-diff review restricts full TSS ESP publication to SX VM86 origin:
ordinary protected entry retains original eSP/B semantics, now directly
checked by the existing IDT fixture on 386/SX with nonzero ESP high bits
(+18/-14 test lines). Source state/API ownership is unchanged. Whole dual-width
repository units are started as a checkpoint, not prematurely counted as
passed or as the complete S6/release exit.

S6 source reconciliation removes the unsupported blanket low-I/O-base guard
(-2 production lines), retaining one selected-byte offset/limit path and
explicit L2 malformed-input arithmetic. Literal map edges, invalid cached-TSS
shutdown paths, outer IRET virtual-flag permissions and CPL0 VM86-return frames
add actual-path proof. Full writer/Core-mode review reconciles R1--R6; exact
fitted silicon is not guessed. Final CPU production is +199/-56.

Specialized gates reject misplaced private CPU binding in the public FPU
fixture; its complete admission contents are restored and actual SX proof
moves into the existing internal Executor test, without gate relaxation or
new API/file/registration. A separate stale Jcc observation assertion left
behind by S5 is updated +2/-1 while preserving the early inspect return and
all other checks. First failures and their corrections remain in evidence.

The final ownership snapshot passes 602/602 whole units per width and both
complete specialized aggregates. All eight profile/width integration groups
pass and eight verified stripped 0560 PC EXEs are deployed; original graphs
are restored to default. Public-six, MyNES and owner INI/media/ROM/snapshot
inputs are unchanged. [S6 delivery evidence](../etc/evidence/t560-s6-feature-exceptions.md#final-implementation-delivery-candidate)
contains counts, terminal logs, source/model limits and final artifact hashes.
This is the complete implementation candidate pending pushed-P review; S6
acceptance and later S7--S10 completion are not inferred from these results.
