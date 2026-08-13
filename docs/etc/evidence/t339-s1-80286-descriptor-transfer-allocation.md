# M5 T339 S1: 80286 Descriptor And Protected-Transfer Allocation

## Scope And Method

This is T339's allocation record, not a claim that historic 80386-oriented
smokes close 80286 behavior.  The Intel 80286 architecture is the authority
for the 16-bit protected-mode layouts; the Intel 80386 authority applies only
where the same route is shared and the difference is explicitly classified.
The record begins with the current implementation graph, then maps retained
evidence and assigns every remaining 80286 row to one later T339 S or an exact
80386DX receiving owner.

The five mechanism owners are intentionally vertical:

| Owner | Current route and state boundary | T339 responsibility |
| --- | --- | --- |
| Descriptor/table control | `INS_0F_00`, `INS_0F_01`, `LAR_R32_RM32`, `LSL_R32_RM32`, `CLTS`; `_d_modrm_table_memory`; `_s_load_gdtr`, `_s_load_idtr`, `_s_load_ldtr`, `_s_load_tr`, `_s_load_cr0_msw` | Form decode, table/selector source or destination preflight, table/cache publication, privilege and restart rules. |
| Selector/cache materialization | `_ksa_prepare_*_sreg`, `_ksa_load_sreg`, `_s_load_{cs,ss,ds,es}`, `_e_load_far`, `_e_pop_sreg` | Validate before cache/register publication and preserve 80286 data/code/stack semantics. |
| Protected entry | `_ser_int_protected_16`, `ExecInt`, gate/table readers, `_ksa_prepare_code_sreg`, `_ksa_prepare_stack_sreg` | 16-bit gate selection, frame construction, outer-stack selection, error frame, external origin, and prepublication failure. |
| Protected return | `_e_iret`, `_ser_iret_protected_same`, `_ser_iret_protected_outer`, stack peek/pop helpers | 16-bit same/outer return source preflight, cache/frame publication, restored privilege and IRQ composition. |
| Task transition | `_ser_task_switch_tss`, `_ser_task_return_tss`, `_s_load_tr`, `_s_load_ldtr`, far/task-gate dispatch | TSS16 image, busy/back-link, task-gate/direct transition, validation-plan-commit ordering, and task return. |

## Existing Evidence Disposition

| Existing record | Usable 80286 evidence | Not silently inherited |
| --- | --- | --- |
| T318 SGDT/SIDT | `0F 01 /0,/1`, 80286 real/protected store image, memory-only and write-preflight boundaries. | 80386 `66/67` and VM86 assertions transfer to T341/T342. |
| T319 LGDT/LIDT | `0F 01 /2,/3`, 80286 CPL0, pseudo-descriptor load and source atomicity. | 80386 attributes and VM86 delivery transfer. |
| T316 S57/S58/S61/S62/S63 | Bounded `LAR/LSL`, `VERR/VERW`, `SLDT/STR/LLDT/LTR`, `SMSW/LMSW`, and `CLTS` form proof. | Historic 80386-only width/prefix presentation is not 80286 closure evidence. |
| T323 S1--S7 | 16-bit protected data/cache, direct far transfer, same/outer gate entry, outer IRET, and 16-bit call-gate checkpoints. | 32-bit frame, TSS32, VM86, and paging paths transfer. |
| T329 S1--S7 | The task transition plan/commit discipline and bounded TSS16 direct/task-gate evidence. | TSS32 image, CR3/paging, debug-trap, VM86, and LDT32 breadth transfer. |

Historic evidence is accepted only after the owning S compares its exact
profile, mode, frame width, selector/table type, fault producer, and
publication boundary. A named smoke is never a substitute for that comparison.

## Future T339 Allocation Ledger

