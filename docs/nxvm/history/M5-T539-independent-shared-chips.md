# M5 T539: Independent Shared Chips

## Scope And Baseline

Owner admitted the first [migration proposal](../proposals/m5-shared-chip-extraction.md)
after Td S174 (996a19a17), requesting research/design before source changes.
T538 is closed; T539 remains open. Current records admission and execution status.

## S1: Boundary Research And Design

Original request: determine file/directory structure and dependencies, the
independence gaps of every current chip, required diffs and architecture
decisions that must precede implementation.

Allowed changes are NXVM documentation only. Audit all tracked files under
src/app-nxvm/devices and their actual production/test/build consumers. Retain
source anchors, a finite migration ledger and proposed responsibility map.
No production migration, shared API change, firmware import or binary rebuild.

Research delivery: [design review](../etc/architecture/t539-independent-chip-design.md)
and [81-file ledger](../etc/evidence/t539-chip-migration-ledger.md). All 81 tracked
files occur exactly once in the ledger. Inspection identifies direct peer state,
fixed PC port/board wiring, timing catalog/build coupling and mixed chip/board
tests; it does not claim a fresh instruction-semantic completeness audit.

Proposed interfaces do not become runtime authority by being written in a
research record. Source changes require the next admitted S. Decisions for owner
review: non-Intel adapter scope, one-instance PIC/DMA contracts, KBC/PPI subset
identity, CPU firmware hook/FDC unready ownership and explicit timing units.

P1 delivers admission and research documents only. Source, tests, assets, INIs,
manifests and receiving executables remain unchanged. Validation: exact 81/81
inventory comparison, diff whitespace check, NXVM documentation governance gate
and all local Markdown links in the nine delivery documents passed. No runtime tests or builds
are required or claimed for this design-only S.

## S1 Acceptance

Coordinator-role actual-change review accepted P1 `8a8a97991` on 2026-09-28:
nine NXVM documentation files only, 414 insertions and 29 deletions. Compared the
original request, admitted brief, complete file inventory and source anchors to
the delivered design; no Shared/MyNES/product-runtime changes or unsupported
hardware-completeness claims were introduced. The two governance skills kept
component ownership explicit and preserved the original handler-style constraint.

P2 records S1 closure only. T539 remains open; production batches and unresolved
architecture decisions await owner review. No other T or S is admitted.

## S2: Concrete Boundary Contracts

Continuation admitted on 2026-09-28 under the owner's implementation goal.
NXVM docs only, baseline 2197486b0. The
[contract record](../etc/architecture/t539-boundary-contracts.md) specifies
bus/signal/time/lifecycle constraints and a bounded PIT extraction. All seven
firmware provider definitions have null software-interrupt slots: remove that
unused capability at CPU extraction, not replace it. FDC media/READY and
reference-derived response remain a separately reviewed class, not a claimed
hardware fix. The original 81-file ledger remains entirely not migrated.

Owner approved the PIT Shared/NXVM first extraction through the asynchronous
review question. S2 still edits no source. Its exit is delivery of reviewable
contracts, source evidence, document links, diff check and NXVM documentation
governance. Next S may consume the approved PIT batch; other chip behavior
changes still require their appropriate review. No new executable for design.

S2 acceptance: coordinator-role review of P1 360e7d4ee confirmed the complete
design brief, seven provider definitions, distinct FDC evidence limits and
approved PIT boundary. Local links, diff check and NXVM documentation gate pass.
P2 closes this design S only; no chip has moved and T539 remains open.

## S3: Approved PIT Extraction

Continuation from 9c1eadf0d, explicitly approved for Shared and NXVM only.
The sole 8253/8254 mechanism and pure chip tests move to x86; NXVM retains
port decoding, clock conversion, IRQ0/refresh/speaker and auxiliary-PIT/D4
wiring. Original waveform logic is preserved; no timing-grade change.

[S3 evidence](../etc/evidence/t539-s3-pit-extraction.md) records preservation
review, cleanup/failure tests, dual-width units/integration, static checks and
eight 0539 receiving artifacts. Shared and NXVM use separate P commits;
Lib/Common, MyNES, sibling repositories and owner INIs remain unchanged.
Coordinator actual-change acceptance follows the delivered commits. T539
does not close with this first chip batch.

## S3 Acceptance

On 2026-09-28, coordinator-role actual-diff review inspected Shared P1
24162ac93 and NXVM P2 797ad8887 against the original approval and packet:
P1 contains only src/x86 and test/x86; P2 only NXVM receivers, build entries,
documentation and artifacts. The two original PIT files are the entire accepted
ledger batch; all other inventory dispositions remain pending, not implicitly
accepted. No duplicate chip implementation or board-state mirror survives.

