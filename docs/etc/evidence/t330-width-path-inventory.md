# T330 Width-Path Construction Inventory

## Scope And Method

This is the T330 whole-file audit record for `src/core/machine/cpu_instructions.c`.
It classifies architectural mechanism families, not merely identifiers ending in
`_16` or `_32`. A family is examined when 16-bit and 32-bit forms can reach
different validation, state materialization, access preflight, commit, or
fault-delivery paths. Ordinary operand-width dispatch alone is not a separate
construction family when it already uses the same checked helper sequence.

The audit records one of four dispositions:

| Disposition | Meaning |
| --- | --- |
| Shared mechanism retained | One mechanism already owns validation and publication; width selects an Intel-required field size or layout. |
| Intel-required divergent layout | Separate code remains necessary because the architecture defines distinct TSS, gate, frame, or register-state layouts. T330 still compares its fault and commit boundary. |
| T330 remediation required | The paths implement one architectural transition but have accidental construction divergence. T330 must repair and prove it. |
| Separate architecture package | The candidate is real but its remediation would cross a named package boundary; T330 records the transfer rather than absorbing it. |

The source scan deliberately did not treat every `cpu_profile` branch as a
candidate. The file has 268 profile checks and 27 operand-byte switches; most
are instruction-form selection. The review first reduced those to the
mechanism families below, then followed every helper that can validate a
descriptor, materialize a target image, preflight memory, write a descriptor
or TSS, or publish CPU/cache state. This avoids both a name-only search and a
spurious rewrite of ordinary instruction handlers.

## Initial Whole-File Mechanism Inventory

| Mechanism family | Entry and shared owners | 16/32 distinction | Initial disposition | Required T330 action |
| --- | --- | --- | --- | --- |
| Operand/address decode and ordinary ALU/memory forms | `_d_modrm`, `_m_read_rm`, `_m_write_rm`, `_m_write_ref`, arithmetic helpers, `_GetOperandSize`, `_GetAddressSize` | Operand and effective-address widths | Shared mechanism retained | Verify that size is passed through the common checked decode/read/write pipeline; no framework rewrite or per-opcode duplicate audit. |
| Checked physical/linear/logical memory | `_kma_prepare_physical_linear`, `_kma_commit_physical_linear`, `_kma_*_logical`, `_kma_test_*` | Access byte count and 16/32-bit offset formation | Shared mechanism retained | Preserve the central checked-memory transaction boundary; any needed paging or exception redesign transfers to its named package. |
| Ordinary stack, near/far control transfer | `_kec_push`, `_kec_pop`, `_kec_call_*`, `_kec_jmp_*`, `_kec_ret_*`, `_s_test_ss_*` | SP/ESP, IP/EIP, operand/frame sizes | Shared mechanism retained | Recheck that the byte-size switch and preflight precede publication; retain true frame width differences. |
| Descriptor-table and control-state loads | `_s_load_gdtr`, `_s_load_idtr`, `_s_load_cr0_msw`, `_s_write_cr0_80386`, `_s_write_cr3_80386` | 286 versus 386 base/control fields | Intel-required divergent layout | Retain existing named T318/T319/T321/T325 boundaries; do not merge CR0/CR3 policy into T330 unless the audit proves a local construction defect. |
| Protected call gates | `_ser_call_far_call_gate`, `_ser_call_far_call_gate_32`, stack/selector helpers | 16-bit versus 32-bit gate/frame/parameter units | T330 remediation required | The 32-bit route rejects a nonconforming target whose DPL equals CPL, while the 16-bit route accepts that same-level CALL path. The 80386 PRM permits a CALL gate target DPL less than or equal to CPL. Preserve the distinct frame layouts, but correct the common privilege rule and prove same-level and inter-level fault/commit behavior. |
| Protected interrupt/trap delivery | `_ser_int_protected_16`, `_ser_int_protected_32_same`, `_ser_int_protected_32_outer`, `_ser_int_protected` | Gate type and 16/32-bit frame/outer-stack layout | Intel-required divergent layout | Both routes validate all descriptors and frame accesses before descriptor/cache/frame publication. The 32-bit same-CPL route delegates to its outer route only for a lower target CPL; the 16-bit route performs the same distinction inline. Retain these layouts; do not merge exception/IRQ policy into T330. |
| IRET and protected returns | `_e_iret`, `_ser_iret_protected_same`, `_ser_iret_protected_outer`, `_ser_iret_protected_to_vm86` | Frame width, outer frame, VM86 form | Intel-required divergent layout | Same- and outer-CPL helpers preflight all source frame fields and descriptor/cache checks before publication. VM86 return is a distinct 80386 frame. Retain existing T320/T323-owned behavior; VME/PVI and further return breadth stay transferred. |
| Task transition | `_ser_task_transition_tss`, `_ser_task_transition_tss_plan`, task-gate and nested-return callers | Old and incoming 286/386 TSS layouts, CR3, FS/GS, T-bit | T330 remediation required | Build one private plan/commit vocabulary that selects old and incoming TSS formats independently; prove all four width pairs and every task entry/return caller. |
| Post-switch debug trap | TSS32 debug word and `_e_except_n` post-commit route | Only 386 TSS has T-bit | Intel-required divergent layout | Keep a 386-only post-commit hook; audit that the unified task plan invokes it only for an incoming 386 TSS. |
| Port/string repetition and I/O | `_p_input`, `_p_output`, string helpers and I/O permission test | CX/ECX and SI/ESI/DI/EDI progression | Shared mechanism retained | Existing T316 string/I/O evidence owns instruction semantics; any provider or I/O bitmap architecture change remains separate. |
| Far pointer and loaded-selector forms | `CALL_PTR16_32`, `JMP_PTR16_32`, `LES/LDS/LSS/LFS/LGS`, `_e_call_far`, `_e_jmp_far`, `_e_load_far` | Pointer offset and destination register width | Shared mechanism retained | Form handlers decode the width-specific image, then converge on the common protected transfer/selector helper. The helper owns validation before final register publication. |
| Prefix/profile dispatch | `_adv`, `_GetOperandSize`, `_GetAddressSize`, prefix legality routes | 80386 attribute availability and instruction length | Separate architecture package | T316/T328 own the completed form and legacy LOCK matrices. No new width-construction defect was found in the dispatch boundary. |

