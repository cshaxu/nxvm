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
