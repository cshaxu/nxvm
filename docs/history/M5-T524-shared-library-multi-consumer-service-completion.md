# M5 T524: Shared-Library Multi-Consumer Service Completion

T524 turns the existing `src/lib` roots into a complete, independently verified
source-identical corpus for NXVM and a later SoftPC adoption. It does not make
the library an emulator or a product-binding owner.

## S10 Owner-Reopen: File-Backed Storage Repair

The owner rejected the prior S6 conclusion: an owned RAM byte image is not a
direct disk-file medium.  SoftPC's existing floppy and fixed-disk paths retain
an open file and perform offset I/O for direct and readonly media, whereas
NXVM had first read the complete file into RAM.  S10 corrects this shared
library gap with an opaque neutral byte-medium contract.  It must leave CHS,
media topology, controller timing and product policy outside `src/lib`; the
task remains open after the implementation P for owner confirmation.

The owner further specifies the overlay contract: it is a readonly physical
base plus sparse 4-KiB dirty pages.  A read observes a dirty page when present
and otherwise reads the base at that byte offset.  It must not materialize the
whole source merely to mount it.  It is discard-only: direct is the sole
write-through mode and overlay never commits a replacement.

## S8 Local P1

Owner-approved P1 publishes the completed library-local portability batch:
the standalone CMake entry runs a neutral public-header consumer, Windows CI
runs its CTest gate, and the host-clock contract no longer carries product time
terminology. This is not S8 or T524 acceptance: the current Model-40 FDC
integration row remains red. See
[`t524-s8-portable-closure-audit.md`](../etc/evidence/t524-s8-portable-closure-audit.md).

The first upstream P1 matrix exposed two publication defects: MSVC cannot
compile the retained C11 atomic contract, and the previous manifest verifier
depended on CMake text hashing. The active S8 correction selects UCRT64 GCC on
Windows and verifies LF-only raw source bytes. The owner does not require a
remote Linux runtime gate; strict Linux source syntax remains the Linux proof.

## S8 Owner-Directed Closure

On 2026-09-06 the owner explicitly directed S8 to close and admitted S9 for
the Model-40 repair. The S8 corpus result is accepted with the known red
Model-40 FDC integration row recorded as a transfer, not passing evidence.
T524 remains open; S9 may repair only the Model-40 owner and must not add a
`lib` workaround.

## S9 Clean Model-40 Disposition

The transferred Model-40 row is green without a source change. A concurrent
instruction-level diagnostic had consumed host capacity and polluted a
wall-clock integration result. With that probe absent, the unchanged external
YAML reaches `installer-running` in 156.37 seconds. Complete integration is
44/44 and complete unit is 311/311. The record is retained in
[`t524-s9-model40-clean-integration-replay.md`](../etc/evidence/t524-s9-model40-clean-integration-replay.md).
No FDC, CPU, memory, firmware, profile or library workaround was added.

## S1 Admission

S1 is a read-only completeness audit. It freezes the entire tracked library
corpus, manifest and CMake source-list dispositions, public-ABI vocabulary and
existing neutral-conformance coverage. It records the known missing clock
entries as a P0 finding and defines bounded later batches; it changes no
library contract or implementation.

The active packet in [Current](../states/CURRENT.md) is the authoritative S1
contract. The owner-revised candidate is
[the retained multi-consumer completion proposal](M5-T524-shared-library-multi-consumer-service-completion-proposal.md).

## S1 Result

The complete audit is retained in [T524 S1 evidence](../etc/evidence/t524-s1-shared-library-readiness-audit.md).
It confirms the five-file clock omission in a 61-file corpus, no independent
library build or CI drift gate, and material neutral-boundary defects in Linux
key translation, mailbox/lifetime semantics, host wait semantics and storage
lease semantics. It assigns one bounded receiver per library root; no API was
added during the audit.

## S1 Acceptance