The review checked the opaque contract, retained waveform bodies, transactional
seven-route attachment, sink lifetime/reset order, primary/auxiliary consumers,
preserved test cases, boundary negatives and actual EXE/INI identities. Both
governance skills enforced one state owner and cohesive original handler style,
not a new framework. No unsupported timing upgrade or new dependency was added.

One duplicated failure-diagnostic print was caught in this review and removed;
both matrix test binaries compile again. It does not alter tests' admission or
success criteria, production sources or product artifacts. Required unit,
integration, standalone chip, static and manifest results are in S3 evidence;
the document gate, 46 local links, identifiers, queue and source mapping agree.
P3 accepts S3 and removes its active packet. T539 stays open, with no next S
admitted and no claim that the remaining chip extractions are complete.

## S4: RTC Extraction

Continuation from 803c9d019 under the owner's 2026-09-28 automatic-S admission.
Shared x86 owns the opaque MC146818 mechanism; NXVM retains index/NMI,
PIC binding, seed/checksum and clock/provenance. The two old RTC files leave
the product tree. Malformed-month array bounds are contained at their sole
owner without a new hardware-accuracy claim.

[S4 evidence](../etc/evidence/t539-s4-rtc-extraction.md) records preserved
functions, migrated tests, all-month sanitizer proof, full dual-width units
and integration, single-pass profile boots and eight receiving artifacts.
Shared and NXVM are separate implementation P targets; coordinator actual-change
acceptance follows the complete deliveries. No other chip is accepted by S4.

S4 coordinator acceptance reviewed Shared 8a8435648 and NXVM 06f99605d against
the contract and finite ledger. Opaque ownership, board wiring, allocation
rollback, reset/destruction, register-read side effects and all callers were
checked in the actual diff. Full units are 339/339 per width, independent chip
suites 10/10, default integration 20/20, and every other profile/width boot
passes once. All eight 0539 artifacts are current; INIs and other products are
unchanged. The bounds correction is safety containment, not a timing upgrade.
S4 closes with no remaining item in its bounded brief; T539 stays open.

## S5: PIC Extraction

Automatically admitted from be86dee2e. Shared x86 owns one opaque 8259;
NXVM pic_bus owns ports, source counts and cascade wiring. All callers and
tests migrate without a second PIC implementation. The initial allocation
failure regression was repaired at machine construction and covered by the
original test plus all eight PIC port-registration rollback/retry cases.

[S5 evidence](../etc/evidence/t539-s5-pic-extraction.md) records both-width
341/341 units, 20/20 default integration, 12/12 independent chip tests,
single-pass remaining profile boots, manifests/gates and eight artifact hashes.
Shared d6dc6ca3a and NXVM 6cf3cee40 are pushed. Coordinator inspected their
actual ownership, priority/cascade/poll, lifetime, rollback and caller/test
changes; no outstanding gap remains in the bounded brief. S5 is closed;
T539 remains open and no remaining chip batch is implicitly accepted.

## S6: DMA First-Service Prerequisite

Automatically admitted from 4f278a918 for NXVM only. Before opaque extraction,
source review found that secondary channels perform their first transfer directly
in arbitration rather than through the normal/compressed phase handler. S6
removes that bypass and covers the complete 126-case first-service family.
[Evidence](../etc/evidence/t539-s6-dma-first-service.md) records the Intel source,
negative control and remaining DMA extraction boundary. Full dual-width units
(341/341), default integration (20/20), all other profile boots once, gates and
eight receiving artifacts pass. NXVM 0746220bf is pushed. Coordinator-role
actual-diff review accepts the one removed bypass, 126-case fixture, preserved
original tests, corrected source claim and artifact identities. No Shared
implementation or other product changes are part of S6. The active packet is
removed; S6 closes and T539 remains open for the pending DMA extraction.

## S7: DMA Extraction

Automatically admitted from 0ceb739f6. The [boundary review](../etc/architecture/t539-s7-dma-boundary.md)
separates one opaque 8237A controller from NXVM's page/lane and paired-bus
integration. The old chip implementation and accelerated advance path are
removed; all callers and original behavioral scenarios migrate to public
registers, copied signals and the real phase path. Construction failures roll
back port publication and chip ownership without hiding an earlier error.

