# M5 T524: Shared-Library Multi-Consumer Service Completion

T524 turns the existing `src/lib` roots into a complete, independently verified
source-identical corpus for NXVM and a later SoftPC adoption. It does not make
the library an emulator or a product-binding owner.

## S1 Admission

S1 is a read-only completeness audit. It freezes the entire tracked library
corpus, manifest and CMake source-list dispositions, public-ABI vocabulary and
existing neutral-conformance coverage. It records the known missing clock
entries as a P0 finding and defines bounded later batches; it changes no
library contract or implementation.

The active packet in [Current](../states/CURRENT.md) is the authoritative S1
contract. The owner-revised candidate is
[the multi-consumer completion proposal](../proposals/m5-shared-library-multi-consumer-service-completion.md).

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
