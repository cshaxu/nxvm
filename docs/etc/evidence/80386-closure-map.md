# Intel 80386DX Closure Map

## Purpose And Authority Boundary

This supporting crosswalk prevents an accumulation of opcode smokes from being
mistaken for a complete 80386DX architecture claim. It records the two views
required by [the M5 roadmap](../../design/ROADMAP.md) and the ordered
[Queue](../../states/QUEUE.md): horizontal instruction-form evidence and vertical
architecture-state closure. It does not allocate tasks, define a current
baseline, or supersede those principal authorities.

`Complete` means the named bounded state transition has its required focused
evidence and no unclassified in-scope form. `Planned` means the Queue owns the
next admission. `Deferred` means the entry has the named debt boundary rather
than a completion claim.

## Architecture-State Closures

| Closure | Required composition boundary | Current disposition | Queue or debt destination |
| --- | --- | --- | --- |
| Real and ordinary execution | Profile gates, operand/address attributes, ordinary faults, and deterministic instruction/IRQ ordering compose in real mode. | Complete for the Intel 80386 ordinary application-form matrix reconciled by T322 S1; T316's bounded owner smokes collectively cover the named data, arithmetic, FLAGS, string, and I/O routes; T328 closes the retained 8086/80186/80286 `LOCK`-prefix profile boundary; T340 closes the residual 80386DX width, prefix, FS/GS, and non-privileged `0F` form ledger. | [T322 S1 evidence](t322-s1-ordinary-form-reconciliation.md), [T328 legacy LOCK evidence](../../history/M5-T328-legacy-lock-legality.md), and [T340 closure audit](t340-s4-80386dx-form-closure-audit.md). |
| Protected segmentation and privilege | Descriptor validation, selector/cache publication, gates, and CPL transitions compose without bypassing memory or fault atomicity. | Complete for the Queue's non-task, non-VM86 protection/privilege boundary: T301/T303/T306/T307 retain selector, same-CPL transfer, 32-bit entry/return, and call-gate evidence; T323 S1--S7 closes the complementary protected 16-bit direct-transfer, gate, external-entry, outer-return, and parameterized call-gate matrix. | [T323 protection/privilege closure audit](t323-protection-privilege-closure-audit.md) and T341's system-state closure; VME/PVI and unadmitted VM86 family breadth remain external. |
| VM86-to-protected delivery | VM86 `#GP`, `#UD`, `#NM`, and IRQ delivery cross an IDT gate onto a TSS-selected CPL0 stack with an Intel-correct frame and bounded return. | Complete: T320 proves IDT/TSS entry, full frame/failure atomicity, and 32-bit CPL0 `IRET` return; T341 S3 composes it with paging/task callers. VME/PVI and VM86 family breadth remain separate. | T320 history and [T341 S3 composition](t341-s3-vm86-task-paging-composition.md); VME/PVI TODO. |
| Exception, interrupt, and return | Fault/trap/IRQ producer ordering, gate effects, restart points, and IRET compose across the T321-admitted real/protected/VM86 paths. | Complete for the bounded active exception routes: T321 S2--S5 reconcile `#DE/#PF/#MF`, IRQ/NMI priority, software/return composition, and VM86 table-load delivery; T326 S1 closes protected/VM86 `#UD` vector-6 restart/no-error-code delivery; T341 S4 closes ordinary DR6/DR7/vector-1 composition. | [T321 S6 evidence](t321-s6-processor-control-closure.md), [T326 S1 evidence](t326-s1-protected-ud-delivery.md), and [T341 S4 graph](t341-s4-ordinary-debug-vector1.md); VME/PVI, reset, and broader trap policy remain external. |
| Processor control and table state | Bounded CR0/MSW/CLTS, descriptor-table loads/stores, and CPU-side coprocessor interface compose with their privilege/fault consumers. | Complete for T321's assigned control composition: T316 S61--S65, T318/T319, and T321 S5 have exact bounded form evidence and transfers; T341 S2 reconciles the remaining CR/DR/TR/LDT/table state owners. | [T321 S6 evidence](t321-s6-processor-control-closure.md) and [T341 S2 reconciliation](t341-s2-control-debug-table-reconciliation.md); x87 execution remains external. |
| Paging and translation | Non-PAE translation, protection, `#PF`, invalidation, and diagnostics compose with the exception and privilege paths. | Complete for the CPU-native 80386 page-walk boundary: T325 S1--S3 reconcile CR0/CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no persistent cache, and pre-486 `INVLPG #UD`; T326 S1 delivers the latter through vector 6 in protected mode; T341 S3 composes task/VM86 callers. | [T325 S3 closure audit](t325-s3-paging-closure-audit.md), [T326 S1 evidence](t326-s1-protected-ud-delivery.md), and [T341 S3 composition](t341-s3-vm86-task-paging-composition.md); a persistent TLB/TR6/TR7 model remains external. |
| Task, LDT, debug, and VM86 breadth | Task/LDT state, remaining VM86 instructions, debug/test-register behavior, and their transfer/fault boundaries compose. | Complete for the bounded Intel 80286/80386 protected task-transition state machine: T329 S1--S7 cover 16/32-bit direct far-JMP/CALL images, GDT/IDT task-gate entry, backlink/NT, bounded nested IRET return, incoming non-null LDT task images, source-CR3 preflight/incoming-CR3 commit, and the TSS post-switch debug trap; T341 S4 closes ordinary debug-register/breakpoint behavior. | [T329 S1 direct-TSS evidence](t329-s1-tss16-direct-jump.md), [S2 evidence](t329-s2-tss32-direct-jump.md), [S4 entry evidence](t329-s4-task-gate-call-entry.md), [S5 evidence](t329-s5-task-return-idt-task-gate.md), [S6 LDT evidence](t329-s6-task-ldt-images.md), [S7 paging/debug evidence](t329-s7-task-paging-debug.md), [the state-machine record](t329-task-switch-state-machine.md), and [T341 S4 graph](t341-s4-ordinary-debug-vector1.md). VM86 family breadth, VME/PVI, and persistent test-register/cache behavior remain external. |
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
| VME/PVI | Deferred post-80386 extension. | Explicit control/virtual-interrupt contract and VM86 delivery/privilege matrix. |
| 80287/80387 execution | External-coprocessor execution boundary. | Corpus-driven instruction, formats, state, exception, and provider-contract matrix. |
| Windows compatibility | Product/system validation, not CPU completeness. | Owned Windows readiness map with CPU, device, timing, profile, media, and installation/runtime evidence. |