| Later S | Complete mechanism matrix | Required rows and proof boundary | Exact transfers |
| --- | --- | --- | --- |
| S2: descriptor-table and system-word forms | `0F 01 /0`--`/4,/6` and `0F 06`; table-register/table-memory and CR0 low-word materialization. | SGDT/SIDT/LGDT/LIDT/SMSW/LMSW/CLTS across 80286 real/protected CPL conditions; memory-only ModRM; pseudo-descriptor/image widths; GDT/IDT/MSW publication; read/write limit atomicity; `#UD`/`#GP` restart; no-shadow IRQ after successful forms. | `66/67`, VM86, CR0 32-bit/paging effects -> T341/T342. Accepted T328 owns pre-386 `LOCK`. |
| S3: selector-query and cache-load forms | `0F 00 /0`--`/5`, `0F 02/03`, `8C/8E`, legacy segment POP/PUSH, and `C4/C5` protected consumers. | SLDT/STR/LLDT/LTR/VERR/VERW/LAR/LSL plus selector/cache loads: null, TI, GDT/LDT, type, DPL/RPL, present/accessed, source-limit and cache/destination publication. Preserve only true 16-bit selectors and operands. | FS/GS, `LSS/LFS/LGS`, 32-bit operands/addressing -> T341. VM86 selector behavior -> T342. Accepted T328 owns `LOCK`. |
| S4: protected 16-bit entry and gate transfer | Direct far `CALL/JMP`, software/external/NMI entry, 16-bit interrupt/trap/call gates and TSS16 outer-stack selection. | Code/gate DPL/type/present/limit validation; three/four/five-word frame images; IF/TF and error-code rules; PIC/NMI order; same/outer CPL and prepublication failure. All gate readers/writers are swept together. | 32-bit gates/frames, VM86 entry, paging fault composition -> T342. Accepted T328 owns `LOCK`; 80386 attributes -> T341. |
| S5: protected 16-bit return and frame composition | `IRET`, `RETF`, and matching return stack/cache publication after S4. | Same/outer return, source-frame preflight, CS/SS/cache selection, IOPL/IF permitted FLAGS, 16-bit SP high-half discipline, restored-IF IRQ order, and no-publication rejection. | Return-to-VM86, 32-bit frames, NT/task return and VME/PVI -> T342. |
| S6: TSS16 task-state closure | Direct far TSS jump/call, task-gate entry, nested task IRET, busy/back-link, LDT/TI and TSS16 image. | TSS16 descriptor/type/limit/present, incoming/outgoing image, selector/cache materialization, old/new busy state, backlink/NT, target stack/code preflight, fault-before-commit and post-commit IRQ/debug ordering. | TSS32, CR3/paging, debug-trap and VM86 task interactions -> T342. |
| S7: 80286 descriptor-transfer closure audit | All S2--S6 ledgers and their shared validation-to-delivery edges. | Compare source, metadata, form matrix, state writers, focused proof, retained T328 `LOCK` ownership and every historic claim. Return a gap to S2--S6; do not implement under audit. | Produce the input ledger for T341/T342 transfers. |

## Required Cross-Cutting Invariants

Every implementation S above must prove the following for its assigned
mechanism rather than copying a nearby 80386 result:

1. 80286 real mode is `#UD` where the instruction is protected-only; protected
   mode has the exact selector/table/gate privilege rule.
2. A genuine 16-bit operand, effective-address, stack, frame, and TSS layout
   remains explicit. Shared private preparation may select either layout but
   may not flatten validation, preflight, commit, or restart behavior.
3. Any fault after a source read but before commit has a recorded state and
   memory/cache nonpublication boundary; a delivered exception may change only
   the documented delivery state.
4. `LOCK` is not inferred from the 80386 whitelist. Accepted T328 owns the
   pre-386 legality policy for each allocated memory-capable mechanism.
5. No VM86, 32-bit width, FS/GS, paging, debug, or x87 conclusion is made
   from an 80286 vector. Those transfers are explicit above.

## Similar-Issue Inventory

The allocation sweep is bounded by the following source searches and current
owner evidence:

```text
rg -n "INS_0F_00|INS_0F_01|LAR_R32_RM32|LSL_R32_RM32|CLTS" src/core/machine/cpu_instructions.c
rg -n "_s_load_(gdtr|idtr|ldtr|tr|sreg)|_ksa_prepare|_ksa_load_sreg" src/core/machine/cpu_instructions.c
rg -n "_ser_int_protected_16|_ser_iret_protected|_ser_task_(switch|return)|_e_(iret|load_far|jmp_far|call_far)" src/core/machine/cpu_instructions.c
rg --files tests/machine | rg "(80286|descriptor|dttr|sgdt|lgdt|sreg|protected_16|task|iret|far)"
```

No implementation is performed by S1. A defect found while an implementation
S executes is repaired at its owner after the caller/write/fault sweep, or is
transferred through Queue/TODO under the active packet's stop conditions.

## S1 Result

The T339 boundary contains no unallocated 80286 descriptor/table/protected
transfer row. S2--S6 are the only implementation owners; S7 is audit-only.
The task must not close until every S2--S6 row is proved or transferred as
listed, and S7 reconciles that disposition.