Coordinator actual-diff review accepts `69a8e875`: the evidence reconciles the
whole manifest rather than only the reported clocks, verifies the source/CMake
and public-vocabulary findings against current code, and does not add a
speculative library interface. Repository-only unit is 311/311 in 23.23
seconds; documentation governance and diff checks pass. T524 remains open for
the owner to admit S2's single manifest/build replacement boundary.

## S2 Result

S2 establishes one complete 63-file publish corpus and makes `src/lib` its
only source-list owner. The standalone CMake entry, full verifier, disposable
failure fixtures and Windows/Linux CI gate replace the partial UX-only checker
and root-owned duplicate lists. The [S2 evidence](../etc/evidence/t524-s2-library-publish-boundary.md)
records exact coverage and verification. No public library or product contract
changes; T524 remains open for the per-root contract batches.

## S2 Acceptance

Coordinator actual-diff review accepts `8e926c03`: the root has one source
inventory, `ux` is one root rather than a cross-root target edge, and the
manifest covers all 63 non-self files. Standalone build/CTest, failure fixtures,
root unit 312/312, documentation governance and diff checks pass. T524 remains
open for S3--S8.

## S3 Result

S3 makes the entire UX root neutral: Linux terminal adapters publish shared key
identities rather than an x86 transition, and NXVM alone performs its protocol
conversion at its product binding.  The root now rejects invalid copied frames,
documents mailbox lifetime/generation rules, rejects duplicate or over-capacity
action chords, removes product/machine vocabulary, and supplies two independent
public-header consumer proofs.  The complete contract and verification record
is retained in [S3 evidence](../etc/evidence/t524-s3-ux-neutral-contract.md).

## S3 Acceptance

Coordinator actual-diff review accepts `94e81d1b`: the terminal no longer
emits an x86 protocol from `lib`; the sole conversion stays in NXVM's binding.
Frame, action and mailbox contracts are enforced rather than merely described,
and the two consumer tests use only public library headers. Focused native and
binding tests, complete unit 312/312, manifest verification, standalone CMake
build/CTest, vocabulary and peer-root sweeps, documentation governance and
diff checks pass. T524 remains open for S4--S8.

## S4 Result

S4 gives the shared host root one explicit manual-reset, cancellation and
wait-any contract. The Linux backend uses monotonic generation-based waiting;
the Windows backend has the same cancellation-first result order. Task
destruction has one safe cancellation/join path. The retained evidence is
[S4 host contract](../etc/evidence/t524-s4-host-contract.md).

## S4 Acceptance

Coordinator actual-diff review accepts `ab0c0adc`: all new synchronization
state remains inside the host root, no product callback or cross-root include
was introduced, and both native implementations implement the same visible
priority/lifecycle contract. Windows smoke, strict Linux syntax, complete unit
312/312, manifest, vocabulary/dependency sweeps, documentation governance and
diff checks pass. T524 remains open for S5--S8.

## S5 Result

S5 retains `lib/session/state` as the sole neutral lifecycle capability and
deletes the unproven `lib/session/executor` facade. NXVM owns its one local
execution callback boundary, so library state has no product-run policy or
thread/safe-point behavior. The complete disposition and verification are in
[S5 evidence](../etc/evidence/t524-s5-session-disposition.md).

## S5 Acceptance

Coordinator actual-diff review accepts `9ecb6713`: the library loses the
one-product executor rather than gaining a speculative callback framework, and
NXVM has exactly one local execution boundary around the retained neutral
lifecycle state. The review also confirms that the repaired UX wake fixture
now obeys its existing validity contract. Focused checks, standalone lib build,
manifest, complete unit 311/311, session-root vocabulary/dependency sweeps,
documentation governance and diff checks pass. T524 remains open for S6--S8.

## S6 Result (superseded by S10 file-backed medium)

S6 introduced an explicit exclusive byte-image lease and replacement
transaction. S10 supersedes its RAM-backed direct/overlay interpretation with
one file-backed byte-medium contract; S6's replacement ownership rule remains
the retained transfer mechanism. The complete former contract is retained in
[T524 S6 storage lease contract](../etc/evidence/t524-s6-storage-lease-contract.md).

