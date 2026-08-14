# T357 S5: 80186 Instruction-Timing Ledger

## Source contract

S5 selects the parenthesized 80186 figures in Table 1-16 of the *Intel iAPX
86/88/186/188 User's Manual* (order 210912-001).  The table states its
prefetched, no-wait/no-HOLD and even-word assumptions.  Those conditions are
not simulated here; bus ownership, prefetch depletion, physical waits and
cycle-exact execution remain later transfers.

| Form | 80186 clocks | Focused evidence |
| --- | ---: | --- |
| `NOP`; `CLC`; word immediate/register `MOV` | 3; 2; 4/2 | baseline vectors |
| `MOV reg,memory`; `MOV memory,reg` | 12+EA; 9+EA | direct-memory vectors |
| accumulator/memory `MOV` | 8 read; 9 write | odd-address moffs vectors |
| segment override; odd word | +2; +4/word | overridden/direct odd vectors |
| short Jcc | 13 taken; 4 not taken | taken and fall-through vectors |
| `MOVSB`; `REP MOVSB` | 9; 8+8/repetition | three primitive vector |
| `IN` immediate/DX; `OUT` immediate/DX | 10/8; 9/7 | installed-provider vectors |

## Construction and transfer boundary

8086 and 80186 share the real 16-bit EA, segment-override and odd-word
construction because the cited sources prescribe the same shape.  They use
separate immutable contracts for every clock value, `REP` setup/iteration,
and Jcc direction.  This removes a duplicate width/profile construction while
retaining source-required differences.  The sole elapsed-tick publisher is
unchanged and executes only after successful instruction refresh.

The owner smoke uses deliberately unrelated compatibility configuration
values. It proves source selection, unallocated one-tick fallback, delivered
`#UD` and requested-stop zero publication, reset/replay, budget preflight,
overflow, provider publication, and the 27-clock maximum selected form.

The generic compatibility smokes now select 80286: once both 8086 and 80186
are source-backed, they cannot truthfully assert their old synthetic cadence.
80286 and all unallocated 80186 forms remain explicit future source-ledger
transfers.