[Evidence](../etc/evidence/t539-s7-dma-extraction.md) records 342/342 units,
20/20 default integrations and 13/13 independent chip tests per width, every
remaining boot once, eight verified artifacts, all six manifests and gates.
Production C/H net -49; tests net +301. INIs, MyNES and sibling repositories
are unchanged. Shared 53b4be21d and NXVM 217125697 are pushed. Coordinator-role
review inspected the actual chip/board split, callback ownership, transfer
failure/TC/reset ordering, sparse ports, paired grants, original-test mapping,
construction rollback, negative boundaries and receiver identities. No item
remains in S7's bounded brief. S7 closes; T539 remains open for the remaining
finite inventory and the next automatic admission is the AT keyboard chain.

## S8: AT Controller, Keyboard And AUX Extraction

Automatically admitted from 23b732201. The [boundary review](../etc/architecture/t539-s8-kbc-boundary.md)
separates three opaque Shared owners and NXVM's port/IRQ/A20/reset attachment.
No full 8042 MCU or timing upgrade is claimed. Original command/BAT/reply,
parameter-interleaving and serial/typematic order is preserved; private-state
test/dump access is removed without adding a test-only getter.

[Evidence](../etc/evidence/t539-s8-kbc-extraction.md) records 345/345 full units,
20/20 default integrations and 16/16 standalone chip checks per width, six
other boots once, eight verified artifacts, all manifests and static/doc gates.
Production C/H net +486; tests net +764. INI contents and other products are
unchanged. Shared eb1e2e208 and NXVM 6ae9802dc are pushed. Coordinator-role
review checked the real chip/adapter, construction rollback, callback ordering,
test/diagnostic migration, build/manifest and artifact diffs against the packet.
No S8 item remains; S8 closes and its active packet is removed. T539 remains
open for the remaining finite inventory, with XT PPI/keyboard next.

## S9: XT PPI And Serial Keyboard Extraction

Automatically admitted from 625ea7054. The [boundary review](../etc/architecture/t539-s9-xt-boundary.md)
separates qualified PPI registers and serial/BAT state from NXVM board wiring.
The old keyboard pair and duplicated PPI latches are removed. Negative controls
expose and repair refused-byte counter wraparound, lost registration failure,
non-progressing inhibited deadlines and direction/BAT release gaps through
the same existing owner paths, without a new framework or timing claim.

[Evidence](../etc/evidence/t539-s9-xt-extraction.md) records final 347/347 units,
18/18 independent checks and 20/20 default integrations per width, all other
profile boots once, eight verified artifacts and the static/manifest/doc gates.
Production C/H net +106, tests net +294; INI contents and other products are
unchanged. Shared 0f9c6b1a8 and NXVM 31e759965 are pushed. Coordinator review
accepts the actual owner, callback, rollback, original-test, build, document
and artifact diffs against the complete packet. No S9 item remains. S9 closes
and its packet is removed; T539 remains open, with FDC next automatically.

## S10: FDC Extraction Prerequisite

Automatically admitted from 02931b886. [Decision record](../etc/architecture/t539-s10-fdc-boundary.md)
checks the archived Intel original and read-only PCjs/86Box logic, maps every
command/readiness consumer and specifies the chip/drive/board boundary. One
120-case characterization matrix adds 106 test lines; no production or Shared
change. Full units pass 347/347 per width; documentation, local links and
whitespace checks pass. No executable-input change requires a new artifact.

P1 7fa0f75d5 is pushed. Coordinator actual-diff acceptance closes this bounded
prerequisite, not FDC migration. The source contradicts Intel SEEK/READY rules
and has pending-completion identity/capacity risks; those remain explicit T539
cutover gates. S11 automatically begins the pending seek/completion repair.

## S11: Pending FDC Operation Ownership

P1 a714e194b repairs command-buffer identity leakage and bounds outstanding
seek completions through admission, not result eviction. Production net +16
lines; tests net +144. [Evidence](../etc/evidence/t539-s11-fdc-seek-ownership.md)
records reproduced negative controls, 347/347 units and 20/20 default
integrations per width, six other profile boots once, eight artifact hashes
and static/document/manifest checks. Shared, MyNES and INI are unchanged.

Coordinator-role actual-diff review accepts operation ownership, capacity
proof, unsupported-sequence qualification, reset/SIS coverage and receiver
evidence. S11 closes after its complete P1 push; it does not close FDC
extraction. S12 automatically admits drive-input/status qualification and
repair under the same T539; its bounded packet is in Current.

## S12: FDC Drive/Status Qualification And Embedded Firmware

