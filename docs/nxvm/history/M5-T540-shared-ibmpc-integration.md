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
