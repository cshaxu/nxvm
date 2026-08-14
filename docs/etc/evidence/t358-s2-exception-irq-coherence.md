# M5 T358 S2: Exception And IRQ Entry Coherence

## Authority And Method

Intel 80286 and 80386 protected-mode exception tables define `#TS` as vector
10 and `#SS` as vector 12, including an error code.  The project-owned 80386
exception-entry records at `t308-exception-delivery-admission.md`,
`M5-T320-vm86-delivery.md`, and `t320-s1-vm86-delivery-matrix.md` remain the
retained frame and VM86 evidence.  This S re-ran the source-to-consumer sweep
with:

```text
rg -n "_SetExcept_[A-Z]+\\(" src/core/machine/cpu_instructions.c
rg -n "_e_(except|intr)_n\\(" src/core/machine/cpu_instructions.c
rg -n "core_machine_pic_(peek|get)_interrupt|flagNMI|_e_intr_n" src/core/machine/cpu_instructions.c
```

The sweep is limited to CPU entry publication.  Device IRQ production, NMI
policy, VME/PVI, task-gate redesign, inverse IRET, and paging algorithms stay
outside this S.

## One Entry Ledger

| Boundary | Owner and commit rule | Evidence and disposition |
| --- | --- | --- |
| Instruction producers | `_SetExcept_DE/DB/GP/UD/NM/BR/NP/SS/TS/PF/MF` record one mask/code in `instruction_state`; `ExecFinal` restores the instruction-start CPU before fault delivery. | All producer masks are classified in `ExecFinal`. `#SS/#TS` had an accidental `>=80386` classifier guard despite being 80286 protected-mode exceptions. It is corrected to `>=80286`. |
| Software and synchronous entry | `_e_except_n` selects `_ser_int_real` or `_ser_int_protected`; the protected serializer chooses actual gate type, preserving real 16-bit, protected 16-bit, 32-bit same-CPL, and 32-bit outer/VM86 layouts. | Retained T305/T308/T320 probes cover real, protected same/outer, VM86, error-code, failed-preflight, and double-fault boundaries. No layout was merged or changed. |
| 80286 error exceptions | `_ser_int_protected_16` preflights target code and all frame writes, then pushes error code, saved IP, CS, FLAGS and publishes CS/IP/IF/TF. | `core-machine-80286-protected-mode-smoke` now installs vectors 10 and 12. It proves `MOV SS, nonpresent` delivers `#SS(0010)` with frame `0010,0013,0000,0002`, and NT `IRET` with a short loaded 16-bit TSS delivers `#TS(0000)` with frame `0000,0006,0008,4002`; both reach their HLT handlers. |
| Real fault delivery | `_e_final_deliver_real_exception` owns restore, IVT entry, delivered diagnostic, and rollback when serialization fails. | Existing real exception and FPU-interface tests cover `#DE/#DB/#PF/#MF/#UD/#BR/#NM/#GP`; `#SS/#TS` are protected-only here and intentionally have no real-IVT claim. |
| Hardware events | `ExecInt` calls `_e_intr_n` for NMI and only peeks then acknowledges the PIC after entry leaves no exception. | Retained interrupt-entry, CLI/STI, VM86, and I/O-owner tests prove successful IRQ acknowledgement/frame order and failed-entry non-acknowledgement. The S1 I/O audit separately covers ordinary/string callers. No PIC policy change is made. |
| Failed protected delivery | `ExecFinal` restores `fault_cpu`; 80386 contributor-pair escalation retains the bounded `#DF` path. | Retained T308/T320 matrices prove preflight/nonpublication and diagnostic boundaries. This S does not expand recursive-delivery or reset policy. |

## Repair And Similar-Issue Sweep

The sole implementation defect was the profile threshold on the two exact
`ExecFinal` branches.  A source sweep found no other profile threshold on the
same delivery owner.  The repair changes neither `_e_except_n`, any serializer,
PIC acknowledgement, error-code construction, nor 16-/32-bit frame layout.

The focused marker is `M5:T358:S2:EXCEPTION-IRQ:OK`, emitted by the retained
current-gate 80286 protected-mode owner after its new vector-10/vector-12
delivery cases.  The full current gate remains the regression proof for all
retained producers and IRQ callers.

## Explicit Transfers

Task-gate/task-switch redesign, VME/PVI, inverse IRET semantics, NMI/PIC
policy, device IRQ production, paging/TLB behavior, and physical timing are
not changed or newly claimed.  They remain ordered cross-mode or L3 Queue
work under their existing proposals.
