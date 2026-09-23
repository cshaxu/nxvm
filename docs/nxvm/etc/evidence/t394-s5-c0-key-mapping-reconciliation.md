# T394 S5 P2 C0 Eligibility-Key Mapping Reconciliation

## Scope

This record completes the finite mapping of T390 S27's accepted 82-child C0
universe to the S4 Core eligibility key. It selects no descriptor entries and
makes no Model-40 physical-retirement, board-clock, firmware, device-timing,
cycle-exact, or L3 claim.

## Replay Basis

A disposable historical S26 capture build, replayed only against already
owner-managed inputs, reproduces the accepted C0 checkpoint: 18,255 classified
successful retirements, zero source-unallocated retirements, protected-return
terminal, and 82 aggregates. The current S5 replay against the same inputs
preserves the 18,255/zero/terminal result and emits 81 bounded normalized Core
keys without reaching the capture limit.

Thus the mapping is finite and total: 80 historical children have one distinct
observed key, and the remaining two children share one observed key. There are
no absent children, unallocated successes, unexpected key collisions, or
capacity losses.

## Recorded Same-Key Equivalence

The sole collision is:

| Historical children | Historical aggregate counts | Common normalized key dimensions | Equivalence basis |
| --- | ---: | --- | --- |
| C0-027 and the C0-031 memory/ext-6 child | 1 + 2 = 3 | 80386; primary timing family; no opaque source-form; prefix-normalized `MOV` primary opcode; no escape; memory ModR/M shape with extension 6; no control/repeat distinction; real mode/CPL 0/16-bit operand and address/no LOCK or REP | C0-027 is the CS segment-override instance of the same `MOV r16,r/m16` semantic form. Its only extra distinction is a literal segment prefix. T394 S3 deliberately defines instruction identity as prefix-normalized and excludes literal prefixes from selection; all retained architectural and timing-family dimensions are equal. |

The current aggregate has count 3, exactly preserving the two historical
children's combined successful-retirement count. The former `other` name was a
historical capture-label artifact: that harness named from the first raw byte,
while the current capture and the S4 key identify the prefix-normalized opcode.
The label is not a S3 eligibility-key dimension.

## Verification

- The S5 P1 pre-retirement snapshot regression remains covered by the `LMSW`
  Core observation smoke.
- The current capture's synthetic aggregation smoke emits
  `M5:T394:S5:C0-KEY-MAPPING:OK` and confirms fieldwise equality aggregation.
- Read-only historical/current replay comparison proves the 82-child/81-key
  cardinality and identifies the only count-preserving collision above.
- Focused Core timing, retirement-observation and deterministic Model-40
  composition tests pass; the complete current-gates build and documentation
  governance pass.

## Result And Transfer

S5's C0-to-key mapping exit criterion is met: every accepted child is
represented by an observed key or the recorded same-key semantic equivalent.
This does not itself authorize a production qualification descriptor or
physical retirement. A later separately admitted selection task must decide
whether the complete 81-key C0 set is an eligible physical descriptor and must
continue to preserve unallocated/absent-key prepublication rejection.