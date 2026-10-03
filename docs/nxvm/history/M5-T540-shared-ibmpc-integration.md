# M5 T540: Shared IBM PC Board Integration

## Scope And Baseline

The owner admitted the queue's first successor to T539 at `0412c9ffa`.
T539 extracted independent chip mechanisms to `src/x86/devices`; T540 now
examines reusable IBM-PC board assembly without turning distinct machines into
one profile-driven pseudo-board.

## S1: Board-Graph And Reuse Audit

S1 is documentation-only. It inventories XT, IBM 5170, DeskPro Model 40 and
default PC/AT construction, retained board adapters, tests and build targets.
It may define later bounded source packages only after distinguishing actual
shared mechanism/lifetime from coincident port numbers or chip selection.
No source, interface, firmware, asset, INI or executable change is admitted.

## S2: Retained Adapter Ledger

S2 expands the family decision into a finite adapter ledger.  It distinguishes
the App-owned generic x86 machine executor from IBM-PC board attachments: the
former has a required neutral `x86/core` receiver before independent Apps can
be real; the latter may move one proven mechanism at a time to
the flat `x86/ibmpc-common`, `x86/ibmpc-at` and `x86/ibmpc-xt` components. It
introduces no source directory, ABI or runtime
change.  Its completion record identifies the first safe source batches and
the profile regressions that remain product-owned.

## S74: Firmware Publication Boundary Accepted

P1 `7997202a6` moves the sole firmware binding/rollback transaction to neutral
Core and replaces board ROM-table borrowing with bounded copied coverage and
atomic window publication. P2 `eb6e1db17` closes the whole-production gate
coverage gap. Actual pushed-diff review accepts both deliveries.
[S74 evidence](../etc/evidence/t540-s74-firmware-publication-boundary.md)
records complete dual-width 470/470 units/gates, standalone Core linkage,
eight one-shot boots and the eight stripped 0540 artifact identities.
Shared, MyNES, INIs and external master inputs remain unchanged. Attachment,
remaining scalar boundaries, test classification and physical source relocation
are still due; this S acceptance does not close T540.

## S75: Lifecycle Observation And READY Ownership Accepted

P1 `b39d60c31` replaces all six board input/display private lifecycle reads
with the existing copied Core operation and moves both unchanged READY bodies
to the neutral scheduler. Actual pushed-diff review accepts the complete
eighteen-file NXVM-only delivery. [S75 evidence](../etc/evidence/t540-s75-lifecycle-ready-boundary.md)
records dual-width 470/470 units, specialized gates, eight independent neutral
link proofs, eight one-shot external boots and optimized stripped 0540 hashes.
Shared, MyNES, INIs and external master inputs remain unchanged. The complete
timing publication/observation receiver, attachment, test classification and
physical source relocation are still due; T540 remains open.

## S76: Timing Publication Boundary Accepted

P1 `8a4e3bd12` puts complete copied-table validation/publication under neutral
Core and supplies qualification on the existing board deadline callback.
Actual pushed-diff review accepts all 22 NXVM-only paths. [S76 evidence](../etc/evidence/t540-s76-timing-publication-boundary.md)
records both-width 470/470 units/gates, eight neutral-link executions, eight
one-shot external boots and optimized stripped 0540 identities. Shared,
MyNES and owner INIs are unchanged. S77 receives Running Port-B time through
an actual I/O-cycle input contract; attachment, direct-test classification and
physical relocation remain required. T540 remains open.

## S77: Port Read-Cycle Time Input

The sole typed read contract now receives Core guest tick by value. CPU bus,
paused bus/Debug and bounded firmware supply their own Core clock at dispatch;
ordinary, byte-lane and wired-OR routes use the same value. Both existing
Port-B callbacks use that input instead of borrowing the private clock. All
twelve production and twenty-eight synthetic callbacks are reconnected;
write callbacks, register algorithms, lifecycle guards and timing grades are
unchanged. [S77 evidence](../etc/evidence/t540-s77-port-read-time-input.md)
owns the source review, regression and artifact proof. This delivery does
not close T540's attachment, direct-test classification or physical relocation.

