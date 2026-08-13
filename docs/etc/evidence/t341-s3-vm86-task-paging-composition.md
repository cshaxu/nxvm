# T341 S3: VM86, Task, And Paging Composition

## Composition Graph

T341 S3 verifies the state-machine edges that prior bounded packages could not
prove by themselves. It retains one page-walk transaction owner, one task
transition plan/commit owner, and the distinct Intel-required 16/32-bit TSS
and frame layouts.

| Source state and event | Validation and materialization | Commit / rollback boundary | Focused proof |
| --- | --- | --- | --- |
| VM86 `#UD`, `#GP`, `#NM`, TF, or IRQ | `_e_except_n`/`_e_intr_n` select `ExecFinal`/`ExecInt`; protected outer delivery validates an IDT 32-bit gate, busy 32-bit TR/TSS, SS0/ESP0, and the complete target stack frame before publication. | Successful delivery clears VM/IF/TF, publishes CPL0 caches and TSS stack, then records a VM86 dword frame. Failed facility preflight retains the source CPU. | T320 S1 VM86 delivery matrix. S3 adds paging-enabled VM86 `#UD` delivery: the source instruction, IDT/TSS reads, downward CPL0 frame, and handler fetch all use one CR3 mapping. |
| VM86 instruction-page absence | `_kma_prepare_physical_linear` detects the VM86 source fetch through the current CR3; `ExecFinal` preserves the page-fault CR2/code and delegates to the same VM86 outer-delivery planner. | `#PF(4)` is delivered after source fetch failure through mapped IDT/TSS/SS0/handler pages; the ten-dword frame holds error code, restart EIP, VM86 CS/EFLAGS/ESP/SS/ES/DS/FS/GS. | New S3 vector in `core-machine-vm86-delivery-smoke` deliberately leaves VM86 code page 4 absent while retaining the CPL0 handler page. It proves CR2=`0x4000`, error code 4, frame, target CPL0 state, and handler HLT. |
| CPL0 32-bit `IRET` to VM86 | `_ser_iret_protected_to_vm86` preflights all nine dwords, builds private real-style VM86 caches, and does not publish partial state on a short frame. | It commits VM86 flags, all segment caches, EIP and ESP together; subsequent VM86 fetch is a normal page-walk consumer. | T320 S2 retained frame/atomicity proof. S3 adds a mapped page-0 IRET, page-8 source stack, and page-2 VM86 target fetch under CR3=`0xa000`; after IRET plus NOP, VM86 EIP is `0x11`, ESP is `0x1234`, and CR3 is retained. |
| 16/32-bit task entry, return, LDT and CR3 | `_ser_task_transition_tss` selects source and target TSS formats independently; `_ser_task_transition_tss_plan` validates TSS spans, descriptors, selectors, LDTR, target stack, and incoming CR3 while source CR3 is still active. | The plan writes outgoing state, commits busy/backlink/LDTR/TR and incoming CR3/CPU/cache state in architectural order; target T-bit enters vector 1 only after commit. | T329 S1--S7 task state-machine record and `core-machine-task-switch-smoke` prove direct/task-gate/nested paths, all TSS format pairs, source-page failure rollback, incoming CR3 fetch, LDT, and post-commit debug trap. |
| Page translation and fault consumption | `_kma_prepare_physical_linear` materializes candidate PDE/PTE accesses; `_kma_commit_physical_linear` publishes A/D only after a successful access. | Cross-page preflight prevents partial A/D/data/stack/fetch publication; `ExecFinal` delivers restartable `#PF` with CR2 and an error-code frame. | T325 S1--S3 and `core-machine-80386-paging-smoke` prove CR0/CR2/CR3, permissions, A/D, split access, page-fault delivery, and no persistent translation cache. |

## Similar-Issue Sweep

The complete execution and evidence scope was searched with:

```powershell
rg -n "VM86|VCPU_EFLAGS_VM|_ser_iret_protected_to_vm86|"
  "_ser_task_transition_tss(_plan)?|_ser_task_return_tss|CR3|"
  "_kma_prepare_physical_linear|_kma_commit_physical_linear|ExecFinal|ExecInt"
  src/core/machine tests/machine CMakeLists.txt docs/etc/evidence docs/history
```

All state-writing paths are represented in the graph. No second page-walk,
TSS transition, VM86 frame, or exception publication path was found. The
source and target layout differences are architectural: VM86 outer delivery
uses a ten-dword error-code frame where applicable, VM86 IRET consumes nine
dwords, and 286/386 TSS images remain independently selected.

## Explicit Boundaries

- VME/PVI remains a post-80386 virtual-interrupt extension in TODO.
- Persistent TLB/TR6/TR7 remains external until a cache lifetime, CR3/task
  flush, and test-register matrix is admitted.
- Ordinary DR6/DR7 breakpoint cause, RF interaction, and vector-1 delivery
  remain T341 S4; the TSS debug-trap edge is retained only as a post-commit
  ordering proof.
- VM86 instruction-family breadth is not claimed by the selected producers
  and return form.

## Verification

S3 reruns `current.core-machine-vm86-delivery-smoke`,
`current.core-machine-vm86-iret-smoke`, `current.core-machine-task-switch-smoke`,
and `current.core-machine-80386-paging-smoke`, then the full current gate.
The two modified VM86 owner targets remain existing target-local strict GCC
targets; no public API, CMake topology, production state owner, or generic
delivery helper changed.