## Confirmed First Defect

The current task-transition dispatcher chooses the entire construction path
from the current TR type. Consequently, it assumes the incoming TSS has the
same format as the outgoing one. The Intel 80386 PRM explicitly supplies task
switch timing matrices for both old and new TSS types, including 386-to-286 and
286-to-386 combinations. This is accidental construction divergence, not an
Intel layout rule.

The current 16-bit outgoing image also needs a dedicated audit of its complete
state span, including LDTR, before the unified plan is accepted. Source review
already identifies a concrete omission: `task_switch_state_16` spans `0x0e`
through `0x2b`, but the old write preflight covers only `0x1c` bytes and its
commit writes through DS at `0x28` without writing the LDTR at `0x2a`. The
unified implementation must use the full span and persist LDTR.

The protected 32-bit call-gate serializer has a second confirmed structural
drift: its target-code rule rejects `DPL == CPL`; the 16-bit serializer admits
it as the no-stack-switch case. The 80386 PRM permits CALL-gate transfer when
the target DPL is less than or equal to CPL. This is a shared privilege rule,
not a 16/32 frame-layout difference.

## Mechanism-Level Caller And Boundary Review

| Family | All relevant entry routes reviewed | Validation/materialization/commit conclusion |
| --- | --- | --- |
| Task transition | Direct far CALL/JMP TSS, far CALL/JMP task gate, IDT task gate, nested IRET backlink | All converge at `_ser_task_transition_tss`; its current old-TR-only format selection is the root defect. The replacement must select outgoing and incoming TSS formats independently before a single preflight/commit boundary. |
| Call gate | `_e_call_far` through `_ser_call_far_call_gate` and `_ser_call_far_call_gate_32` | Both prepare code and stack caches and preflight copied parameters/frame before mutation. The DPL-equality check is the only confirmed non-layout divergence; retain separate word/dword parameter images. |
| Protected IDT delivery | `_e_int3`, `_e_into`, `_e_int_n`, `_e_intr_n`, exception finalization through `_ser_int_protected` | Gate type selects the architecturally different frame. Both widths perform target and stack preflight before descriptor/cache/frame publication; no T330 remediation is indicated. |
| Protected IRET | `_e_iret` dispatches same-CPL, outer-CPL, VM86, and nested-task routes | Frame width and VM86 form are architecturally distinct. Same/outer helpers consume their whole frame before cache/state publication; no accidental width construction drift is indicated. |
| Ordinary control and stack | `_kec_push`, `_kec_pop`, `_kec_call_*`, `_kec_jmp_*`, `_kec_ret_*` | Each size switch remains inside a common checked helper chain. No duplicated validation/commit mechanism exists to consolidate. |

## Required Remediation Set Before Task Decomposition

1. Replace old-TR-selected task transition with a private transition plan that
   selects outgoing and incoming 286/386 TSS layouts independently; retain
   the actual layout differences for CR3, FS/GS, VM/T-bit, and stack fields.
