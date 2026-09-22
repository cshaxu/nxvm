# M5 T535 Canonical SoftPC Six-Component Refresh

## Admission

The owner admitted T535 on 2026-09-22 to import the latest committed SoftPC
shared source/test corpus, preserving it exactly and adapting NXVM only at its
product boundary.  The active task proposal is
[Canonical SoftPC six-component refresh](M5-T535-canonical-softpc-six-component-refresh-proposal.md).

## S1: Frozen Upstream Audit

P1 `6eb052df` records the committed upstream identity, six-tree delta,
provenance and adaptation boundary before any shared source is copied. Its
[S1 audit](../etc/evidence/t535-s1-softpc-six-component-audit.md) admits an
exact corpus copy as S2 and records no source change. Coordinator review
accepted S1: the frozen source excludes dirty media, classifies every delta,
and passes documentation governance and whitespace review.

## S2: Exact Six-Tree Corpus Replacement

P1 imports the six named roots byte-for-byte from the frozen SoftPC revision,
including tests, manifests and independent build descriptions. The complete
unit gate adds three canonical audio smokes and passes 336/336. The fresh build
also exposed old test-only opaque-Profile/mirror assumptions; S2 removes those
assumptions without altering production. [Evidence](../etc/evidence/t535-s2-canonical-six-component-import.md)
proves exact equality and verifier results. No NXVM consumer adaptation exists,
so S3 is skipped; S4 receives final integration/artifact convergence.