The owner expanded this prerequisite to recover project-owned BIOS source and
embed each selected machine's firmware at build time. Raw vendor ROMs remain
external; the owner explicitly authorizes the eight embedded-ROM EXEs in the
product artifact commit. No Shared, MyNES or INI change is included.

The complete implementation removes the DeskPro-specific unready policy and
runtime ROM file routes, separates READY/media and physical head/PCN, repairs
source-confirmed status/SCAN/TC behavior, and reconnects the guest default BIOS
through real FDC/DMA/PIC services. A verification-discovered stopped-HLT race
is repaired at the NXVM wait boundary, with deterministic before/after proof.
[S12 evidence](../etc/evidence/t539-s12-fdc-drive-status.md) records source
pages, changed expectations, eleven guest BIOS scenarios, 350/350 units per
width, default integrations 20/20 per width, six vendor boots once, runtime
input-denial probes, static/manifests, code-size review and eight final hashes.

P1 408a31cc7 is pushed. Coordinator actual-diff review accepts the complete
bounded batch, including source qualification, removed duplicate loader routes,
guest firmware ownership, stopped-HLT cancellation and verification limits.
The post-commit specialized aggregate passes and all eight deployed hashes
still match the evidence. S12 closes and its packet is removed; FDC extraction,
other chips and T539 remain open under the finite migration ledger. Reusable
build trees remain needed for the immediately following FDC extraction.

## S13: Independent 8272A Chip

Shared P1 1d6dc5876 and NXVM P2 c18859898 are pushed. The command/PCN/cause
owner and independent tests move to x86/devices/fdc8272; NXVM retains PC
registers, physical drive/record services and PIC/DMA/port wiring. Private
scheduler/diagnostic access and the old command implementation are removed.
[Evidence](../etc/evidence/t539-s13-fdc-extraction.md) maps original tests,
actual-diff corrections, failure rollback, 355/355 units and 20/20 default
integrations per width, six final vendor boots once, tools-off 22/22, static
gates, six manifests and eight final EXE hashes. Counted source/test/build
change is +3300/-2297, net +1003, excluding manifests/docs/artifacts.

Coordinator actual-commit review accepts the complete bounded FDC row,
including command-entry recording qualification, distinct READY triggers and
the unchanged flat-record/timing limits. No new silicon-completeness claim.
The active S13 packet is removed on acceptance; T539 remains open for CPU/FPU,
HDC, video and every remaining ledger disposition. MyNES link inputs, its
0043 artifacts, external originals and owner INIs are unchanged.

## S14: Independent HDC Family

Shared P1 9020d8bba extracts the opaque ATA/Compaq/WD1003/Xebec controller
and independent tests. NXVM P2 reconnects ports, media IDs, board timing and
PIC/DMA signals; removes private diagnostics and duplicate chip tests; and
delivers eight embedded-ROM EXEs. [Evidence](../etc/evidence/t539-s14-hdc-extraction.md)
maps original cases, semantic corrections, 358/358 units and 20/20 final
default integrations per width, six vendor boots once, tools-off 25/25,
six manifests and final artifact identities. Source/test/build delta is
+2572/-1580, net +992; the record explains independent-boundary and test costs.

Final diagnostic review caught unsafe live reads and stop-induced counter
reset. Terminal Windows probes now confirm pause and join before sampling;
the checkpoint retains its ATA predicate. The optional setup probe is built,
not claimed runtime-qualified. No production lifecycle change, timing upgrade,
ESDI addition, owner INI, external master or MyNES artifact change is included.
P1 9020d8bba and P2 86e0f82cb are pushed. Coordinator actual-commit review
accepts the complete HDC row, including the diagnostic pause/join correction,
original-case ownership, unchanged silicon limits and target-separated delivery.
The S14 packet is removed; T539 remains open for the remaining finite ledger.

## S15: Independent Video Family

The sole opaque register/VRAM/raster/frame owner moves to x86/devices/video.
NXVM retains port/physical-memory attachment, copied presentation adaptation
and failure-atomic construction. Dead presentation helpers and their isolated
test are removed after caller proof. [Evidence](../etc/evidence/t539-s15-video-extraction.md)
maps original cases, actual semantic-boundary review, 367/367 units and 20/20
default integrations per width, six vendor boots once, tools-off 43/43, six
manifests and eight artifact hashes. Source/test/build delta is +5362/-3957,
net +1405; independent tests account for most growth. Shared P1 522d0b27f is
pushed; NXVM P2 88ae417ff delivers receiver source, artifacts and evidence.
Coordinator actual-commit review accepts the complete video/display batch,
including original-case mapping, the optional VGA route, typed backing access,
construction rollback and the caller-proven dead-helper retirement. Post-delivery
standalone tests pass 43/43; final artifact hashes and untouched INIs agree.
S15 closes and its active packet is removed. CPU/FPU and remaining ledger rows
stay in T539; the next automatic admission still requires its boundary review.