2. Correct and prove the complete 286 outgoing state image, including LDTR and
   its access preflight.
3. Correct and prove the 32-bit CALL-gate same-CPL privilege rule without
   changing its 32-bit frame layout or the 16-bit route.
4. For both corrected mechanisms, exercise every caller, success pair or
   privilege branch, and validation/fault/commit boundary that can differ.

## Required Next Audit Nodes

1. Finish task-transition source/target format, caller, image-write, fault,
   and post-commit mapping; then create the implementation S brief.
2. Compare 16/32 call-gate serializers against the retained T323 matrix.
3. Compare 16/32 protected-gate and IRET serializers against T320/T323
   validation, frame, and fault-context evidence.
4. For every retained shared mechanism, record the exact common helper chain
   and why a new abstraction would be redundant.

## T330 S1 Remediation And Evidence

`_ser_task_transition_tss` now reads the current and target TSS descriptors
before choosing a private `_ser_task_transition_tss_plan`. The plan selects
the outgoing and incoming image formats independently. It therefore owns all
four 80386 combinations (286-to-286, 286-to-386, 386-to-286, and 386-to-386),
while keeping the two real TSS layouts separate: the 386 image owns CR3,
FS/GS, and the T-bit post-commit hook; the 286 image owns its 16-bit register
and selector fields.

The plan preflights the complete outgoing and incoming ranges, backlink and
descriptor writes before publication. Its 286 outgoing writer covers the
complete `0x0e`--`0x2b` span, including LDTR at `0x2a`. The task-only data
selector validator now materializes an invalid cache for a null ES/DS/FS/GS
selector, while retaining `#TS(0)` for a null SS; this follows task-switch
selector semantics and is exercised by the 386-to-286 nested return.

`core_machine_task_switch_smoke` proves each width pair through direct far
JMP, nested far CALL, nested task-gate entry, and bounded nested IRET return.
It records outgoing 286 LDTR persistence, backlink and busy-bit publication,
TR/cache state, CR0.TS, and the layout-specific CR3/FS/GS dispositions. The
existing fault corpus continues to exercise descriptor, TSS-size, stack,
LDT, paging, and debug-trap boundaries through the same dispatcher.

The caller/write sweep is closed for `_ser_task_transition_tss`: direct far
TSS transfer, task-gate transfer, IDT task-gate transfer, and nested IRET
return all enter through it. The plan is the only T330 S1 writer of the
cross-format outgoing image, backlink, and old/new TSS descriptors; its
checked-memory preflight remains in the existing descriptor/TSS ownership
layer. No generic memory, descriptor, paging, or exception mechanism changed.

## T330 S2 Call-Gate Convergence And Evidence

The 32-bit CALL-gate serializer now matches the retained 16-bit privilege
decision: a nonconforming target code descriptor is valid when its DPL equals
the current CPL, and only a target DPL greater than CPL faults. The 32-bit
route now reads the TSS, selects a replacement SS:ESP, copies parameters, and
writes the old SS/ESP frame fields only when the target CPL is lower. Its
same-CPL path instead preflights and publishes just the 32-bit return EIP/CS
frame on the current stack; it requires no TR/TSS.

`core_machine_call_gate_privilege_entry_smoke` adds a CPL0-to-CPL0 32-bit
CALL-gate transfer with an invalid TR. It proves the equal-DPL target is
accepted, EIP/CS select the target, the current SS remains selected, ESP moves
by exactly two dwords, the saved EIP/CS image is correct, and the adjacent
stack word plus EFLAGS and non-stack GPRs remain unchanged. The retained
privilege-entry corpus proves the lower-CPL TSS/parameter/outer-stack path and
its descriptor, TSS, and stack failure boundaries. This preserves the genuine
word/dword frame distinction rather than abstracting the two layouts.

## T330 S4 Corrective: Single Task-Transition Construction Owner

The whole-source corrective audit found that the former private
`task_switch_plan_32`, `_s_task_plan_transition_32`,
`_s_task_commit_transition_32`, and `_ser_task_switch_tss_32` construction
chain remained compiled after S1, but no task entry called it. Keeping this
dead second model would permit future 32-bit-only changes to diverge from the
accepted independent old/new-format plan.

S4 removes that unreachable chain. The retained private owners are explicit:
`_ser_task_transition_tss_plan` stages validation and materialization,
`_ser_task_transition_tss` selects the plan, `_ser_task_switch_tss` reaches it
for direct and task-gate entry, and `_ser_task_return_tss` reaches it for the
backlink return. `_s_task_write_state_16` and `_s_task_write_state_32` remain
layout writers, not parallel transition constructors.

