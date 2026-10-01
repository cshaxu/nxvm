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
`x86/ibmpc/{common,xt,at}`.  It introduces no source directory, ABI or runtime
change.  Its completion record identifies the first safe source batches and
the profile regressions that remain product-owned.
