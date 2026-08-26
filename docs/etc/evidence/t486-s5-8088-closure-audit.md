# T486 S5 8088 Closure Audit

`M5:T486:S5:8088-CLOSURE-AUDIT:ACCEPTANCE-CANDIDATE`

| Frozen List-1 universe | Current closure disposition | Evidence/receiver |
| --- | --- | --- |
| Arithmetic, compare/test, adjust/conversion, ordinary MOV, LEA, XCHG, NOT/NEG and scalar flags | Exact Manual-L3 CPU-side result at the existing primary owner. | S2 source rows, S3 disposition and S4 retirement coverage. |
| String operations/repetition and IN/OUT | Exact Manual-L3 CPU-side result at the existing string/I-O owner. | S2/S3 and retained byte/word/repetition/port-I/O retirement coverage. |
| Calls, jumps, returns, stack, branches, HLT and software delivery | Exact Manual-L3 CPU-side result at the existing control/stack owner. | S2/S3 and S4's segment-stack coverage. |
| Segment-register MOV, LDS/LES, XLAT, ESC, Group-2 and LOCK | Exact Manual-L3 CPU-side result at the existing primary or control/stack owner. | S4 source-to-retirement cases and one selector sweep. ESC is CPU dispatch only. |
| Group-3 MUL/IMUL/DIV/IDIV ranges | Intel ranges are retained as Manual-L3 source facts, but no exact runtime value is selected. | Source-unallocated/L2 result; a future source-qualified selection rule, not this CPU T, is the receiver. |
| WAIT `3+5n` | The formula is retained, but no external wait-iteration service contract is owned by the CPU timing path. | Source-unallocated/L2 CPU-to-x87 boundary. |
| Non-string REP; invalid/non-applicable prefix combinations | No standalone prefix retirement is legal. | Source-unallocated compatibility endpoint; strings retain their existing owner. |
| INTR, NMI, SINGLE STEP | Intel labels them non-instructions. | XT board/8259A and later board interrupt-contract tasks are the earliest receivers. |

The final owner sweep finds one selector in `cpu_timing.c`, with private
string/I-O, primary and control/stack evaluators in `cpu_timing_model.c`; no
8088 legacy evaluator, profile or VM timing route is selected. The isolated
Debug focused retirement smoke and current specialized gate (356 targets)
pass. The release-probe O3 build still reports unrelated pre-existing
maybe-uninitialized test warnings; it does not alter this source-to-retirement
conclusion or introduce a T486 receiver.

T486 is ready to close: its remaining rows are explicit source-bounded
dispositions, not hidden repair work. The next implementation candidate is the
independent IBM 5160 board/bus contract.
