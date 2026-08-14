# T359 S6: 80286/80386 Privileged-Form Timing Ledger

## Authority and publication boundary

This ledger closes the S1 receivers that are system-state forms, not the
architecture's exception, task, bus, or device timing.  The primary sources
are Intel's *80286 and 80287 Programmer's Reference Manual*, Appendix B, and
the original [Intel 80386 Programmer's Reference Manual](https://www.read.seas.harvard.edu/~kohler/class/aosref/i386.pdf), section 17.2.2.3.
The latter states its rows under the processor's prefetched, no-wait and
no-HOLD execution model.  The [80286 manual archive](https://www.bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf)
is retained only as provenance for Appendix-B reconciliation; it is not a
runtime dependency or imported source.

`core_machine_80386_privileged_source_instruction_cost()` is private to
`machine.c` and runs after successful refresh at
`core_machine_instruction_cost()`.  It reads only the completed decoder
capture and old CPU mode/privilege state.  It neither changes decoder,
handler, table, selector, CR/DR/TR, task, exception, or bus state.  Rejected
LOCK/prefix forms, memory/selector faults, every exception/IRQ/NMI delivery,
task switch, and unsuccessful validation publish no successful instruction
time through this owner.

## Allocated 80386 successful rows

| Receiver | PRM row | Classifier selection |
| --- | --- | --- |
| `63 /r ARPL r/m16,r16` | protected `20/21` register/memory | Protected non-VM86 success only; ModRM selects register or memory. |
| `0F 00 /0 SLDT` | protected `2/2` | Protected non-VM86 store; register and memory both two clocks. |
| `0F 00 /1 STR`, `/3 LTR` | protected `23/27` register/memory | Protected non-VM86 success; no task switch is attributed to `LTR`. |
| `0F 00 /2 LLDT` | protected `20` | Protected non-VM86 success. |
| `0F 00 /4 VERR`, `/5 VERW` | protected `10/11`, `15/16` register/memory | Protected non-VM86 selector query; selector failure is a successful ZF outcome, while memory fault remains zero publication. |
| `0F 01 /0 SGDT`, `/1 SIDT` | `9` | Memory-only successful pseudo-descriptor store in real, protected, or ordinary VM86 execution. |
| `0F 01 /2 LGDT`, `/3 LIDT` | `11` | Memory-only real or protected CPL0 success; VM86/CPL failure remains zero publication. |
| `0F 01 /4 SMSW` | real `2/3`, protected `2/2` register/memory | Width remains the instruction's 16-bit MSW form; source mode and ModRM select the exact row. |
| `0F 01 /6 LMSW` | `10/13` register/memory | Real or protected CPL0 success; no mode-transition, following-fetch, or delivery cycle is added. |
| `0F 02 LAR` | protected `15/16` register/memory | Protected non-VM86 success, including the ZF=0 selector result. |
| `0F 06 CLTS` | `5` | Real or protected CPL0 successful form. |
| `0F 20/22` CR moves | `6`; `10/4/5` for writes to CR0/CR2/CR3 | Register-direct, real or protected CPL0 success only; operand-size prefix does not alter the architecturally fixed 32-bit payload. |
| `0F 21/23` DR moves | DR0--DR3 `22`, DR6/DR7 `14/16` | Register-direct, real or protected CPL0 success only. |
| `0F 24/26` TR6/TR7 moves | `12` | Register-direct, real or protected CPL0 success only. |
| `0F A0/A8 PUSH FS/GS` | `2` | Successful stack form. |
| `0F A1/A9 POP FS/GS` | real `7`, protected `21` | Successful non-VM86 stack/selector publication only. |
| `0F B2 LSS` | real `7`, protected `22` | Successful full-pointer load, with the real/protected descriptor distinction retained. |
| `0F B4/B5 LFS/LGS` | real `7`, protected `25` | Successful full-pointer load, with the real/protected descriptor distinction retained. |

The owner smoke executes representative real-mode rows from each fixed source
shape (`CLTS`, CR/TR moves, table store, MSW store/load, and FS stack push),
checks the provider's published elapsed time, and proves a rejected LOCK form
does not retire or publish a tick.  The retained system-owner smokes named by
the form inventory continue to prove protected selector, table, control,
debug, full-pointer, task, and VM86 semantics; this ledger only gives their
successful retirement one numerical owner.

## Reconciled transfers

| Receiver or condition | Disposition and receiver |
| --- | --- |
| 80286 `63`, `0F 00 /0`--`/5`, `0F 01 /0`--`/6`, `0F 02/03/06` | **T360 Four-profile Intel timing source reconciliation.** Appendix B is authoritative, but its byte-following and protected-state presentation has not been normalized into the post-refresh classifier. No 80386 number is back-propagated to the 80286 profile. |
| `0F 03 LSL` | **T360.** The PRM has distinct byte/page-granular rows (`20/21` and `25/26`); choosing one requires a nonpublishing descriptor-granularity capture not present at the retirement publisher. |
| 80386 system forms with a source prefix/mode combination not explicitly selected above | **T360.** The manual's form row is not permission to infer a lexical-prefix, paging, selector, or VM86 outcome. |
| Protected CPL violation, VM86 rejection, selector/table/segment/page fault, rejected ModRM or LOCK form | **Cycle-exact selected-profile receiver.** These are not successful instruction retirement and must not receive a synthetic source clock. |
| Far call/jump task switch, task gate, nested task return, generic exception/IRQ/NMI entry, cross-privilege or VM86 frame delivery | **Cycle-exact selected-profile receiver.** The applicable instruction pages combine the operation with an unmodelled transition/delivery path. |
| Memory/I/O wait, HOLD/DMA ownership, prefetch/cache, alignment, pin phases and device service | Later PC/AT bus/service/cycle-exact Queue candidates. |

## Whole-receiver sweep

The S1 inventory's `63`, `0F 00 /0`--`/5`, `0F 01 /0`--`/6`,
`0F 02/03/06`, `0F 20`--`26`, `0F A0/A1/A8/A9`, and
`0F B2/B4/B5` entries were cross-checked against the primary/secondary table
installations in `cpu_instructions.c`.  Reserved `0F 00 /6,/7`, `0F 01 /5,/7`,
and invalid special-register selections remain decoder rejection paths.  The
ledger does not reclassify S5's non-privileged bit, double-shift, IMUL,
bit-scan, MOVX, or near-Jcc rows.

The static inventory verifier binds this ledger to the private classifier,
the S6 smoke marker, the existing sole publisher ordering, and the transfer
anchors above.  It prevents a later code edit from silently moving a system
row into a handler-local clock or treating a delivery outcome as retirement.