Coordinator accepts pushed P1 `bd0256b47` after actual 58-path diff review,
dual-width 470/470 units/gates, eight neutral executions and eight one-shot
external boots. Pure-governance P2 closes S77 only; T540 remains open.

## S79: Copied Attachment Binding Delivery

The entire nineteen-callback class now has one typed public binding, one
constructor publication and one Core lifetime owner. The old private slots
and owner field are deleted; all phases/firmware/shutdown receive explicit
context. Three fixtures preserve phase, failure and context coverage without
runtime production rebinding. [S79 evidence](../etc/evidence/t540-s79-copied-attachment-binding.md)
records fourteen source/test paths (+369/-218, net +151), dual-width 470/470
units, specialized/injected-negative gates, eight independent Core executions,
eight one-shot real-INI checkpoints and the rebuilt stripped 0540 pairs.
Shared, MyNES, INIs and external masters remain unchanged. Coordinator
acceptance follows the complete pushed implementation's actual diff review;
T540's opaque-board, test-classification and physical moves remain open.

Coordinator accepts immediately pushed P1 `8717d3af2` after the actual
37-path source/test/build/document/artifact review and complete packet-to-proof
mapping. Governance P2 closes S79 only, keeps the eight verified artifact
identities, removes its active packet and leaves T540 open.

## S80: Actual Board Callback Context

The complete nineteen-callback binding now receives the existing board
allocation instead of Core layout. Board borrows the same opaque Core execution
handle for bounded operations; Core retains the unique attachment finalizer.
All scheduler forwarding calls use their saved production context. No public
API, chip algorithm, timing grade or second state path is introduced.
[S80 evidence](../etc/evidence/t540-s80-board-callback-context.md) records
the complete receiver sweep, null-context preservation, verification and
artifact identities. Public board callers, direct-test classification and
physical neutral Core/IBM-PC relocation remain separate required receivers;
this implementation does not close T540.

Final units pass 470/470 per width, complete specialized and seven injected
negative gates pass, eight neutral Core executions and eight one-shot INI
boots pass, and all eight optimized stripped 0540 products are refreshed.
The six source/test paths add 197/remove 187 lines, net +10; Shared, MyNES
and owner INIs are unchanged. Coordinator acceptance follows the pushed
implementation's actual-change review.

Coordinator accepts immediately pushed P1 `24fb33b1e` after actual review of
all 25 NXVM-only paths and the eight committed artifact identities. Governance
P2 closes S80 only, removes its packet and leaves T540's public board API,
test classification and physical relocation open.

## S81: Frozen-Plan Board Handle Publication

The one frozen-plan constructor now publishes Core and its opaque board
allocation together after topology and timing application succeed. Failure
destroys the unpublished candidate through the existing Core owner. Every
plan caller receives the new output; the driver clears its borrowed board
handle when Core is destroyed. No getter, state mirror, second factory or
destruction path is added. [S81 evidence](../etc/evidence/t540-s81-plan-board-publication.md)
records publication/rollback checks and final verification. Public board
operations, configuration-only fixture callers, test classification and the
physical source move remain required; this delivery does not close T540.

Complete x64/x86 units pass 470/470 each, specialized and nine injected negative
gates pass, and all eight product builds, neutral executions and one-shot INI
boots pass. Eight optimized stripped 0540 products are refreshed. The initial
CPU negative timeout remains recorded; its complete rerun passes without
removing checks. Shared, MyNES, INIs and external masters are unchanged.

Coordinator accepts immediately pushed P1 `c01ee5ede` after actual review of
all 26 NXVM-only paths and the eight committed artifact identities. Governance
P2 closes S81 only, removes its active packet and leaves the public board
operation/caller cut, direct-test classification and physical relocation open.

## S83: Whole Board Input Receiver

