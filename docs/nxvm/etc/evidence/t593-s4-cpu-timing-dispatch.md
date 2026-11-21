# T593 S4 CPU timing dispatch proof

The remaining shared selector used to invoke the dynamic multiply,
legacy arithmetic, secondary and privileged evaluators for an ordinary
80386 or Am386SXLV retirement before reaching the common ledgers.
The repeat and string evaluator still runs first. A non-string can
clear the active repeat state, so its rejected path is meaningful.

The [new selector](../../../../src/x86-core/cpu/cpu_timing.c) uses the
opcode indexed by the existing decoded prefix count. It keeps the
[evaluator bodies](../../../../src/x86-core/cpu/cpu_timing_model.c)
unchanged. The admission proof follows directly from those bodies.

| Evaluator | Prior necessary predicate | Preserved invocation |
| --- | --- | --- |
| Dynamic multiply | 386 semantics and 69h, 6Bh, F6h/F7h extensions 4 or 5, or 0Fh AFh | Invoke only for those candidates. The owner verifies full length, effective operands and valid prefix shape. |
| Secondary | 386 semantics and 0Fh with a following byte | Invoke for that escape after a rejected AFh multiply. |
| Privileged | 386 semantics and 63h or 0Fh with a following byte | Invoke directly for 63h, or after the secondary evaluator rejects the escape. |
| Legacy arithmetic | In this selector tail, only 8086. The 8088 and 80186 already have separate branches. | Retain for the 8086 and preserve the other profile branches. |

Skipping an unavailable evaluator does not remove a state update. These
specialized evaluators reject an excluded opcode before any write to the
outcome. The shared try operation resets timing attribution before each
remaining evaluation. Dropped attempts are followed by another existing
attempt, so the initially unattributed value and selected final value
match the original. Same-profile precedence inside the 0Fh escape is
multiply, secondary, privileged. The legacy and 386-specific outcomes
are disjoint by profile. The common source, control and compatibility
fallback chain, all timing constants and all timing origins are unchanged.

Each ordinary 386 primary opcode outside the recognized groups avoids
four original evaluator calls and the associated repeated candidate
checks. The new code performs bounded opcode predicates instead. This
reduces executed work for those instructions. The total selection
remains bounded per instruction, so this is not a new asymptotic bound
for full execution and does not imply a MyOB300 wall-time multiplier.
No exploratory performance replay was added.

The two cached Release widths passed the same 30 existing correctness
cases, covering profile ledgers from 8086 through 80386, V30 and 486SX
timing, string/port timing, dynamic arithmetic, privileged and secondary
forms, next-lexeme preview, and retirement observation. No new test
framework or timing threshold was added.

| Transcript | SHA-256 |
| --- | --- |
| x64 targeted tests | `89f7fdface2357fc368da5f10a2834617bce197d8b77d3259949f4ebdd78e83a` |
| x86 targeted tests | `9a3322eb08e9e9a3282c99ecc1b6d21ae94e98caaeb844bf69e1c24d278066f5` |
