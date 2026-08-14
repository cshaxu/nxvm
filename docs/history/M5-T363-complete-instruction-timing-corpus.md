# M5 T363: Complete Instruction-Timing Corpus

## Task Record

T363 consumes the closed T357--T362 timing ledgers and establishes the complete
profile-aware instruction-timing corpus for 8086, 80186, 80286, and 80386.
It allocates work only after every implemented form and relevant
mode/prefix/addressing variant has a primary-source, same-profile-model, or
explicit receiver disposition.  It is a model-L3 CPU timing task and does not
claim physical or cycle-exact timing.

## Active Progress

### S1: Full form inventory, ownership, and source classification

S1 reconciled the 256 primary and 256 secondary dispatch slots,
metadata/profile rejection, current source-cost owners, existing T357--T362
evidence, and every remaining fallback into a complete source and mechanism
ledger before any runtime allocation.

S1 is accepted at `5e44789a`. Its
[inventory](../etc/evidence/t363-s1-complete-instruction-timing-inventory.md)
retains the complete dispatch partition, source ladder, explicit descriptor
transfers, and the bounded S2--S7 mechanism order. S2 may now reconcile the
primary arithmetic/FLAGS/data/ModR-M/EA matrix only.

### S2: Arithmetic, FLAGS, data and ModR/M/EA source matrix

S2 is accepted at `50861f53`. Its
[reconciliation](../etc/evidence/t363-s2-arithmetic-data-reconciliation.md)
confirms complete shared source ownership without a runtime change. S3 may
reconcile only control, stack and real-control timing.

### S3: Control, stack, and real-control source matrix

S3 is accepted at `6523d514`. Its
[reconciliation](../etc/evidence/t363-s3-control-stack-reconciliation.md)
confirms complete successful CALL/JMP/RET, stack, LOOP/Jcc, HLT,
INT/INTO/IRET and real-control ownership without a runtime change. Protected,
VM86, task, cross-privilege, generic delivery, and physical forms remain
explicit receivers. S4 may reconcile only string/repeat and ordinary I/O
timing.
