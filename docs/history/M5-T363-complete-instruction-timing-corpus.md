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

### S4: String, repeat, and ordinary-I/O source matrix

S4 is accepted at `76222b51`. Its
[reconciliation](../etc/evidence/t363-s4-string-io-reconciliation.md)
confirms complete selected primitive, REP/restart and ordinary-I/O ownership,
with one restart-safe publisher and explicit 80286 `REP LODS` and physical/
provider receivers. The focused smoke and 246/246 current-gate suite passed.
S5 may reconcile only 80386 secondary integer and prefix/width timing.

### S5: 80386 secondary integer and prefix/width source matrix

S5 is accepted at `0e521db7`. Its
[reconciliation](../etc/evidence/t363-s5-secondary-prefix-reconciliation.md)
confirms the selected non-privileged `0F` integer forms, their legal
prefix/width variants, and their 106-clock preflight boundary retain one
source-backed publisher. S6 may reconcile only privileged/table/control/debug
and task timing forms.

### S6: Privileged, table, selector, control, debug, and task-adjacent matrix

S6 is accepted at `0721ba8f`. Its
[reconciliation](../etc/evidence/t363-s6-privileged-reconciliation.md)
confirms all selected successful 80386 system forms retain one source-backed
publisher, while context-incomplete 80286, transition/delivery and physical
forms remain explicit receivers. S7 may perform only the task-level corpus
closure audit.
