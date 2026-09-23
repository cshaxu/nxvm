# T363 S5: 80386 secondary and prefix/width reconciliation

## Result

S5 finds the selected non-privileged 80386 secondary-integer mechanism
completely source-backed by the accepted T359 S5 ledger; no runtime change is
warranted. `core_machine_80386_secondary_source_instruction_cost()` accepts
only successful 80386 `0F` encodings with source-backed prefixes, and remains
one classifier under the sole post-refresh publisher.

| Form partition | Owner/disposition |
| --- | --- |
| Near `Jcc`, `BT`/`BTS`/`BTR`/`BTC`, immediate bit forms, `SHLD`/`SHRD`, `MOVZX`/`MOVSX` | Exact 80386 PRM register/memory and branch-outcome rows, including the existing lexical component term for taken near branches. |
| Two-operand `IMUL`, `BSF`/`BSR`, `SETcc` | Existing dynamic IMUL, zero-scan and primary-shape owners retain their selected source rows and 106-clock maximum; S5 confirms the same publisher. |
| Legal `66`/`67`/segment and memory-modifying LOCK | Existing prefix and shared LOCK validation selects the identical row only after successful refresh. |
| Rejected LOCK, fault, 8086 `0F` POP-CS, 80186/80286 rejection | No successful-retirement clock. |
| Table/control/debug/task/selector/VM86/delivery, physical service and waits | Explicit S6/T360/physical receivers; no profile borrowing or fabricated clock. |

## Verification

On 2026-08-14, `core-machine-t359-s5-timing-smoke` passed. It covers near
branch outcomes, every fixed-row family, register/memory, immediate forms,
66/67/FS variants, legal and rejected LOCK, dynamic IMUL, scan extremes and
the 106-clock preflight boundary. The static sweep covers every selected `0F`
family, prefix validator, dynamic owner, sole publisher, focused smoke and
T359 inventory gate; it finds no handler-local elapsed tick or second
publisher. The unchanged runnable tree also passed the current-gate suite
246/246 during the preceding S4 full-gate run. No developer artifact is
created. S6 receives only privileged/table/control/debug/task timing forms.
