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