## S16: Independent FPU

Shared P1 5fa831a2b and NXVM P2 9783297fb are pushed. The sole opaque
Types-only FPU owns existing stack/status and completion intervals; CPU owns
pairing/operand cycles and the machine owns instance lifetime. Old files and
private consumers are removed. Original arithmetic/timing algorithms remain;
independent tests receive chip-local assertions without losing machine cases.
[Evidence](../etc/evidence/t539-s16-fpu-extraction.md) records 368/368 units
and 20/20 default integrations per width, tools-off 44/44, all six manifests,
specialized/document gates, eight artifacts and source-package net +211 lines
including test/build growth. The paused EDIT assertion now compares one frozen
state instead of a cadence-cached prior frame.

XT/AT boots pass once per width. Model40's initial per-instruction diagnostic
probes hit 90 seconds; one controlled contrast per width disables only that
existing observer and reaches the same installer-ready predicate in 75.39 s
and 88.77 s. Inputs, production source, time limit and guest execution remain
unchanged; original failures stay visible. Coordinator actual-commit review
accepts the complete FPU row and these verification limits. S16 closes and its
packet is removed. CPU and all remaining ledger rows stay in T539.

## S17: CPU Observation Prerequisite

Shared P1 cadaf0990 and NXVM P2 f85888d3e are pushed. CPU preview and timing
descriptor reads now use one physical resolver with explicit observation intent;
display backing capture follows it, while operational reads retain effects.
Shared video calculates CPU-visible bytes without publishing read latches for
inspection. [Evidence](../etc/evidence/t539-s17-cpu-observation.md) records every
provider/caller disposition, new tests, original-handler preservation, 370/370
units and 20/20 default integrations per width, tools-off 45/45, six vendor
boots once, manifests/gates and eight 0539 artifacts. Source/test/CMake net
growth is 283 lines, mainly bounded observation regressions.

Coordinator reviewed the actual P1/P2 changes and accepts this prerequisite.
The new test's missed constructor classification was repaired; no gate was
removed. Both Model40 boots meet the unchanged 90-second bound with the existing
observer-free probe. INIs, external masters and MyNES remain unchanged. S17's
packet is removed; opaque CPU extraction and all remaining ledger dispositions
stay in T539, not transferred or declared complete.

## S18: Complete CPU Extraction

Admitted at 25ec0f6c3 under the owner's automatic-S authorization. The
[boundary](../etc/architecture/t539-s18-cpu-extraction.md) consumes all nine CPU
files and their callers/tests, rather than closing another prerequisite-only
batch. Implementation and receiving verification are pending; Current owns the
active packet. No CPU migration or artifact result is claimed by admission.

### Owner Replanning, 2026-09-29

The owner requested smaller S deliveries because the original CPU packet was
not trackable. [S18-S32](../etc/architecture/t539-cpu-work-packages.md) supersede
its all-in-one delivery scope. S18 is still unaccepted and now first recovers a
green incremental baseline; existing uncommitted work and narrow test results
remain in its evidence. This amendment neither closes the CPU row nor waives
full-unit gates. Later S packages are planned; Current owns their admission.

### S18 Baseline Acceptance

NXVM implementation P1 `0067d80c4` was pushed and reviewed as the amended
incremental-baseline delivery. Coordinator review accepts S18, not the complete
CPU extraction. The full-unit baseline is green (370/370 per width), default
integration is 20/20 per width, standalone tools-off is 45/45, and six vendor
boots pass once. Eight updated 0539 artifacts, manifests, specialized gates and
documentation checks are recorded in S18 evidence. Shared/MyNES/INIs are
unchanged. The inventory preserves deferred work and assigns all remaining
consumers to S19-S32; S24 owns the unresolved 32-bit BOUND observation.
The S18 packet is removed. T539 stays open; S19 is the next planned package.

## S19: CPU Bus Boundary Acceptance

