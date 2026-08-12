# Intel 80386DX Closure Map

## Purpose And Authority Boundary

This supporting crosswalk prevents an accumulation of opcode smokes from being
mistaken for a complete 80386DX architecture claim. It records the two views
required by [the M5 roadmap](../../design/ROADMAP.md) and the ordered
[Queue](../../QUEUE.md): horizontal instruction-form evidence and vertical
architecture-state closure. It does not allocate tasks, define a current
baseline, or supersede those principal authorities.

`Complete` means the named bounded state transition has its required focused
evidence and no unclassified in-scope form. `Planned` means the Queue owns the
next admission. `Deferred` means the entry has the named debt boundary rather
than a completion claim.

## Architecture-State Closures

| Closure | Required composition boundary | Current disposition | Queue or debt destination |
| --- | --- | --- | --- |
| Real and ordinary execution | Profile gates, operand/address attributes, ordinary faults, and deterministic instruction/IRQ ordering compose in real mode. | Complete for the Intel 80386 ordinary application-form matrix reconciled by T322 S1; T316's bounded owner smokes collectively cover the named data, arithmetic, FLAGS, string, and I/O routes. | T322 S1 evidence; legacy 80186/80286 LOCK legality remains its separate Queue/TODO boundary. |
| Protected segmentation and privilege | Descriptor validation, selector/cache publication, gates, and CPL transitions compose without bypassing memory or fault atomicity. | Complete for the Queue's non-task, non-VM86 protection/privilege boundary: T301/T303/T306/T307 retain selector, same-CPL transfer, 32-bit entry/return, and call-gate evidence; T323 S1--S7 closes the complementary protected 16-bit direct-transfer, gate, external-entry, outer-return, and parameterized call-gate matrix. | [T323 protection/privilege closure audit](t323-protection-privilege-closure-audit.md); task/LDT/debug/VM86 breadth and paging retain their named Queue packages. |
| VM86-to-protected delivery | VM86 `#GP`, `#UD`, `#NM`, and IRQ delivery cross an IDT gate onto a TSS-selected CPL0 stack with an Intel-correct frame and bounded return. | Complete: T320 proves IDT/TSS entry, full frame/failure atomicity, and 32-bit CPL0 `IRET` return; VME/PVI and VM86 family breadth remain separate. | T320 history; future VME/PVI and task/LDT/VM86 breadth packages. |
| Exception, interrupt, and return | Fault/trap/IRQ producer ordering, gate effects, restart points, and IRET compose across the T321-admitted real/protected/VM86 paths. | Complete for T321's bounded producer and event matrix: S2--S5 reconcile `#DE/#PF/#MF`, IRQ/NMI priority, software/return composition, and VM86 table-load delivery. | [T321 S6 evidence](t321-s6-processor-control-closure.md); debug/breakpoint, task, VME/PVI, reset, and broader trap policy remain later packages. |
| Processor control and table state | Bounded CR0/MSW/CLTS, descriptor-table loads/stores, and CPU-side coprocessor interface compose with their privilege/fault consumers. | Complete for T321's assigned control composition: T316 S61--S65, T318/T319, and T321 S5 have exact bounded form evidence and transfers. | [T321 S6 evidence](t321-s6-processor-control-closure.md); paging/translation owns CR0/CR2/CR3 consumer policy, task/LDT/debug owns remaining system state, and x87 execution remains external. |
| Paging and translation | Non-PAE translation, protection, `#PF`, invalidation, and diagnostics compose with the exception and privilege paths. | Complete for the CPU-native 80386 page-walk boundary: T325 S1--S3 reconcile CR0/CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no persistent cache, and pre-486 `INVLPG #UD`. | [T325 S3 closure audit](t325-s3-paging-closure-audit.md); task/VM86 paging, a persistent TLB/TR6/TR7 model, and the transferred protected-`#UD` delivery defect remain later packages. |
| Task, LDT, debug, and VM86 breadth | Task/LDT state, remaining VM86 instructions, debug/test-register behavior, and their transfer/fault boundaries compose. | Partial; selected LDT, DR, TF, and task-register slices are retained. | 80386DX task, local-descriptor, virtual-8086, and debug/test-register closure. |
| External coprocessor boundary | CPU-side ESC/WAIT and `#NM` interface is stable without claiming an x87 numerical unit. | Complete as a CPU interface boundary; x87 execution deferred. | Broaden present x87 TODO. |

## Horizontal Evidence And Transfer Rules

| Evidence family | What it proves | What it cannot close alone | Required transfer rule |
| --- | --- | --- | --- |
| T316 ordinary-execution matrix | Assigned instruction forms, prefixes, publication, controlled faults, and local PIC boundaries. | General privilege delivery, paging, VM86 stack transitions, task state, or whole-CPU completeness. | Every retained `Partial` row must name one Queue package, TODO, or the external-coprocessor boundary. |
| T318/T319 table-register slices | SGDT/SIDT and non-VM86 LGDT/LIDT form behavior, table atomicity, and bounded consumers. | General processor-control or VM86 delivery closure. | Re-admit VM86 LGDT/LIDT only after the VM86 delivery foundation. |
| Future Queue packages | A coherent architecture-state transition plus the forms needed to exercise it. | Unrelated family breadth or downstream compatibility. | Update this map and the relevant form matrix at acceptance; leave unresolved rows planned or deferred. |

## Deferred Boundaries

| Boundary | Disposition | Admission condition |
| --- | --- | --- |
| 80186/80286 LOCK legality | Deferred cross-cutting profile policy. | Intel profile-by-profile legality matrix, all affected opcode/ModRM routes, and retained 80386 evidence. |
| VME/PVI | Deferred post-80386 extension. | Explicit control/virtual-interrupt contract and VM86 delivery/privilege matrix. |
| 80287/80387 execution | External-coprocessor execution boundary. | Corpus-driven instruction, formats, state, exception, and provider-contract matrix. |
| Windows compatibility | Product/system validation, not CPU completeness. | Owned Windows readiness map with CPU, device, timing, profile, media, and installation/runtime evidence. |
