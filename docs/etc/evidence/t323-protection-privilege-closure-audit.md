# T323: 80386DX Protection And Privilege-Transfer Closure Audit

## Decision

T323 closes the Queue's **80386DX protection and privilege-transfer**
boundary. This is a state-composition closure, not a claim that all 80386
system-state architecture, paging, VM86 instruction breadth, x87 execution,
or legacy profile prefix policy is complete.

The audit re-read the current serializers and the focused owner evidence for
selector/cache preparation, direct far transfer, protected gate delivery,
call-gate entry, outer return, and TSS stack selection. The shared routines
remain bounded by their established consumers: no task, LDT, VM86, paging,
generic exception-finalizer, or PIC-controller policy was changed by T323.

## Reconciliation Matrix

| Required protection/privilege condition | Code and focused evidence | Disposition |
| --- | --- | --- |
| Protected selector/cache and data access | `_ksa_prepare_*_sreg`, checked memory access, and T301/T323 S2 cover DS/ES/SS selector, rights, null, ordinary and expand-down limit publication. | Complete. |
| Direct protected code transfer | `_ser_call_far_*` / `_ser_jmp_far_*`; T303 and T323 S1 cover immediate and ModRM `CALL`/`JMP`, selector/type/DPL/present/limit and stack preflight. | Complete. |
| Same-CPL protected IDT entry | `_ser_int_protected_16` and retained 32-bit path; T305/T323 S3 prove software interrupt/trap types, DPL, frame and IF/TF rules. | Complete. |
| External protected entry | `ExecInt` and protected gate serializers; T307/T323 S4--S5 prove 16/32 same- and outer-CPL IRQ/NMI entry, hardware DPL bypass, TSS16/TSS32 stack selection, frame layout, source ownership, and rejection boundaries. | Complete. |
| Outer protected returns | `_ser_ret_far_outer` / `_ser_iret_protected_outer`; T306 proves 16/32 `RETF` and `IRET` frame, new SS, `imm16`, EFLAGS, descriptor and source atomicity. T323 S6 composes the 16-bit IRET branch with the current TSS/IRQ route. | Complete. |
| 32-bit call-gate entry | `_ser_call_far_call_gate_32`; T307 proves CPL3-to-CPL0 frame/parameter entry and target-stack rejection boundaries. | Complete. |
| 16-bit call-gate entry | `_ser_call_far_call_gate`; T323 S7 proves 80286 TSS16 and 80386 TSS16/TSS32 outer calls, same-CPL DPL behavior, two word parameters, target frame, source/target preflight, and IRQ ordering. The local serializer now preflights/copies the declared words. | Complete. |
| Task-gate and task-switch state | Task gates, task returns, busy/back-link state, and broad TSS task semantics. | Transferred to the Queue's **80386DX task, local-descriptor, virtual-8086, and debug/test-register closure**. |
| VM86 instruction breadth and VME/PVI | T320 closes only VM86-to-CPL0 delivery and bounded IRET composition. Instruction breadth and VME/PVI remain excluded. | Transferred to the same task/LDT/debug/VM86 Queue package; VME/PVI remain a post-80386 deferred boundary. |
| Paging/translation and `#PF` consumer policy | Paging tables, CR3/TLB, page faults, protection and invalidation. | Transferred to the Queue's **80386DX paging and translation closure**. |
| 80186/80286 LOCK legality | Shared legacy `PREFIX_LOCK` legality, including all affected ModRM forms. | Transferred to its separate legacy LOCK Queue/TODO package. |
| x87 numerical execution | ESC/WAIT CPU interface is closed by T316 S65; arithmetic/state/provider execution is not a T323 consumer. | External-coprocessor boundary; x87 execution remains deferred. |

## Result And Prevention

The only in-scope implementation defect found by T323 was the retained
16-bit call-gate parameter-count rejection. S7 fixes it with the same bounded
source-read, target-stack-preflight, and reverse-push ordering already used by
the 32-bit serializer; its owner smoke prevents reintroducing either the
nonzero-count rejection or partial target-frame publication.

All T323-smoke targets use target-local strict GCC options. The final S7
implementation verification passed documentation governance, `git diff
--check`, exact target discovery, and the 209-test current gate. The refreshed
`vm-0-5-0323` artifact SHA-256 is
`0C901AC5E883FDB26D70B317814E580BB9000CE50899BBCA2AF273912DF1F785`.

No unclassified row remains in T323's bounded protection/privilege scope.
