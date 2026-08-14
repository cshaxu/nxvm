# M5 T358 S3: Selector And Transition Coherence

## Audit Boundary

This S re-audits the production selector/cache transition paths rather than
counting instructions.  The authority is the Intel 80286/80386 protected-mode
descriptor, task, gate, and return model; retained implementation evidence is
T301, T305--T308, T320, T330, and the current owner smokes.  The source sweep
used:

```text
rg -n "_s_load_sreg\(|_ksa_prepare_(code|stack)_sreg\(|_s_load_(ldtr|tr)\(|_ser_task_transition_tss\(|_ser_task_return_tss\(|_ser_call_far_call_gate|_e_(iret|call_far|jmp_far|load_far)\(" src/core/machine/cpu_instructions.c
rg -n "_s_write_xdt\(|_ksa_write_xdt\(|_kec_push\(|_s_task_write_state_(16|32)\(" src/core/machine/cpu_instructions.c
```

Every hit was reviewed for source validation, candidate-cache materialization,
memory/descriptor preflight, first publication, and fault preservation.

## Current Production Ledger

| Family | Shared owner and publication boundary | Current disposition |
| --- | --- | --- |
| MOV/POP segment and LxS loads | `_s_load_sreg` validates and writes an accessed descriptor before publishing the target cache. `_e_pop_sreg` reads the source before cache load and advances stack only after success; `_e_load_far` has no fallible action after cache validation except its bounded register publication. | One load owner; retained T301 covers real/protected/null/type/present/atomicity and LxS. No construction drift found. |
| CS/far transfer and CALL gates | `_e_call_far`/`_e_jmp_far` classify code, call-gate, task-gate, and TSS selectors. `_ser_call_far_call_gate` keeps 16-/32-bit frame layouts distinct but both preflight target and replacement stack before descriptor/cache/frame publication. | T330 S2/S6 closed the prior equal-DPL and preflight-order divergence. Current code retains that sole ordering. |
| Interrupt and exception entry | `_ksa_prepare_code_sreg` and `_ksa_prepare_stack_sreg` build private candidates; real, protected-16, protected-32 same/outer, and VM86 serializers choose their own Intel frame layouts and publish only after their applicable preflight. | Retained T305/T308/T320/T358 S2 evidence. Distinct layouts are architectural, not duplicate construction. |
| IRET and protected returns | `_ser_iret_protected_same`, `_ser_iret_protected_outer`, and `_ser_iret_protected_to_vm86` read and validate complete applicable frames before CS/SS/cache/EIP/flags publication. | Same, outer, and VM86 return images are distinct Intel forms. T320 retains the VM86 inverse evidence; S3 found no new shared-owner divergence. |
| LDTR/TR and task transition | `_s_load_ldtr`/`_s_load_tr` use the common selector load owner. `_ser_task_transition_tss_plan` independently selects outgoing and incoming 286/386 images; `_ser_task_transition_tss`, task-gate entry, and `_ser_task_return_tss` are its callers. `_s_task_write_state_16/_32` are layout writers only. | T330 S1/S4 removed the old parallel 32-bit constructor and proves all source/target layout pairs and callers. No second constructor or write route remains. |
| Descriptor-table/control state | Table/control helpers select actual 286/386 pseudo-descriptor/control layouts and remain outside selector-cache construction. | Intel-required layout difference; prior T318/T319/T321/T325 evidence remains the owner. No S3 rewrite is justified. |

## Reproduced Contract Reconciliation

The S2 repair made protected 80286 `#SS` delivery use vector 12, as the
existing 80286 `#TS`/`#SS` entry evidence requires.  Two outer-return owner
smokes still classified a non-present outer `SS` frame as terminal failure.
The focused re-run showed the correct vector-12 handler stop, error code
`0x0030`, and preserved old kernel CS/SS cache state.  Both shared return
case tables now classify that same input as delivered `#SS`; the two current
owners pass together.  This is an obsolete test contract, not a second
selector construction path or a production change.

## Result And Transfer

Apart from that reconciled test expectation, no new accidental
validation/materialization/preflight/commit divergence was reproduced. The
prior T330 repairs remain singular and the owner tests below exercise their
state and fault boundaries:

| Owner smoke | Retained proof used by S3 |
| --- | --- |
| `core-machine-segment-selector-smoke` | Segment cache load, null/type/present outcomes, LxS and POP atomicity. |
| `core-machine-task-switch-smoke` | All 286/386 source/target TSS pairs, direct/task-gate/backlink callers and writes. |
| `core-machine-call-gate-privilege-entry-smoke` | Same-/outer-CPL CALL-gate cache/stack and failure ordering. |
| `core-machine-iret-s51-smoke`, `core-machine-iret-outer-s52-smoke`, `core-machine-vm86-delivery-smoke` | Same-CPL, outer-CPL, and VM86 frame/cache publication. |

Task-gate policy, new task-switch semantics, VME/PVI, generalized IRET
breadth, paging/TLB, public descriptor/TSS interfaces, and device IRQ policy
remain explicit Queue transfers.  They are not hidden by this no-defect audit.
