# M5 T523: Cross-Product Library Contract Completion

T523 completes the reusable host-side library contract established by T522.
`base`, `ux`, `host`, `session`, `storage`, and `observability` remain peer
capabilities: no library root owns another, a machine, a profile, firmware,
guest state, or product policy. NXVM and a future SoftPC adoption each bind the
same source corpus from their own product boundary.

## S1 Admission

S1 freezes the complete portability and ownership ledger before source work.
It inventories every public value, callback, mutable object, native wait,
timer, action, frame field and storage mode; defines the zero-machine-pointer
ABI; and names the two product binding boundaries. It changes no runtime
source, build route, asset, or product behavior.

The active packet in [Current](../states/CURRENT.md) is the authoritative S1
contract. The retained owner-approved proposal is
[here](M5-T523-cross-product-library-contract-completion-proposal.md).

## S1 Result

The complete ledger is retained in
[T523 S1 evidence](../etc/evidence/t523-s1-cross-product-compatibility-ledger.md).
It finds no required public operation that needs guest or product ownership:
the shared ABI can be reduced to copied values, opaque library handles and
product callbacks. It also records the finite S2--S5 replacement batches.

The first unit replay exposed two missing executables in a stale CMake test
tree, not an UX behavior failure. After an incremental rebuild, both targeted
tests pass and the complete rebuilt repository-only suite passes (`308/308`);
the final log contains no failed or missing executable record. Documentation
governance and `git diff --check` pass. S2 is the next admitted batch.

## S2 Result

S2 establishes `lib/base` as the only public library value/status/atomic
vocabulary and eliminates the direct `type.h`/`type-facade` dependency from
every library target.  Implementations use direct C and neutral `lib_*`
spellings; no NXVM compatibility layer remains in the reusable corpus.

The copied-frame mailbox now owns one platform wake object.  Publish signals
that object; Console and Window wait on it indefinitely with their existing
input/message sources, replacing the 250 ms Console and 16 ms Window polling
routes.  Linux receives the same pipe-backed wake contract.  The detailed
owner and verification record is retained in
[T523 S2 evidence](../etc/evidence/t523-s2-portable-base-event-mailbox.md).

A full rebuilt x64 repository-only replay passes `309/309`; focused mailbox
and manifest checks, documentation governance and `git diff --check` pass.

## S2 Acceptance

Coordinator actual-diff review accepts `22ff221f`.  It confirms that the sole
publication path is `ux_mailbox` and its native-private event/pipe, that the
two former fixed presentation waits are gone, and that the public mailbox
contract has no native handle.  The complete unit replay passes `309/309` in
17.20 seconds; documentation governance and the full library dependency and
legacy-vocabulary sweeps pass.  S3 is the next bounded UX-value and explicit
presenter batch.
