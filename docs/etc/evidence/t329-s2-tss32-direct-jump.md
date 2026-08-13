# T329 S2: 32-bit TSS Direct-Jump Evidence

## Scope

This record covers the active T329 S2 bounded transition only: Intel 80386
protected, CPL0 direct far `JMP` from a busy 32-bit TSS to an available
32-bit TSS, paging disabled, and an incoming null LDTR. It implements the
state machine in
[the T329 state-machine record](t329-task-switch-state-machine.md). It
neither closes nor changes task gates, task `CALL`, nested-task `IRET`,
non-null LDT, task paging, or debug-state behavior.

## Implementation Contract

`task_switch_state_32` names the image spanning TSS offsets `1Ch` through
`63h`; compile-time assertions require four-byte selector slots and the exact
`48h` byte span. The private `task_switch_plan_32` does all admitted reads
and checks before commit:

1. old busy TR and new available TSS descriptors, presence, DPL, and limits;
2. old-image write span, new-image read span, and both descriptor write spans;
3. target image CR3/null-LDTR contract;
4. CS, SS, DS, ES, FS, and GS cache preparation; and
5. target instruction and incoming-stack boundary checks.

Only then does the commit write the outgoing image and busy descriptors, then
publish the incoming CPU, cache, TR, LDTR, and `CR0.TS` state. The direct
32-bit branch returns from the 16-bit handler after success; it cannot fall
through and reinterpret its newly loaded TR as a 16-bit TSS.

## Current Focused Matrix

The owner smoke `core-machine-task-switch-smoke` emits
`M5:T329:S2:TSS32-JMP:OK`. Its S2 vectors currently prove:

| Form or boundary | Evidence |
| --- | --- |
| `EA ptr16:16` | 32-bit TSS transition with complete outgoing 32-bit GPR, EIP, CR3, EFLAGS, and selector image, plus full incoming 32-bit GPR image, CR3, FS/GS, null LDTR, busy bits, TR, and TS. |
| `66 EA ptr16:32` | Same transition with the 32-bit direct operand and its distinct outgoing instruction boundary. |
| `FF /5 m16:16` | Indirect 16-bit operand route. |
| `66 FF /5 m16:32` | Indirect 32-bit operand route. |
| `67 FF /5 m16:16` | 32-bit effective-address route. |
| `66 67 FF /5 m16:32` | Combined operand/address attribute route. |
| Incoming invalid CS | Preflight rejection at the existing no-IDT terminal `#DF` boundary, with source EIP/EAX/TR retained; no commit is claimed. |
| Target busy TSS | Descriptor rejection leaves the source task active at the no-IDT terminal boundary. |
| Old/new TSS short | Both outgoing-write and incoming-read required spans reject before commit. |
| Incoming SS limit | Target stack preflight rejects before the new task state is published. |
| `LOCK EA` and `LOCK FF /5` | Both direct and indirect far-JMP routes reject with `#UD` before pointer read or TSS publication, retaining source GPR, segment-cache, EIP, and TR state. The narrow `INS_FF` fix rejects only its far-JMP `/5` forms; it does not broaden the shared LOCK policy. |
| Incoming IF plus pending IRQ0 | Successful transition publishes IF; IRQ0 is accepted before target-body execution, reaches the handler HLT, updates ISR/IRR and the incoming stack frame, and leaves no target-body side effect. |

The no-IDT fault cases intentionally do not read GDT/TSS memory after the
machine enters its faulted lifecycle; that public boundary rejects ordinary
memory operations. They therefore prove the source-state nonpublication that
remains observable at that boundary and do not manufacture a test-only raw
memory interface.

## Completion Boundary

This completes the admitted direct-JMP, null-LDTR, paging-disabled slice. The
only remaining S2 workflow step is the required actual-change review and
governance closure. Task gates, task `CALL`, nested-task `IRET`, non-null LDT,
task paging, and debug-state behavior remain assigned by the state-machine
record to later T329 slices.
