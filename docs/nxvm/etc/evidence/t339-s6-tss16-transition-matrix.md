# M5 T339 S6: 80286 TSS16 Task-State Transition Matrix

## Mechanism Inventory

The S6 sweep covers direct far TSS transfers, GDT and IDT task gates, nested
task return, and the shared TSS16 transition owner:

```text
rg -n "_ser_task_(transition|switch|return)_tss|_s_load_(tr|ldtr)" \
    src/core/machine/cpu_instructions.c
rg -n "TASK_SWITCH_CASE|task_switch_expect_(switch|fault|task_gate|nested|idt)" \
    tests/machine/core_machine_task_switch_smoke.c
```

`_ser_task_transition_tss` retains the genuine 16-bit TSS layout. It validates
the current and target TSS descriptors, limits, source and target spans, LDT,
and incoming code/stack/data caches before it publishes the target task state.
`_ser_task_switch_tss` owns direct and task-gate entry; `_ser_task_return_tss`
owns the backlink return path. `_s_load_tr` and `_s_load_ldtr` remain shared
selector-cache materializers and were audited, not changed.

## 80286 Evidence

| Origin or boundary | Owner smoke result |
| --- | --- |
| Direct far TSS `JMP` and `CALL` | TSS16 outgoing image, target cache/TR materialization, busy state, and `CALL` backlink/NT are exercised on 80286. |
| GDT and IDT task gates | Successful task-gate entry, IDT task-gate entry, DPL rejection, and not-present task-gate producer disposition are exercised on 80286. |
| Nested task `IRET` | Backlink selection, NT clearing, old/new busy descriptor transition, and restored source task state are exercised on 80286. |
| LDT/TI | A valid incoming LDT image materializes the incoming selectors; a non-present incoming LDT produces `#NP(0040)` before target TR/cache publication. |
| Target TSS validation | Invalid selector, not-present TSS, busy TSS, and short TSS produce respectively `#GP(0040)`, `#NP(0030)`, `#GP(0030)`, and `#TS(0030)` on 80286. |
| Stack/code validation | The retained 80286 stack-limit case produces `#SS`; current TR remains the old TSS and the new task cache is not published. |
| Post-commit order | A pending PIC IRQ after a successful 80286 switch enters after the transition, with the target TR/image and PIC ISR/IRR state proved. |

The owner smoke's terminal no-IDT fault vectors prove the producer, restart
boundary, and unchanged active TR. They intentionally do not claim that an
architectural task switch is a whole-memory transaction: a task switch may
save the outgoing TSS before a later incoming-state validation fault. The
target cache/TR, busy/backlink commit, and delivery state are therefore
classified at their actual mechanism boundaries rather than flattened into an
incorrect zero-write rule.

## Similar-Issue Disposition

No implementation defect was reproduced. The new 80286 rows close historic
gaps where busy/short target TSS, task-gate not-present, and incoming LDT
not-present had only a 80386 or implicit disposition. No local transition,
descriptor, cache, or delivery code changed.

## Exact Transfers

Accepted T328 retains pre-386 `LOCK` legality. TSS32 images, CR3/paging,
debug-trap, 32-bit operand/address forms, and VM86 task interactions transfer
to T341 or T342 as assigned by the T339 S1 ledger. Generic exception-delivery
redesign is not claimed by S6.