`verify-t330-task-transition-construction` mechanically rejects each removed
identifier and requires the canonical plan plus both entry and return call
fragments. The retained task-switch smoke continues to cover all 286/386
source/target pairs, direct/task-gate entry, nested return, descriptor/TSS,
LDT, stack, paging, and debug boundaries. No architectural TSS layout,
descriptor/memory owner, or task-switch behavior changed.

## T330 S5 Media-Provider Construction Reconciliation

The whole-repository construction sweep found the two production
`core_machine_media_provider` implementations in `vm/machine/fdd.c` and
`vm/machine/hdd.c`. Both providers now reject a present medium whose backing
pointer is null before any read, write, or format dereference. FDD now follows
the same construction boundary as HDD: `create_for` publishes present state
and advances the media generation only after backing exists. HDD already used
a candidate-and-commit create path; its format callback was the remaining
backing-precondition omission and now shares the provider contract.

`vm-media-provider-smoke` binds deliberately present-but-null FDD and HDD
contexts through the registry and also invokes their provider callbacks
directly. Read, write, and format deterministically return `PERMANENT` without
publication. It separately simulates a finalized FDD allocation failure and
proves `create_for` leaves presence, generation, and backing unchanged. The
retained normal FDD/HDD create, replacement, insert, remove, formatting, and
atomic-save cases remain in the same smoke. The sweep classified all FDD/HDD
create, replace, insert, remove, read, write, and format routes: candidate
commit routes are aligned; direct FDD I/O and both format callbacks were fixed;
remove/save paths already fail before mutation when persistence lacks backing.
No shared media interface, controller protocol, or public ABI changed.

## T330 S6 CALL-Gate Preflight Ordering

The Intel 80386 CALL operation specifies the more-privilege sequence: obtain
the replacement stack from the TSS, validate its selector and descriptor,
verify its frame capacity and target instruction pointer, then copy the old
stack parameters before publishing the replacement frame. The retained
[80386 CALL reference](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/CALL.htm)
lists the ordering explicitly. Parameter word/dword width and the 16/32-bit
TSS/frame layouts remain architecturally distinct; their validation ordering
does not.

The source sweep covered both call-gate serializers and their only far-CALL
dispatcher. Before S6, the 16-bit path read all old-stack parameter words
before checking TR/TSS/new SS, while the 32-bit path checked the new selector
but read parameter dwords before it preflighted the replacement frame. Both
routes now use the same mechanism boundary: validate target code and TSS/new
SS, prepare and preflight the replacement stack, read the bounded old-stack
parameter image, then write descriptor accessed bits and publish stack/CPL/CS.

The retained 16-bit and 32-bit owner smokes each create a controlled dual
fault: the old stack makes the first parameter unreadable while the TSS supplies
an invalid replacement SS selector. A same-CPL vector-10 handler remains
capable of taking the resulting `#TS`: the 16-bit null selector reports
`#TS(0000)` and the 32-bit selector-with-RPL reports `#TS(0010)`. Both widths
now deliver that target-stack `#TS` rather than reaching the old-stack `#SS`;
no target descriptor, replacement stack, CPL, or call-frame publication
precedes the selected fault. Existing
success, isolated source-stack, isolated TSS/new-SS, target-stack, descriptor,
and IRQ vectors remain in their respective owner smokes. No exception-delivery,
descriptor/TSS helper, paging, or frame-layout mechanism changed.

## T330 S7 State-Machine Matrix Rule Map

The Execution rule now requires a mechanism matrix before P1 for stateful CPU
work. T330 provides the concrete review map: task transition has independent
old/new TSS width selection and all direct, task-gate, IDT, and backlink callers;
CALL gates have width-specific frames but one validation/preflight/commit
boundary; protected exception/IRQ entry and IRET retain distinct same-, outer-,
and VM86 frames; descriptor/control state owns table and control-register
publication; and paging owns translation, access, fault, and rollback state.
Each is a mechanism rather than a bag of opcodes, so its matrix explicitly
records callers, state dimensions, fault priority, preflight, commit, and
transfers. Ordinary operand/address dispatch remains outside this special rule
when it already reaches the common checked decode/read/write pipeline.

The rule does not attempt a prose parser or force false unification. Its
enforcement point is the active packet plus actual-change review: the executor
must supply the matrix before P1, and the reviewer compares it with the caller
graph and tests before accepting a symptom repair. T330 S1/S4, S5, and S6 are
the three observed recurrence classes that motivated it: parallel task
constructors, provider construction/commit publication, and width-divergent
CALL-gate preflight ordering.
