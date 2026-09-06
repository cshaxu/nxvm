# M5 T524: Shared-Library Multi-Consumer Service Completion

T524 turns the existing `src/lib` roots into a complete, independently verified
source-identical corpus for NXVM and a later SoftPC adoption. It does not make
the library an emulator or a product-binding owner.

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

## S6 Result

S6 gives `lib/storage` one explicit exclusive byte-image lease and replacement
transaction.  Direct-readonly, direct-writable and overlay byte modes remain
the only retained persistence modes.  The generic transfer accepts an empty
lease, returns the retired lease for caller-controlled destruction, and leaves
the slot unchanged on an invalid alias.  NXVM FDD and HDD candidate swaps now
both use that one operation; topology and derived-byte-view policy remain
product local.  The complete contract and evidence are in [T524 S6 storage
lease contract](../etc/evidence/t524-s6-storage-lease-contract.md).

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
