# T420 S2 Duplicate Audit Withdrawal

`M5:T420:S2:DUPLICATE-AUDIT-WITHDRAWAL:OK`

## Evidence

T401 S1-S78 already audits the four finite decoder spaces, metadata/profile
rules, lexical coherence, FPU/WAIT interface, form semantics, exception/state
owners, source-timing dispositions and a concrete form requirement matrix. Its
independent S78 closure reconciles the matrix with current dispatch and the
full gate. T360 S1-S5 separately reconciles the selected 8086, 80186, 80286
and 80386 timing sources and every current timing consumer.

## Decision

T420 S1 duplicated those completed owners and would have produced a second
matrix rather than a new defect report. It is withdrawn before implementation.
No Core, VM, test, timing or artifact path changes. The retained TODO entries
are the only valid future CPU admissions, each with its own explicit condition.

Documentation governance passes. This withdrawal preserves one authoritative
CPU audit path and does not revise the T401/T360 transfers.