# M5 T362: Intel-Constrained Legacy Timing-Model Normalization

## Task Record

T362 receives the remaining T361 dynamic-arithmetic routes whose primary Intel
tables provide a bounded timing domain but no operand-to-clock function. It
uses a source-labelled same-profile model only when the exact form and
accounting basis are demonstrated, and records direct, Intel-constrained, or
reference-exhausted results without claiming physical or cycle-exact timing.

## Active Progress

### S1: Source-domain and normalization ledger

S1 is active. It inventories Intel domains, candidate model scalars, accounting
boundaries, and every transferred fallback before any runtime allocation.

S1 is accepted at `ba5a1221`. Its
[ledger](../etc/evidence/t362-s1-legacy-timing-normalization-ledger.md)
establishes the exact Intel domains, MAME i186 register/memory scalar boundary,
and the two direct plus two constrained immediate-IMUL dispositions. S2 owns
the centralized allocation and regression corpus.
