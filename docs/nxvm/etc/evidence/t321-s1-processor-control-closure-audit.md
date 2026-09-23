# M5 T321 S1: 80386DX Exception And Processor-Control Closure Audit

## Authority And Method

This is the explicit design prerequisite of the active T321 implementation
task. Intel's *80386 Programmer's Reference Manual*, especially its interrupt
and exception, protected-mode, virtual-8086, and system-instruction chapters,
is the form authority. The audit is read-only: it imports neither source nor
runtime assets. It cross-checks that authority against the one core executor,
the retained T304/T308/T318/T319/T320 evidence, and the registered focused
tests. It does not claim the later 80386 protection, paging, task/debug, or
external-coprocessor-execution packages.

The source sweep used the exception masks and producers in
`src/core/machine/cpu_instructions.h` and `cpu_instructions.c`, followed the
`ExecFinal` dispatch to `_e_except_n`, `_e_intr_n`, and the real/protected
interrupt and IRET planners, and searched focused machine smokes for first and
delivered fault diagnostics. It also re-read the retained descriptor-system,
exception-delivery, table-register, VM86-delivery, and paging-boundary records.

## Current Crosswalk

| Architectural row | Current route and evidence | Disposition and destination |
| --- | --- | --- |
| VM86 `#GP`, `#UD`, `#NM`, and IRQ0 entry; 32-bit CPL0 IRET return | T320 uses the existing 32-bit protected outer-gate/TSS route, `ExecFinal` vector selection, and the dedicated VM86 delivery/IRET owner smokes. | Complete bounded foundation. Update closure map now; VME/PVI, VM86-origin IRET, task gates, and instruction-family breadth are later packages. |
| Contributory protected `#GP`, `#NP`, `#SS`, and `#TS`; error-code frame and bounded double-fault conversion | `ExecFinal` maps exact masks to vectors 13, 11, 12, and 10; T308 retained evidence proves same/outer delivery and a bounded `#DF` attempt. | Retained, but only those producer classes are closed. T321 S2 must consolidate the common delivery matrix and validate it against remaining source producers. |
| `#BR` and `#NM` | `ExecFinal` maps exact masks to vectors 5 and 7. T318/T320/T316 S54/S65 owner evidence covers bounded producer paths, including real-mode vector delivery. | Partial architecture row: producer-specific evidence exists; T321 S2 must place it in the common delivery matrix and cover protected/real mode disposition consistently. |
| `#DE`, `#PF`, and `#MF` producers | `_SetExcept_DE`, `_SetExcept_PF`, and `_SetExcept_MF` have active production producers, while `ExecFinal` has no exact vector selection for those masks. Existing paging and x87-interface tests intentionally observe terminal diagnostics. | Missing shared delivery closure. T321 S2 is the highest-ROI prerequisite: audit and implement only vector 0/14/16 delivery and frame/error-code rules proven necessary, with a complete producer sweep. Paging behavior itself remains later. |
| Debug and breakpoint/overflow traps (`#DB`, `#BP`, `#OF`) and NMI | The exception constants exist, but no `_SetExcept_DB/BP/OF` producer is present. Existing TF test is a bounded debug route; NMI is not a current architectural delivery owner. | Transfer to the task/LDT/VM86/debug/test-register closure, except any shared dispatcher precondition exposed by T321 S2. Do not invent trap/NMI policy here. |
| Hardware maskable IRQ ordering and gate effects | PIC delivery uses `_e_intr_n`; ordinary T316 and T320 smokes prove local no-shadow/restart cases. | Partial. T321 S3 must unify real/protected/VM86 IRQ priority, IF/TF effects, error-frame exclusion, and NMI separation without a PIC redesign. |
| Software interrupts, INTO, INT3, and protected IRET | T316 S50/S51/S52 and retained T305--T308 cover bounded software-entry and same/outer IRET forms. | Partial. T321 S4 must audit and close only the remaining error/trap/return composition required by the common matrix; VM86 return remains T320-complete. |
| LGDT/LIDT `0F 01 /2,/3` VM86 privilege rejection | T319 closed non-VM86 forms and deliberately transferred VM86 `#GP(0)` before source read. T320 now supplies the delivery foundation. | Missing, narrow consumer. T321 S5 re-admits this exact VM86 proof only; it does not reopen table-load breadth. |
| CR0/CR2/CR3, CLTS, SMSW/LMSW, and table state | T304, T318, T319, and T316 S62/S63 retain bounded instruction evidence. | Partial composition. T321 S6 audits the remaining control-state privilege/fault/delivery intersections; paging policy and debug/test registers remain transferred. |
| ESC/WAIT CPU interface | T316 S65 closed the CPU-side `#NM`/`#MF` interface without an x87 numerical unit. | Complete external-coprocessor boundary. T321 consumes only its `#MF` delivery prerequisite; it must not implement 80287/80387 execution. |

## Ordered T321 Subtasks

1. **S2 - Exception-vector and error-frame closure.** Audit every exact
   current producer mask and implement/prove missing `#DE`, `#PF`, and `#MF`
   vector delivery, including correct error-code versus no-error-code frames,
   restart/trap semantics, real/protected classification, and failed-delivery
   containment. Stop before paging policy, x87 execution, debug/trap policy,
   task switching, or reset/shutdown policy.
2. **S3 - Hardware interrupt and gate-composition closure.** Prove
   maskable-IRQ and NMI classification/priority boundaries across real,
   protected, and already-complete VM86 entry, including IF/TF/gate effects and
   frame ordering. No PIC or NMI-device redesign.
3. **S4 - Trap, software-interrupt, and return composition.** Reconcile
   `INT`, `INT3`, `INTO`, IRET, and the shared exception/trap entry outcomes
   not already closed by S50--S52/T320. It consumes S2/S3; it does not reopen
   their individual ordinary opcode matrices.
4. **S5 - VM86 LGDT/LIDT privilege boundary.** Re-admit only the transferred
   `#GP(0)` before-source-read proof using T320's valid gate/TSS facility.
5. **S6 - Processor-control composition and task closure.** Reconcile the
   admitted CR/table/coprocessor CPU-interface forms with their mode,
   privilege, fault-delivery, and consumer-state edges. Transfer paging,
   task/LDT/debug/test, VME/PVI, and x87 execution explicitly; build T321's
   developer artifact and perform the task closure audit.

This order keeps shared exception delivery ahead of consumers that need it,
and keeps paging and protection-transfer work in their already ordered Queue
packages.

## Similar-Issue Sweep Disposition

The sweep finds active source producers for `#DE`, `#BR`, `#UD`, `#NM`, `#TS`,
`#NP`, `#SS`, `#GP`, `#PF`, and `#MF`. The dispatcher has exact delivery
selection for only `#GP`, VM86 `#UD`, `#NM`, `#BR`, `#NP`, 80386 `#SS`, and
80386 `#TS`; it retains bounded contributory `#DF` conversion. S2 owns the
unclassified active producer masks. No production source changes occur in S1.

`#DB`, `#BP`, and `#OF` currently have no `_SetExcept_*` producer, and NMI is
not a producer in this executor path. Their architecture closure is explicitly
transferred to the later task/LDT/VM86/debug/test-register Queue package rather
than treated as complete by absence.
