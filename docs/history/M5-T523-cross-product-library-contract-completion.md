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

## S3 Result

S3 completes the copied UX values and removes the remaining automatic routing
and product-action meanings from `lib/ux`. Product code now selects targets
explicitly and maps its own action identifiers; the shared library only owns
the native presentation loop, copied frame/input values, registered action
callback ordering and capture state. The detailed field/owner and verification
record is retained in [T523 S3 evidence](../etc/evidence/t523-s3-ux-value-contract.md).

## S3 Acceptance

Coordinator review accepts `73b2c5ef`, `def32c37`, and `d90fd430`. The final
batch has no public product or guest ownership, no automatic library routing,
and one release-all callback before every registered action callback. The full
repository-only unit suite passes `309/309` in 14.74 seconds; documentation
governance, manifest, legacy-vocabulary and routing sweeps pass. S4 is admitted
for the independent host/session/storage completion batch.

## S4 Result And Acceptance

S4 replaces the former polling-only host route with opaque native event/task
create, cancellation, wait and join mechanics; it makes generic lifecycle
state opaque, provides direct-readonly/direct-writable/overlay byte-image
modes, and leaves Core safe-point and controller ownership in NXVM. The old
execution-flip polling helper is deleted. A final review also moves the native
mailbox wait bridge into `ux/internal`, so it cannot be consumed as public UX
ABI.

Coordinator review accepts `9ecb3810` and `b4ba3957`: 56 manifest-selected
library files hash exactly, public headers carry neither native nor product
types, focused native lifecycle proof passes, and the full repository-only unit
suite passes `309/309`. S5 is admitted for neutral conformance, one-path NXVM
audit and the bounded SoftPC adoption handoff.

## S5 Result And T523 Closure

S5 recompiles the entire platform-selected 56-file manifest corpus as a
test-only neutral static target. Its consumer includes only public `lib`
headers and proves copied text/graphics frames, palette, dirty rectangle,
mailbox wake, Unicode text input, action/capture, opaque host/lifecycle state,
all image modes and copied outcomes. It neither links NXVM nor includes Core,
VM, profile, firmware or asset headers.

The retained [S5 ledger](../etc/evidence/t523-s5-neutral-consumer-and-route-ledger.md)
also records the one NXVM generic route per capability and the different
machine-owned reason for each retained Core/VM boundary. The manifest remains
source-identical; public-header, product-vocabulary and peer-root sweeps have
no hits. The Queue-only [SoftPC adoption proposal](../proposals/m5-softpc-shared-library-binding-adoption.md)
requires a byte-identical copy plus a product-local binding, never a source
edit inside `lib`.

Coordinator actual-diff review accepts `fcdc7e99`. The neutral marker and the
native run-handle marker pass; repository-only unit passes `310/310`, and
documentation governance plus `git diff --check` pass. T523 is closed. No
machine/controller/profile behavior, asset or runtime dependency was moved
into the library.

## Corrective Reopening

The owner reopened T523 after the closure review identified four finite
remaining owner cleanups that the original S5 sweep did not cover: duplicate
generic file mechanics in `core/platform/file.*`; a Win32 keyboard-to-PC-input
binding incorrectly located under Core; misleading Core guest-boundary names
under `core/platform`; and generic monotonic-clock mechanics in
`vm/platform/virtual_time.*`. Corrective S6--S9 respectively own those four
surfaces. The former closure remains historical evidence, not a claim that
these newly admitted paths were already clean.