NXVM P1 `f1b43af46` qualifies the retained S18 bus without production changes.
Coordinator actual-commit review accepts the memory/port failure matrices,
observation-only behavior, FPU trace and cascaded INTA rejection/retry proof,
and the nine-file gate with 72 negative controls. Existing reset/prefetch,
operand-completion and interrupt-frame tests remain. Full units pass 371/371
per width; all 66 specialized steps, six manifests and documentation pass.
The newly direct-constructed PIC test is explicitly classified, not exempted.
The six implementation test/build files add 334 and remove 9 lines. EXEs,
production code, Shared corpus, MyNES and INIs are unchanged. The
[evidence](../etc/evidence/t539-s19-cpu-bus-boundary.md) maps each callback and
requirement to code/tests. S19 packet is removed; S20 is next and T539 remains
open. This accepts the bus package, not whole CPU extraction.

## S20: CPU Observation And Adapter Acceptance

NXVM P1 `af06a6259` qualifies the retained S18 observation adapters, removes a
board test's unnecessary CPU-private include, proves snapshot lifetime across
debug mutation/reset and adds five board-access negative controls. Coordinator
actual-commit review accepts S20: all 371 units pass per width; 66 specialized
steps, six manifests and documentation checks pass. The four test/gate paths
add 57/remove 2 lines; production, artifacts, INIs, Shared and MyNES are
unchanged. [Evidence](../etc/evidence/t539-s20-cpu-observation-adapters.md) maps
entry/current, decode/fault, debug, FPU and observer-free paths. The active S20
packet is removed; S21 is next under automatic authorization. Remaining raw
instruction-test consumers and embedded lifetime keep their S21-S31 receivers;
whole CPU acceptance remains S32, and T539 stays open.

## S21: LEA/MOVX Consumer Migration

The intake review split the former 18-file, 11,335-line package into S21-S29;
only unadmitted subsequent packages shift to S30-S40. The current work plan
and inventory own that mapping; earlier entries retain historical numbering.
S21 separates all 53 original LEA/MOVX cases into CPU-owned instruction tests
and public-operation board composition tests. Eight intended MOVX opcode cases
supplement the eight retained original ModR/M sequences. No CPU implementation,
timing grade, Shared corpus, MyNES, INI or executable input changes.
[S21 evidence](../etc/evidence/t539-s21-lea-movx-migration.md) records the fixture
mechanism differences, full verification and size accounting. Executor delivery
is ready for coordinator actual-commit review; acceptance is not yet claimed.

### S21 Acceptance

Coordinator actual-commit review accepts pushed NXVM P1 `1049b9021`. Both widths
pass all 373 units; 66 specialized steps, six manifests and documentation/diff
checks pass. The evidence preserves a failed concurrent desktop run and the
passing isolated x86 full run; its unproven concurrency cause is a TODO, not a
claimed repair. Ten test/build paths add 626/remove 499 lines. The packet is
removed; S22 is next, and T539 remains open through the planned CPU receiving
audit. No production, asset, INI, Shared or MyNES change was required.

## S22: GPR MOV/MOFFS Consumer Migration

Admitted at S21 P2 bf8dd127c under automatic authorization. The 277 original
contexts now have 269 CPU-owned and eight board-owned receivers, retaining
tables, profiles, register/memory rollback, protected DF and real PIC frames.
Public copied CPU access replaces board-private setup; the two original
post-fault physical RAM checks remain because the public memory API excludes
FAULTED state. No public contract or production behavior changed.
[S22 evidence](../etc/evidence/t539-s22-mov-moffs-migration.md) records actual
case mapping, full verification, limits and source/test accounting. Executor
delivery is ready for actual-commit review, not yet coordinator acceptance.

### S22 Acceptance

Coordinator actual-commit review accepts pushed NXVM P1 `9e5382872` with all
277 original contexts retained. Complete units pass 375/375 on each width;
66 specialized steps, six manifests and documentation/diff checks pass.
Ten test/build paths add 1,040/remove 1,174 lines. No production or EXE input
changes. The active packet is removed; S23 is next. The remaining 96 original
private CPU consumers retain explicit receivers; T539 remains open.

## S23: XCHG Consumer Migration

Admitted at S22 P2 8dd52d43f under automatic authorization. The twelve original
test families retain 101 instruction contexts: 96 CPU-local and five real
board fault/IRQ cases. Public register operations/copied observations replace
board-private setup; protected segment attributes now come from guest GDT
loads, not private cache writes. Production, Shared and artifact inputs do
not change. [S23 evidence](../etc/evidence/t539-s23-xchg-migration.md) records
all receivers, verification and scope. Complete units pass 376/376 per width;
executor delivery awaits actual-commit coordinator review.
