# M5 T338: 8086 And 80186 Profile Closure

## Task Record

T338 implements the second package in the M5 four-profile CPU-completeness
program. It owns Intel 8086 baseline and 80186 primary extension reconciliation
after T337 established the shared delivery contracts. It does not own 80286
protected semantics, 80386DX width/state semantics, or external coprocessor
execution.

The active packet in [CURRENT](../states/CURRENT.md) defines each admitted S.
The original proposal is retained as
[historical background](M5-T338-8086-80186-profile-closure-proposal.md).

## S1 Result

S1 establishes the complete primary-form allocation and legacy LOCK linkage
before changing a handler. It retains every 8086/80186 form, grouped ModRM
extension, defined-state boundary, external coprocessor exclusion, and
80286/80386 transfer in [the S1 ledger](../etc/evidence/t338-s1-8086-80186-form-allocation.md).
The ledger assigns four continuation boundaries: legacy ALU/FLAGS/condition,
legacy data/stack/control/I/O, 80186-only primary extensions, then a task-level
closure audit. T328 remains the single shared legacy-LOCK owner.

## S2 Result

S2 closes the allocated legacy ALU, FLAGS, condition, adjustment, shift, and
grouped-form boundary. Its owner smoke supplies 8086 and 80186 proof for the
binary, accumulator-immediate, Group 1--3, TEST, INC/DEC, branch/loop,
adjustment, sign-extension, and direct-FLAGS forms; it asserts only
architecturally defined FLAGS. It also proves real-mode divide-error delivery
through IVT vector 0, including the restart frame and post-gate state.

The mechanism sweep found no production defect. Development corrections were
limited to smoke oracles for ADC/SBB inputs and Group 3 result locations. The
S2 evidence retains the exact form/profile matrix and the link to T328 as the
sole legacy-LOCK policy owner. Commit `9eb3729f` was reviewed; the T338
developer artifact SHA-256 is
`E9626E829FE8F9A1BE7A25219D48295D704C5831F64C4D2D50709671CB144F13`, and
the complete current-gate passed 218/218.

## S3 Result

S3 closes the inherited 8086/80186 data, string, stack, real-control/interrupt,
and ordinary port-I/O boundary. Its retained matrix assigns each allocated form
family to an existing current-gate owner and records the defined publication,
stack, segment, REP/DF, IVT, callback, rejection, and T328 `LOCK` boundary.

The only implementation adjustment was within the existing control-transfer
owner: its real-mode fixture is now profile-parameterized and proves both 8086
and 80186 direct/indirect far transfers, `RETF iw`, near call/return/jump
forms, and the reserved `FF /7` nonpublication path. The mechanism sweep found
no production defect. Commit `317d9df5` was reviewed; the developer artifact
remains SHA-256
`E9626E829FE8F9A1BE7A25219D48295D704C5831F64C4D2D50709671CB144F13`, and the
complete current-gate passed 218/218.

## S4 Result

S4 closes all 80186-only primary extensions and the corresponding 8086
rejection boundary: PUSHA/POPA, BOUND, immediate PUSH/IMUL, INS/OUTS,
immediate-count Group 2, and ENTER/LEAVE. The matrix keeps Intel-defined stack,
signed-bound, port, REP, and undefined-extension behavior explicit.

The profile sweep reproduced a shared local defect: all four INS/OUTS handlers
incorrectly required 80386 even though the opcode metadata and architecture
define them from 80186. The guards now converge on 80186; no port, prefix, or
delivery helper changed. The owner tests now directly prove 80186 `BOUND`
`#BR`, immediate-IMUL r/m16, and all byte/word `C0/C1` extensions. Commit
`1d8fe07a` was reviewed; the rebuilt developer artifact SHA-256 is
`7D79417889821695DB4993DFEA5134B01E5B16D69007C20A6F1E3CBB8C75C05F`, and the
complete current-gate passed 218/218.

## Closure

T338 is closed. S5 reconciled every S1 allocation to accepted S2--S4 evidence
or an explicit 80286, 80386DX, or external-coprocessor transfer. T328 remains
the sole legacy `LOCK` policy owner. The Queue now advances to the 80286
descriptor-table and protected-transfer closure candidate.
