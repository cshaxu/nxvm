# T341 S5: 80386DX System-State Closure Audit

## Audit Basis

This audit re-reads T341's original objective and the finite S1 allocation
ledger. It compares each allocated row with the current source owner,
accepted S2--S4 evidence, focused current-gate owner, Queue, and TODO. It is
an audit only: no source route, test, target, product artifact, or rule is
changed here.

The source sweep used the S5 packet query over tracked CPU execution and
machine-state source, current-gate registrations, S1--S4 evidence, Queue,
TODO, and history. The resulting production hits are the expected single
owners: CR and translation helpers; special-register decoders; table/cache
loaders; VM86 serializers; task plan/commit; and the ordinary debug helpers
added by S4. No second state writer, alternate page walk, parallel TSS
materialization, or duplicate vector-1 path was found.

## Allocation Reconciliation

| S1 allocation row | Current owner and accepted evidence | S5 disposition |
| --- | --- | --- |
| CR0/CR2/CR3 and ordinary paging translation | `_s_write_cr0_80386`, `_s_write_cr3_80386`, `_kma_prepare_physical_linear`, `_kma_commit_physical_linear`, and `ExecFinal`; [S3 composition](t341-s3-vm86-task-paging-composition.md), T325, T329 S7, and `current.core-machine-80386-paging-smoke`. | Accepted. VM86/task callers consume the one page-walk/preflight/commit owner; `#PF` restart/CR2/error-frame evidence remains exact. Persistent translation cache and TR6/TR7 remain the explicit external no-cache boundary. |
| CR/DR guest form privilege and publication | `_d_modrm_creg`, `_d_modrm_dreg`, `MOV_*_(CR|DR)`, and the CR writers; [S2 reconciliation](t341-s2-control-debug-table-reconciliation.md), `current.core-machine-debug-mov-s59-smoke`, and retained CR/paging proof. | Accepted. DR4/DR5 are reserved 80386 decoder rejections, not later aliases. Ordinary DR6/DR7 behavior is not inferred from transfer forms; it is the S4 row below. |
| Descriptor-table, LDT, and task-register forms | `INS_0F_00`, `INS_0F_01`, `_s_load_gdtr`, `_s_load_idtr`, `_s_load_ldtr`, and `_s_load_tr`; S2 evidence plus T318/T319/T321/T339 focused owners. | Accepted. Table and selector/cache forms retain their distinct image/layout and privilege/preflight contracts; task image consumption is the task row below. |
| VM86 entry, IDT/TSS delivery, and return | protected entry serializers, `_ser_iret_protected_to_vm86`, `ExecFinal`, `ExecInt`, and TSS SS0/ESP0 selection; [S3 composition](t341-s3-vm86-task-paging-composition.md), `current.core-machine-vm86-delivery-smoke`, and `current.core-machine-vm86-iret-smoke`. | Accepted for ordinary 80386 VM86 entry, return, paging source, restart, and stack/frame handoff. VME/PVI remains the named later-CPU TODO boundary. |
| Task/TSS transition, LDT, and paging interaction | `_ser_task_transition_tss_plan`, `_ser_task_transition_tss`, `_ser_task_return_tss`, and the actual 16/32 TSS readers; T329 S1--S7 plus S3 composition and `current.core-machine-task-switch-smoke`. | Accepted. The shared transition plan independently selects real TSS16/TSS32 layouts; no old/new construction fork or second task commit path reappeared. |
| DR6/DR7, breakpoint cause, and vector 1 | `ExecIns`, post-access memory record, `ExecInt`, `ExecFinal`, and task post-commit debug; [S4 graph](t341-s4-ordinary-debug-vector1.md), `current.core-machine-tf-db-s60-smoke`, `current.core-machine-vm86-delivery-smoke`, `current.core-machine-task-switch-smoke`, and `current.core-machine-debug-mov-s59-smoke`. | Accepted. Instruction faults, data/TF traps, RF, `DR6.Bn/BS/BT`, local/global DR7 task handoff, and real/protected/VM86 vector 1 have one disposition. Undefined/reserved DR controls and later debug extensions are not claimed. |
| Explicit outside-80386 boundary | Queue and TODO identify VME/PVI, 486+ control/debug behavior, persistent cache/test-register model, x87 numerical execution, timing, device policy, and Windows readiness. | Transferred once. These boundaries are not needed to describe any accepted 80386DX state row and cannot be filled by this audit. |

## Similar-Issue And Evidence Sweep

- The CR/DR/TR/table query found only the S2-owned form/loader paths and the
  explicit TR6/TR7 no-cache transfer; no form was silently promoted to a
  cache model.
- The VM86/task/paging query found only the S3-owned serializers, transition
  plan/commit, and common page-walk/final-delivery paths; no caller makes a
  private VM86 stack, task image, or page-table publication path.
- The DR6/DR7/RF/TF/vector-1 query found S4's single matcher/delivery path,
  the separate host-debug observation boundary, and the post-commit TSS debug
  producer. The older documentation's PIC-before-TF statement was corrected
  by S4; no second priority construction remains.
- The Queue/TODO sweep found the listed external boundaries with admission
  conditions. No T341-owned production hit is left Partial, Missing, or
  unclassified.

## Closure Result

T341 closes its 80386DX system-state package. Its successor is the Queue's
audit-only 80386DX profile closure: that task consumes this finite ledger and
may return a discrepancy to the earliest owner, but may not reimplement a
state mechanism under an audit label.