## S6 Acceptance

Coordinator actual-diff review accepts `ef604b3d`: the replacement contract
rejects nonempty self/output aliasing without dropping an owned lease, while
empty-to-empty stays an explicit no-op.  The two live NXVM candidate routes
share that one operation; creation and destruction remain distinct local
transitions.  Focused storage/FDD/HDD checks, complete unit 311/311 in 26.07
seconds, standalone library build/CTest, manifest, strict Linux syntax,
storage-root vocabulary/dependency sweeps, documentation governance and diff
checks pass. T524 remains open for S7--S8.

## S7 Result

S7 retains copied outcome as the complete observability root.  It documents
exclusive caller serialization and copied snapshot behavior, confirms the
whole consumer set requires only sequence/status/validity, and rejects an
unproven diagnostic-event framework.  The supporting result is [T524 S7
observability disposition](../etc/evidence/t524-s7-observability-disposition.md).

## S7 Acceptance

Coordinator actual-diff review accepts `91fb0218`: the result is an explicit
single-owner copied status object, not an untested synchronization or logging
framework.  Both public neutral consumers and the sole NXVM start-result
binding are accounted for; invalid API entry is covered.  Focused outcome and
run-handle integration proof, complete unit 311/311 in 21.45 seconds,
standalone library build/CTest, manifest, strict Linux syntax,
observability-root vocabulary/dependency sweeps, documentation governance and
diff checks pass. T524 remains open only for S8.

## S10 Implementation P1

S10 replaces the former RAM-only `storage/image` and persistence-sidecar path
with `storage/medium`: Direct and Readonly retain a file handle; Overlay keeps
the readonly base plus sparse 4-KiB dirty pages and always discards them.
FDD/HDD retain geometry and media policy locally while all file-media byte
access uses the one opaque lease. The obsolete commit APIs, session toggle,
sidecar and padding-persistence state are deleted.

Focused proof passes for the neutral medium, FDD/HDD Direct/Readonly and
discard-only Overlay routes. The complete unit replay passes 311/311; the
fresh Release Model-40 1.2MB external-YAML row reaches `installer-running` in
53.75 seconds. T524 remains open for owner confirmation and task-level
integration closure.

## S11 Implementation P1

S11 removes NXVM debugger policy from the shared lifecycle state: step,
numeric pause reason, unused flip, and the NXVM-only fault alias are gone from
`lib/session`. NXVM control alone owns the step/reason atomics, and the runner
uses that one control route to execute one instruction before a generic pause.
The complete owner sweep, current-source focused proof, full unit replay,
manifest and x64/x86 optimized artifact evidence are in [S11 lifecycle
neutrality](../etc/evidence/t524-s11-lifecycle-neutrality.md). T524 remains
open for owner manual artifact testing and subsequent acceptance.

## S11 Acceptance

The owner closed S11 on 2026-09-06 after the reported P1 and its manual-test
handoff. `879e4838` is accepted: `lib/session` retains only neutral lifecycle
state, while NXVM control alone owns step and pause reason. T524 remains open.

## S12 Admission

The owner admitted a narrow storage dead-surface cleanup. It removes APIs with
no production consumer, keeps Direct write-through immediate, and retains the
FDD/HDD eject discard route. It does not reopen the discard-only overlay
contract or add commit semantics.

## S12 Implementation P1

S12 deletes the dead exclusive/exists/replace/remove file helpers and their
native implementations, plus unused medium mode-query and flush operations.
Direct `write_at` remains the sole immediate write-through boundary; FDD/HDD
eject retain `medium_discard` as their destruction-and-null lease path. The
complete evidence is [S12 storage API surface cleanup](../etc/evidence/t524-s12-storage-api-surface.md).
T524 remains open for coordinator review and owner audit.

## S12 Implementation P2

The stale Linux platform-contract gate now verifies the current `linux.c`
host-sync/UX lifecycle and library CMake platform dependencies, rather than a
removed pre-library source file. No runtime code changes. The S12 evidence
records the corrected contract and gate result.
