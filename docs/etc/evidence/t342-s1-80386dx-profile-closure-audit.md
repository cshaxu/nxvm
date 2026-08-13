# T342 S1: 80386DX Profile Closure Audit

## Method

T342 is a verification task. It re-reads the Intel-oriented 80386DX closure
map, T340's horizontal form audit, and T341's vertical state audit before
classifying the current source graph. A row is accepted only when its proof
matches the actual form/state owner and mode, privilege, prefix/`LOCK`,
publication, and delivery boundary. This audit makes no implementation claim.

The source sweep was:

```powershell
rg -n "core_machine_cpu_profile_allows_form|INS_0F|insTable_0f|"
  "_Get(Operand|Address)Size|_kdf_check_prefix|PREFIX_LOCK|"
  "_d_modrm_(creg|dreg|treg)|MOV_(R32_(CR|DR|TR)|(CR|DR|TR)_R32)|"
  "_s_write_cr|_s_load_(gdtr|idtr|ldtr|tr)|_kma_(prepare|commit)_physical_linear|"
  "_ser_(task_transition_tss|task_return_tss|iret_protected_to_vm86)|"
  "_debug_(match|schedule|complete|deliver)|Exec(Ins|Int|Final)|"
  "VCPU_EFLAGS_(VM|RF|TF)" src/core/machine tests/machine CMakeLists.txt docs
```

The form/metadata hits resolve through the single primary/secondary dispatch,
prefix loop, width expressions, special-register decoders, memory/page-walk,
serializer/task plan, and S4 debug helpers already named by T340/T341. No
parallel 80386DX executor, form decoder, page walk, descriptor-table writer,
TSS materializer, or vector-1 matcher was found.

## Complete Residual Table

| 80386DX assigned row | Actual owner and exact proof | Disposition |
| --- | --- | --- |
| Operand/address size, prefixes, FS/GS, and `LOCK` form classification | One prefix loop, `_GetOperandSize`, effective-address owner, FS/GS selector loader, and `PREFIX_LOCK`; [T340 S2](t340-s2-prefix-width-fsgs-reconciliation.md) and its registered form smokes. | Accepted. Genuine 16/32 layout rules remain selected by the existing state/form owner; no duplicate width construction is present. |
| Ordinary primary integer, data, stack, string, I/O, and control forms | T316/T322 ordinary owner smokes, T328 legacy `LOCK`, and [T340 S4](t340-s4-80386dx-form-closure-audit.md). | Accepted for the assigned ordinary form matrix. System-state delivery is not inferred here; it is the next rows. |
| Non-privileged secondary integer and conditional forms | `INS_0F` dispatch and per-family FLAGS/publication owners; [T340 S3](t340-s3-nonprivileged-0f-reconciliation.md) with its named current gates. | Accepted. Each assigned near conditional, SETcc, bit, double-shift, IMUL, scan, and MOVX family has one dispatch and focused proof. |
| CR/DR/TR, LDT, and descriptor-table state | Special-register decoders and one mutable-state/cache/table owner; [T341 S2](t341-s2-control-debug-table-reconciliation.md). | Accepted. DR4/DR5 are 80386 reserved rejections; TR6/TR7 has the explicit no-persistent-cache external boundary. |
| VM86, task/TSS, and paging composition | Existing page walk, protected serializer, task plan/commit, and return owners; [T341 S3](t341-s3-vm86-task-paging-composition.md). | Accepted. True 16/32 TSS and frame layouts remain separate selections within the one transition/delivery construction. |
| Ordinary hardware debug and vector 1 | `ExecIns`, memory access record, `ExecInt`, and `ExecFinal`; [T341 S4](t341-s4-ordinary-debug-vector1.md). | Accepted. Execute faults, data/TF traps, RF, DR6 causes, DR7 task handoff, and real/protected/VM86 vector-1 delivery have one owner graph. |
| Intel-required 80386 rejections | Profile metadata, prefix and `LOCK` classification, form decoders, and retained focused rejections. | Accepted. Rejected rows retain the exact producer/restart/nonpublication evidence in their form or state owner; no legacy profile behavior is silently inherited. |
| VME/PVI, 486+ state/debug, persistent TLB/test registers, numerical x87, timing, device, and Windows behavior | [Closure map](80386-closure-map.md), Queue, and TODO. | Explicit external boundaries, not partial 80386DX rows. Each retains a named admission condition; none is consumed by the profile claim. |

## Reconciliation Result

The audit found and corrected stale closure-map wording that still described
ordinary debug, task/VM86 paging composition, and remaining CR/DR/TR/LDT/table
state as future work after T341 had closed them. The map now points to the
actual T341 evidence and leaves only true external boundaries deferred.

No source or evidence discrepancy returns to T340 or T341. T342 therefore
closes the 80386DX profile ledger. The Queue's final four-profile candidate
may consume this result only as a cross-profile reconciliation; it must return
any newly found implementation gap to the earliest 8086/80186, 80286, or
80386DX mechanism owner rather than repairing it under an audit label.
