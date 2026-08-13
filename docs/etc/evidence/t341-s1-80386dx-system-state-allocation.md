# T341 S1: 80386DX System-State Allocation Ledger

## Scope And Method

T341 allocates vertical architectural state, not a convenient list of system
opcodes. The review starts from the 80386DX closure map, T340's one-time state
transfer, the retained task/VM86/paging/control records, the current executor
routes, current-gate ownership, and TODO boundaries. A row is reusable only
when the retained evidence proves the same mode, privilege, validation,
publication, frame/stack, and fault/delivery contract.

## State-Owner Ledger

| State owner and form boundary | Actual route / reader-writer boundary | Retained proof or disposition | T341 allocation |
| --- | --- | --- | --- |
| CR0/CR2/CR3 and ordinary paging translation | `_s_write_cr0_80386`, `_s_write_cr3_80386`, `_kma_prepare_physical_linear`, `_kma_commit_physical_linear`, page-fault producer, `ExecFinal`; task transition preflights incoming CR3. | T325 S1--S3 proves 80386 non-PAE translation, A/D, CR2/CR3, `#PF`, and no persistent cache; T329 S7 proves task source/preflight and incoming commit. | **S3 composition audit:** prove that VM86/task callers consume the accepted paging contract without creating a second validation/commit path; no standalone page-walk reimplementation. Persistent TLB/TR6/TR7 remains external. |
| CR/DR guest form privilege and state publication | `MOV_R32_CR`, `MOV_CR_R32`, `MOV_R32_DR`, `MOV_DR_R32`, protected privilege gate, CPU state fields. | T304/T325 retain CR form and paging control evidence; `core-machine-debug-mov-s59-smoke` owns current DR form proof. | **S2 control/debug form audit:** reconcile the whole CR/DR form/privilege/rejection matrix and actual state writers. Any reproduced shared validation defect stays with S2. |
| Descriptor-table, LDT, and task-register forms | `INS_0F_00`, `INS_0F_01`, descriptor cache and table serializers, `LLDT`/`LTR`, task switch consumers. | T316 S61--S63, T318, T319, T321 S5, T339 S2/S3, and T329 S6 provide bounded form, cache, and VM86 rejection evidence. | **S2 control/table audit:** establish one form-to-cache/table transition ledger and transfer no remaining form by implication. Task-LDT image composition belongs S3. |
| VM86 entry, IDT/TSS delivery, and return | protected interrupt serializers, `_ser_iret_protected_to_vm86`, `ExecFinal`, `ExecInt`, TSS SS0/ESP0 selection. | T320 S1/S2 proves VM86 `#GP/#UD/#NM` and IRQ delivery plus bounded CPL0 IRET return; T321/T326 retain common delivery / `#UD` producer evidence. | **S3 composition audit:** verify every adopted VM86 producer uses the same frame/stack and restart boundary; VME/PVI remains external. |
| Task/TSS state transition, LDT and paging interaction | `_ser_task_transition_tss_plan`, `_ser_task_transition_tss`, `_ser_task_return_tss`, TSS 16/32 image readers/writers, busy/backlink/NT, CR3/LDTR preparation. | T329 S1--S7 proves direct/task-gate/nested task paths, independent 16/32 TSS formats, LDT images, CR3 preflight/commit, and task debug-trap ordering. | **S3 composition audit:** confirm task callers and VM86/paging handoffs retain the plan/commit boundary. Broader task/VM86 instruction breadth is explicit transfer, not a hidden completion claim. |
| Debug registers, breakpoint cause, and vector-1 delivery | DR6/DR7 state, instruction debug checks, `ExecFinal`/`ExecInt` vector-1 path, task debug-trap handoff. | T329 S7 proves only TSS debug-trap ordering; T337 transfers ordinary DR6/DR7; present debug MOV smoke proves forms, not breakpoint cause/delivery. | **S4 implementation/audit owner:** complete ordinary DR6/DR7 matching, cause publication, vector-1 frame/restart, priority, and fault atomicity matrix or transfer an exact external condition. |
| 80386-only external boundaries | VME/PVI, 486+ CR4/PSE/PAE/INVLPG semantics, persistent TLB/test registers, x87 numerical execution, timing, device policy. | Current TODO and closure map retain explicit admission conditions. | **External:** no T341 implementation claim. |

## Required Later Sequence

1. **S2 - control, debug-register, and table-state reconciliation.** Complete
   source/form/privilege/cache publication matrix for CR/DR/TR/LDT/table
   forms. It may close directly only if the audit proves all rows already have
   exact evidence; otherwise it repairs the one shared state owner.
2. **S3 - VM86, task, and paging composition.** Reconcile the four
   state-machine contracts across real/protected/VM86 task and paging callers,
   preserving real 16/32-bit TSS and frame layouts.
3. **S4 - ordinary breakpoint and vector-1 composition.** This is the only
   presently unproved state-owner row; it must map DR6/DR7 matching, delivery,
   priority, frame, publication, and task interaction before any fix.
4. **S5 - T341 closure audit.** Audit-only; it cannot repair a row and must
   transfer every residual once to an earlier mechanism S, TODO, or a stated
   post-80386 boundary.

## Static Review Surface

The allocation review covers all tracked `src/core/machine` execution and
machine state sources; current-gate registrations and owner tests; T320,
T321, T325, T326, T329, T337, T339, and T340 evidence; the Queue, closure map,
and TODO. Queries include `CR0|CR2|CR3|DR[0-7]|TR|LDT|TSS|VM86|VCPU_EFLAGS_VM|`
`_kma_prepare_physical_linear|_ser_task_transition_tss|ExecFinal|ExecInt`.
This is a source-owner inventory, not a repository-wide refactor request.
