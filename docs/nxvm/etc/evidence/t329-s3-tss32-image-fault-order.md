# T329 S3: 32-bit TSS Direct-JMP Image and Fault Order

## Scope

S3 strengthens only the accepted 80386 protected CPL0 direct task-JMP state
machine: busy 32-bit source TSS, available 32-bit target TSS, paging disabled,
and null incoming LDTR. It does not add task gates, task CALL, nested IRET/NT,
LDT loading, task paging, or debug state.

## Mechanical Image Contract

The private task-switch image uses named constants for every Intel 80386 TSS
field from CR3 at `1Ch` through LDTR at `60h`. C11 `offsetof` assertions bind
the private representation to those offsets, and its exact `48h` byte span is
asserted. The direct path uses those names both for its complete preflight
range and for each outgoing write; it does not rely on an unlabelled struct
copy or a test-only image interface.

## Requirement-to-Proof Matrix

| Requirement | Owner evidence |
| --- | --- |
| Complete valid state image | Direct and indirect 16/32-bit operand and 16/32-bit address forms capture all outgoing CR3, EIP, EFLAGS, GPR, and selector fields. They load all incoming GPR/cache fields, CR3, null LDTR, TR busy state, and CR0.TS. |
| Commit boundary | Target HLT proves normal first instruction. Incoming IF plus pending IRQ0 proves the accepted post-commit IRQ path occurs before target-body execution. |
| Direct-JMP preflight ordering | Valid source-stack IDT handlers for target CS `#TS`, target-busy `#GP`, old/new-TSS-short `#TS`, and incoming-SS-limit `#SS` halt after delivery. Each retains source TR, all named outgoing fields, old busy descriptor, and correct target busy state; no outgoing TSS write or busy-state swap is observed. |
| Form legality | S2 retained the direct/indirect `EA`/`FF /5` `66h`/`67h` matrix. S3 retains `LOCK EA` and `LOCK FF /5` `#UD` nonpublication. The local `INS_FF` repair rejects only far-JMP `/5`; it does not alter the shared prefix policy. |

The smoke emits `M5:T329:S3:TSS32-IMAGE:OK`; its target-local GCC compile
command contains `-Wall -Wextra -Wpedantic -Werror`.

## Transfer

Task gates/CALL/backlink/NT remain T329 S4; nested return and double-fault
chains S5; non-null LDT images S6; and task paging/debug state S7. The direct
TSS state-machine record remains the shared dependency contract.