The five keyboard/mouse/XT-fault operations consume the actual borrowed board
allocation. All 36 calls, their preparation helpers and associated XT direct
chip assertions use that handle. Core keeps the unique lifecycle observer and
mutation eligibility implementation; only the latter declaration becomes
public. No chip algorithm, timing grade, allocation or destructor changes.
[S83 evidence](../etc/evidence/t540-s83-board-input-handle.md) records the
complete receiver sweep and verification. Display/configuration, chip wiring,
fixture classification and physical relocation remain required before T540
can close. Shared, MyNES, owner INIs and external masters remain unchanged.

Complete x64/x86 units pass 470/470 each, specialized and nineteen injected
negative gates pass, and eight product builds, neutral executions and single
boots pass. The eight optimized stripped 0540 products are refreshed. Actual
pushed-diff review was required before S83 acceptance.

Coordinator accepts immediately pushed P1 `231d0ec95` after reviewing all
30 actual changed paths and committed artifact identities. Governance P2
accepts S83 only; T540's remaining board receivers and relocation stay open.

## S84: Whole Board Display Receiver

Configure, observe and capture consume the constructor's actual borrowed
board handle. Every production/unit/integration caller and the frozen-plan
topology operation uses that same allocation. VADP state/cache and Core's
configuration/lifecycle authority retain their unique owners; the existing
configuration-open declaration becomes public, without a new implementation.
[S84 evidence](../etc/evidence/t540-s84-board-display-handle.md) records the
caller sweep, status preservation and verification. Other board configuration,
chip wiring, fixture classification and physical relocation remain open.
Complete x64/x86 units pass 470/470 each, specialized and 21 injected negative
checks pass, and all eight product builds, neutral executions and one-shot
INI boots pass. Eight optimized stripped 0540 EXEs are refreshed. The
substring gate false positive and sandbox build retry are recorded without
miscounting old binaries. Shared, MyNES, INIs and external masters are unchanged.
Coordinator accepts immediately pushed P1 `d23281d1b` after actual review of
all 41 committed paths, retained algorithms/statuses, caller inventory and
eight artifact identities. Governance P2 accepts S84 only and removes its
active packet; T540's remaining receivers and physical movement stay open.

## S85: Whole Board Controller Receiver

The five DMA, FDC-binding, RTC/CMOS, FDC and HDC operations consume the
constructor's actual borrowed board. All sixty original calls and seven
controller callbacks use that allocation; PIT refresh registration agrees
between construction and cold reset. FDC keeps its separate opaque Core
connection for route publication. Core eligibility, rollback, reset order
and unique finalization remain unchanged. No getter, duplicate allocation,
state owner or forwarding path is introduced. The
[S85 evidence](../etc/evidence/t540-s85-board-controller-handles.md) records
the complete function/caller comparison and required verification. Complete
x64/x86 units pass 470/470 each, specialized gates and 22 injected negatives
pass, and all eight rebuilt products, neutral executions and single INI boots
pass. The initial x64 modal-test failure and successful full rerun are retained
without asserting an unverified cause. Coordinator accepts immediately pushed
P1 `8b67d2cdb` after reviewing all 38 committed paths and their worktree blob
identity, twelve complete definitions, seventeen unextended callers and both
extended fixtures. Governance P2 accepts S85 only and removes its active packet. Parity/D4,
RAM-resize veto, direct-fixture/constructor association and physical
relocation remain open; this controller boundary does not close T540.

## S86: Whole Electrical And RAM-Admission Receiver

All 28 remaining electrical operations/helpers, D4 memory callbacks and their
construction/cold-reset registrations use the actual borrowed board handle.
The neutral Core owns one RAM-resize operation; the existing copied attachment
delivers the exact optional board veto before allocation or mapping mutation.
No getter, duplicate state, waveform change or second resize implementation is
introduced. [S86 evidence](../etc/evidence/t540-s86-board-electrical-boundary.md)
records whole-definition/caller comparison, the initial fixture/gate failures
and their repairs, full 470/470 units per width, both specialized targets,
34 injected negatives, eight neutral executions/single INI boot checkpoints
and eight fresh stripped 0540 products. Source/test changes are +375/-353
across 26 paths; with four gates, +455/-361 across 30 paths. Shared, MyNES,
owner INIs and external masters remain unchanged. Constructor/private
association and actual Core/flat IBM-PC source movement remain required.
