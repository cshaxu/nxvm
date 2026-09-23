# T363 S6: privileged-form reconciliation

## Result

S6 finds selected successful 80386 privileged/system forms completely
source-backed by the accepted T359 S6 ledger; no runtime change is warranted.
`core_machine_80386_privileged_source_instruction_cost()` is private, runs
after successful refresh at the one instruction publisher, and reads only
decoded instruction plus prior CPU mode/privilege state.

| Form partition | Owner/disposition |
| --- | --- |
| `ARPL`; `SLDT`/`STR`/`LLDT`/`LTR`/`VERR`/`VERW`; `SGDT`/`SIDT`/`LGDT`/`LIDT`/`SMSW`/`LMSW`; `LAR`; `CLTS` | Exact 80386 source rows selected only on their successful memory/register, protected/real and CPL/VM86 conditions. |
| CR/DR/TR moves; `PUSH`/`POP FS/GS`; `LSS`/`LFS`/`LGS` | Exact selected register/memory and real/protected rows at the same publisher. |
| Rejected LOCK/prefix/ModRM, CPL/VM86 denial, selector/table/page/segment fault | No successful-retirement clock. |
| 80286 privileged rows needing normalized contextual capture; `LSL`; task/gate/cross-privilege/VM86 frame and generic delivery | Explicit T360/cycle-exact receivers; no 80386 value is back-propagated. |
| Memory/I-O waits, HOLD/DMA, prefetch/cache, pin phases and device service | Later PC/AT bus/service/cycle-exact receivers. |

## Verification

On 2026-08-14, `core-machine-t359-s6-timing-smoke` passed. It covers fixed
real-mode source shapes, table and MSW store/load, `CLTS`, CR/TR moves, FS
stack publication and rejected-LOCK zero publication. The source sweep covers
all selected system `0F` extensions and `63`, mode/privilege guards, one
publisher ordering, focused smoke and the T359 inventory verifier; it finds no
handler-local elapsed tick or second publisher. The unchanged runnable tree
passed the 246/246 current-gate suite during the preceding S4 full-gate run.
No developer artifact is created. S7 receives only task-level corpus closure
and transfer auditing.
