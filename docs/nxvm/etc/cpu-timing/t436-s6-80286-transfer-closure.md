# T436 S6 80286 Transfer Closure

S6 owns the 27 Appendix-B transfer bases and all 193 legal canonical
observations for `CALL`, `RET`, `JMP`, `INT`/`INT3`, and `IRET`.  The runner
executes direct and indirect real/protected paths, same- and outer-privilege
returns, call gates, task/TSS transfers, IDT task gates, and NT task return.
Every capture is a successful classified Control Stack retirement; every
next-byte row proves the `CONTROL` formula input, and the two legal indirect
memory contexts retain their EA/segment input checks.

The implementation fixes the retired-path selection rather than adding a
fallback.  The core timing owner identifies a successful 80286 task transfer
from the published TR transition and selects the Appendix-B task rows: CALL
177/182, JMP 175/180, INT 167, IRET 169, with the next-instruction byte term.
It also selects protected direct call-gate rows (41, 82 plus four clocks per
parameter) and protected INT rows (40/78).  The execution owner now implements
the previously unimplemented same-privilege 16-bit JMP call-gate path: it
validates the gate and target descriptors, publishes the target CS:IP, and
does not create a return frame, stack switch, or parameter copy.

Base records are paired with their actual `NEXT-BYTE-1` retirement.  This is
one physical execution of the same successful transfer path, recorded once as
the path base and once as its Appendix-B `+N` context; it does not synthesize
timing or copy an unrelated result.

The focused runner now captures 764 of 771 live records.  S3/S4/S5 remain
303/129/54; S6 is complete at 193, while S7 retains the remaining 7 system
records.  Final 771-key emission remains deliberately refused until S7.

```text
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=764:canonical=771
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
M5:T436:S6:I286-TRANSFER-OBSERVED:193:canonical=193
M5:T436:S6:I286-TRANSFER-COVERAGE:PASS:canonical=193
M5:T436:S6:I286-PROTECTED-PATHS:PASS
```
