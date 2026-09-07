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
no hits. SoftPC consumer adoption transfers outside the NXVM queue and retains
the same byte-identical-copy and product-local-binding requirement; it never
permits a source edit inside `lib`.

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

## S6 Result And Acceptance

S6 deletes the duplicate `core/platform/file.*` implementation and makes
`lib/storage/file.*` the one generic byte, exclusive-create, replace, remove
and opaque text-stream owner. VM asset validation, media semantics and debug
record formatting remain at their existing product owners; only their generic
file mechanics moved. Two obsolete boundary checks now prove the real
`lib/storage` atomic-media and caller routes rather than referring to deleted
paths.

Coordinator actual-diff review accepts `bd6141b1`: the code and test delta is
`+190/-230` outside documentation, including the focused opaque stream proof;
the historical `type` standard-C facade remains only for standard streams and
product debugger commands, not as a Core file owner. Full repository-only
unit passes `311/311` in 18.46 seconds, both refreshed CMake boundary checks,
documentation governance, manifest and obsolete-symbol sweeps pass. S7 is
admitted for the single Win32 keyboard binding move.

## S7 Result And Acceptance

S7 deletes Core's Win32 keyboard implementation. Review rejected the initial
VM relocation because it duplicated `lib/ux/win32/input.c`; corrective P2
removes that duplicate. `lib/ux` is now the sole host-layout normalizer and VM
has one value-only `ux_event` to Core-input binding used by both native input
and test injection.

Coordinator actual-diff review accepts `ce9ba52f` plus `64a30bca`: only
`lib/ux/win32/input.c` reads Win32 keyboard-layout APIs; the obsolete Core and
VM normalizer symbols have no source hits. Focused shared-UX and VM ingress
proofs pass, as does repository-only unit `311/311` in 15.88 seconds.
Documentation governance and diff checks pass. S8 is admitted for semantic
Core guest-boundary names only.

## S8 Result

S8 moves the Core guest-frame value, guest-input source and copied-frame
publication mailbox out of the misleading `core/platform` namespace into
`core/machine`. This is a semantic source and test target consolidation only:
Core remains the sole guest-state owner, while `lib/ux` remains the sole native
presentation owner. The old paths and names are deleted rather than wrapped.
The retained ledger is [T523 S8 evidence](../etc/evidence/t523-s8-core-guest-boundary-owner-ledger.md).

## S8 Acceptance

Coordinator actual-diff review accepts `7963034f`: all guest-boundary source,
test and CMake names now resolve under `core/machine`; the authority source map
no longer describes a Core platform module. The guest mailbox/input source
remain sole Core state owners and `lib/ux` retains native presentation. Focused
boundary proofs, 311/311 repository-only unit in 17.11 seconds, refreshed
boundary verifiers, documentation governance and diff checks pass. S9 is
admitted for the generic monotonic-clock replacement.

## S9 Result And Acceptance

S9 deletes VM-owned virtual-time mechanics. `lib/host/clock.c` owns the one
generic validation/conversion route; its Win32 and Linux private readers own
only their respective counter API. VM retains only display cadence and
completed-Core-progress pacing comparison, and Core has no host-clock input.
Coordinator review accepts `743ec108`: focused proof and repository-only unit
311/311 pass in 18.87 seconds; retired VM clock names have no consumers and
the generic wall-clock sweep has only `lib/host` readers. S10 is admitted for
task-level integration and final owner audit.

## S10 And T523 Closure

The final audit finds no duplicate generic file, keyboard, guest-boundary or
host-clock route. Repository-only unit passes 311/311. All 44 external-asset
integration rows pass through a non-profile 24/24 replay plus isolated serial
profile rows, including the Model 40 and IBM boot terminals. Documentation
governance and diff checks pass. The retained [closure audit](../etc/evidence/t523-s10-closure-audit.md)
records the complete owner disposition; T523 is closed.
