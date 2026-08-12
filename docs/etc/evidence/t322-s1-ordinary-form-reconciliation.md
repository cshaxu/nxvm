# M5 T322 S1: Ordinary Application-Form Reconciliation

## Audit Result

The P0 ordinary-execution and flag-completeness candidate is withdrawn.  The
T316 S66 parent-row wording retained broad residual labels, but the accepted
S23--S65 evidence added after the original horizontal grouping already covers
every Intel 80386 ordinary application-instruction form named by that transfer.
The remaining items are not ordinary opcode gaps: they are explicit privilege,
translation, task/debug/VM86, legacy-LOCK, or external-coprocessor boundaries.
Creating a new implementation slice would duplicate owner smokes and violate
the task's no-fabricated-work requirement.

## Form-To-Evidence Crosswalk

| Intel 80386 ordinary form set | Route family reviewed | Accepted focused evidence | Disposition |
| --- | --- | --- | --- |
| Data movement and exchange | `MOV` moffs, GPR, Sreg, and far-load routes; `XCHG`; `LEA`; `CBW/CWD`; primary dispatch and operand/address decoders. | T316 S23--S32 and S41 owner smokes. | Complete for the named ordinary forms; control/debug MOV forms are not application data movement. |
| Stack and frame forms | General, immediate, legacy-Sreg, FS/GS, PUSHA/POPA, PUSHF/POPF, ENTER/LEAVE, and r/m stack routes. | T316 S23, S42--S47 owner smokes. | Complete for ordinary instruction forms. Stack switching, task stacks, and privilege-owned selector behavior transfer to protection/task packages. |
| Primary arithmetic and conversion | `TEST`, `ADD` through `CMP`, Groups `80/81/83`, unary Groups, decimal/ASCII adjust, `XLAT`, Group-2 shifts/rotates, one-operand multiply/divide, `BOUND`, `ARPL`, and immediate IMUL. | T316 S2--S19 and S53--S56 owner smokes; primary dispatch and arithmetic helper sweep. | Complete for the named Intel 80386 primary forms. Undefined FLAGS are deliberately not asserted; legacy LOCK remains separate. |
| Local FLAGS and halt behavior | `LAHF/SAHF`, CMC/CLC/STC/CLD/STD, PUSHF/POPF, CLI/STI, and HLT. | T316 S39, S40, S47--S49 owner smokes. | Complete for instruction-local behavior. Event/return composition is owned by T321; VME/PVI and task behavior remain later system-state work. |
| String and port-string forms | `MOVS`, `STOS`, `LODS`, `SCAS`, `CMPS`, `INS`, and `OUTS`, including declared REP, DF, segment, operand/address, limit, and IRQ cases. | T316 S33--S38 owner smokes; S64 shared-prefix semantic-class proof. | Complete for the named ordinary string opcodes. Generic I/O permission policy and port-device behavior are protection/device work. |
| Ordinary I/O and transfer forms | Immediate/DX `IN`/`OUT`; near/far ordinary transfer and the admitted software INT/IRET forms. | T316 S50--S52 and S55 owner smokes; retained T303 control-transfer evidence; T321 S4 composition smoke. | Complete for their declared ordinary forms. Gates, task returns, VME/PVI, and privilege transitions are transferred. |

## Actual-Code Review

The audit traced each family through the only executor,
`src/core/machine/cpu_instructions.c`, its primary table, `INS_0F` secondary
table where applicable, profile metadata, operand/address decoders, and the
named current-gate owner smoke.  The smoke entry points were read to confirm
that their form, profile, prefix, publication/fault, and IRQ assertions are
actually invoked rather than merely compiled.  The current-gate registration
and target-local strict compilation inventory contain every named owner smoke.

No second CPU executor, unregistered replacement smoke, or unowned primary
ordinary form was found.  The Intel 80386 PRM instruction-form authority was
used only to compare application-form encodings and defined FLAGS semantics;
no external source or implementation was imported.

## Transfers That Remain Real

| Boundary | Sole next owner |
| --- | --- |
| Descriptor, CPL, gates, selector/stack switching, and I/O privilege policy | **80386DX protection and privilege-transfer closure** Queue package. |
| Translation, CR0/CR2/CR3 consumer policy, invalidation, and page diagnostics | **80386DX paging and translation closure** Queue package. |
| Tasks, LDT breadth, remaining VM86 forms, debug/breakpoint/test registers, task gates and returns | **80386DX task, local-descriptor, virtual-8086, and debug/test-register closure** Queue package. |
| 80186/80286 LOCK legality | Existing legacy LOCK Queue/TODO boundary. |
| VME/PVI and numerical x87 execution | Existing VME/PVI and broaden-present-x87 TODO boundaries. |

The Queue therefore begins with the protection and privilege-transfer closure.
T322 closes as a reconciliation task only; it makes no product, ABI, runtime,
or artifact change and does not claim the overall 80386 program complete.
