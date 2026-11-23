# M5 T593 S6 - Active: Shared execution optimization

S5 closed at `9d77899d1`. The [S6 delivery](../etc/evidence/t593-s6-decoded-execution.md)
includes decoded execution reuse, the disabled breakpoint fast path,
native Windows GCC Release IPO, PIT event advancement, compact CPU access records,
clock arithmetic and IRQ line aggregation.
All sixteen EXEs were rebuilt together. Each host width passed 717 unit
tests and 48 integrations.

The MyOB300 DOS replay recorded x64/x86 cold through VER of 55.376/74.776 s
and DIR to prompt of 4.592/6.677 s. See the evidence for exact budgets and scope.
The replay verified key echo and pause. Native redraw and RDP were outside
the validation.

After two held batches, the selected SMI admission, private CPU state and
memory routing changes passed paired DOS comparisons and full receiving
qualification. All sixteen EXEs were rebuilt together. Each host width
passed 717 unit tests and 48 integrations without skips. The paired x64/x86
cold times fell from 49.789/62.340 s to 44.412/58.855 s. DIR fell from
4.276/5.310 s to 4.123/5.206 s, above the one-second target. MyOB300 native
DOS input and redraw passed on a private Windows desktop. Both Machine
restart sequences returned to DOS in fresh generations on both widths.
RDP remains open. The owner approved continuing performance work on
2026-11-22 with the original DIR gate. The first page-filter prototype
is held after an overlapping-ROM regression on both widths.

The prefetch batch passed focused checks on both widths, then stopped at an
independently reproduced native Lib window-modal failure. The previous
executable hashes were restored. An isolated compiler-feedback build improved
paired MyOB300 DOS cold boot and DIR on both widths. Its DIR remained
4.209/4.855 s, above the original gate. Neither candidate is deployed.

The owner approved supplementary Release compiler feedback and continued
investigation of generic execution costs. P39 compiled and qualified
the two Core objects with feedback. Both widths passed 717 unit tests
and 48 integrations, including the previously intermittent Lib window
test. P40-P42 record all sixteen verified executables. The paired MyOB300
DIR measured 3.813 s on x64 and 6.053 s on x86, above the one-second
target.

P43-P46 deliver the generic ordered memory routes and all sixteen
verified Release executables. Each host width passed 717 unit tests
and 48 integrations. The final MyOB300 DOS replay measured DIR at 4.511 s
on x64 and 5.201 s on x86. S6 remains open. See the evidence for the
trained build inputs, measurements and artifact identities.

The owner withdrew the [standalone ROM Windows candidate](../proposals/m5-ob300-rom-windows.md).

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T593 S6 after S5. |
| Admission And Approval | On 2026-11-21 the owner approved generic Core optimization based on 86Box and Bochs. The owner also selected exact reductions in clock and PIC work based on their operation counts. On 2026-11-22 the owner confirmed continued performance work after the paired results missed the DIR target. |
| Objective | Reduce repeated work in the generic CPU and board execution paths while preserving guest fetch, timing and device event order. Use focused equal-work probes. |
| Non-goals | No product-specific path, third-party code import, public ABI change, JIT, second instruction engine or timing grade reduction. |
| Reference Baseline | `9d77899d1` and the published S5 receiver set. Local Bochs 2.6 decoded trace cache is a design reference. |
| Candidate Proposal | [Performance S sequence and decoded execution boundary](../proposals/m5-ob300-performance.md), S6. |
| Files And ABI Surface | X86 Core CPU and Machine internals, X86 NXVM peripheral and board internals, Windows GCC Release compiler feedback build inputs, and NXVM evidence. Shared source changes reach all sixteen executables across NXVM, MyNES and MyRaidenII. |
| Applicable Rules | NXVM Task Reading Set, shared Execution, Architecture, Coding and Documentation, local Core contract and NXVM source policy. |
| Verification | Use dual-width decode and timing contracts, clock/PIC regressions and short equal-work probes. Qualify retained Shared changes with all sixteen published EXEs, the full cached unit and integration suites on both widths, and the MyOB300 read-only DOS replay. |
| Expected Markers | Fewer redundant decode, clock or board operations. Preserve fetches, architecture state, execution and event order, faults, timing attribution and ticks. |
| Asset Needs | Task-owned build/t593-s6 for evidence and the existing stable Ninja trees. Standalone probes have separate 60-second limits. The cached release gates and existing read-only MyOB300 DOS fixture use the budgets recorded in the S6 evidence. |
| Reporting Requirements | Distinguish observational preview from real execution and operation counts from host elapsed time. Record viable and rejected candidates and original usability gaps. |
| Stop Conditions | Reject redundant caches, unproved invalidation or a second semantic path. Keep original performance targets. |
| Exit Criteria | Review the design and concrete experiment results, retain only independently justified changes, and document the next architectural decision. |
| Original Owner Request | Bring the useful generic 86Box and Bochs Core performance techniques into NXVM. |
| Similar-Issue Sweep | Operands, mode changes, prefetch ownership, memory writes, debug traps, crossed pages, asynchronous entries and REP. Check exact clock phase and overlapping IRQ sources on both widths. |

[Prior status records](../history/M5-T593-prior-current-records.md)
