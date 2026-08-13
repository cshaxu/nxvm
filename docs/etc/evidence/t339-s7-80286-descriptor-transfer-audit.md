# M5 T339 S7: 80286 Descriptor-Transfer Closure Audit

## Audit Scope

S7 re-ran the S1 allocation searches across `cpu_instructions.c`, the current
gate registrations, and S2--S6 owner evidence. It is an audit-only closure:
this document records the current source/evidence disposition and does not
create a new runtime path.

| Mechanism owner | Current source owner | 80286 proof and disposition |
| --- | --- | --- |
| Table/system word | `INS_0F_01`, `_d_modrm_table_memory`, table pseudo-descriptor readers/writers, `_s_load_{gdtr,idtr}`, `_s_load_cr0_msw`, `CLTS` | S2 closes `0F 01 /0`--`/4,/6` and `0F 06`: real/protected form disposition, complete source/destination preflight, CPL producer boundary, and successful PIC ordering. |
| Selector/cache | `INS_0F_00`, `LAR_R32_RM32`, `LSL_R32_RM32`, `_s_load_{ldtr,tr,sreg}`, `_e_load_far` | S3 closes the selector-query, table-selection, cache-publication and genuine 16-bit far-load rows, including GDT/LDT/TI, null, type, present, RPL/DPL, and source boundaries. |
| Protected entry | `_e_{call,jmp}_far`, `_ser_int_protected_16`, `_ser_call_far_call_gate`, `ExecInt` | S4 closes direct, software, PIC/NMI, same/outer 16-bit gate and call-gate publication. TSS16 outer-stack selection remains the true 16-bit layout owner. |
| Protected return | `_e_iret`, `_ser_iret_protected_{same,outer}`, `_ser_ret_far_{same,outer}` | S5 closes same/outer IRET and RETF/RETF imm16 source-frame preflight, cache materialization, permitted FLAGS and restored-IF PIC ordering. |
| Task transition | `_ser_task_{transition,switch,return}_tss`, `_s_load_{tr,ldtr}`, direct far and task-gate dispatch | S6 closes direct `JMP/CALL`, GDT/IDT task gates, nested IRET, busy/backlink, incoming LDT/TI, target TSS validation, active-TR boundary, and post-commit PIC ordering. |

## Validation, Publication, And Fault Crosswalk

No audited mechanism has a second same-layout publisher. The source keeps
distinct serializers only where Intel layouts differ: 16- versus 32-bit
frames/TSS images, VM86 returns, and task return. The accepted 80286 owners
preflight their declared source/destination spans and cache/descriptor input
before their respective table/cache/frame/target-state publication.

Task switching is the one intentional non-flat case: an outgoing TSS image may
be saved before a later incoming-state validation fault. S6 therefore proves
the producer/restart and absence of target TR/cache commit, rather than making
a false whole-memory atomicity assertion. This is a genuine task-transition
ordering distinction, not a duplicate construction path.

## Complete T339 Ledger And Transfers

| Residual boundary | Receiving owner |
| --- | --- |
| 80286 `LOCK` legality for memory-capable S2--S6 forms and inherited profile rows | T340, 80286 `LOCK` and profile closure |
| `66/67`, FS/GS, 32-bit operands/effective addresses, 32-bit gate/frame and TSS32 layouts | T341, 80386DX width/prefix/integer closure |
| VM86 entry/return/task semantics, paging/CR3, debug/DR state, generic 80286 `#TS/#SS` delivery composition | T342, 80386DX system/VM86/paging/debug closure |

No S2--S6 allocation row remains partial or unclassified after these explicit
transfers. The current owners and exact current-gate targets are retained in
the S2--S6 evidence documents; this audit does not replace them.

## Verification Record

The audit re-runs the accepted owner search set, verifies the current artifact
identity, documentation governance and whitespace, and executes the full
current gate. No implementation defect was found, so no production change or
similar-code repair is required.
