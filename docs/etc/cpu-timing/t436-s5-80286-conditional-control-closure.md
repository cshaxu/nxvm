# T436 S5 80286 Conditional-Control Closure

S5 owns the 42 base `Jcc`, `JCXZ`, `LOOP`/`LOOPE`/`LOOPNE`, and `INTO`
forms and their legal ordinary-control contexts. The final partition is 54
canonical records: 32 `Jcc`, two `JCXZ`, six LOOP-family outcomes, two INTO
base outcomes, and twelve positive-length INTO next-byte contexts.

The Appendix-B clock-table footnote adds one clock per byte of the next
instruction executed. The inherited implementation applied this to taken
80286 `INTO` but omitted it for the non-overflow outcome. S5 repairs the
single control-stack timing owner so non-overflow INTO also uses the existing
next-instruction preview. It also marks control-stack retirements with the
existing `CONTROL` formula input; no second publisher, fallback, public API,
board policy, or timing route was added.

The exhaustive recipe sweep executes both outcomes for every conditional
form. It checks exact ticks, origin and classified disposition, and directly
checks the resulting IP: conditional branches use their taken or fall-through
destination; INTO uses the interrupt handler or immediate fall-through. The
runner captures every INTO next-byte length from one through six for both
outcomes, plus one real base observation for each outcome.

S5 also corrects a shared context-legality defect. The inherited selector
generated `NEXT-BYTE-0` for 29 control-transfer forms. That cannot represent
a successful next instruction, which necessarily executes at least one byte.
Removing those impossible contexts changes the live 80286 universe from 800
to 771 records: 286 base + 423 single-axis + 62 combined. S3/S4 remain
303/129; S5 is 54; the future S6 partition becomes 193; S7 remains 92.
Historical S1--S4 capture totals remain evidence of their then-current runs;
the manifest, partition and final writer now use the corrected live universe.

The rebuilt runner captures 603 unique records. Its final writer still refuses
output because S6 and S7 retain 168 records.

```text
M5:T436:S1:I286-PARTITION-COMPLETE:PASS:canonical=771:S3=303:S4=129:S5=54:S6=193:S7=92
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=603:canonical=771
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
M5:T436:S5:I286-CONDITIONAL-OBSERVED:54
M5:T436:S5:I286-CONDITIONAL-COVERAGE:PASS:canonical=54
M5:T436:S5:I286-OUTCOME-NEXT-BYTE:PASS
```
