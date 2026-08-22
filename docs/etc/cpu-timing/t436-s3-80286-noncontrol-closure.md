# T436 S3 80286 Non-Control Closure

S3 reconciled two faults in the inherited generated coverage set before adding
runtime proof. The `ODD-WORD` selector had admitted seven register or immediate
forms, although the retained Appendix-B/context rule permits that term only
for a 16-bit memory transfer. They were removed rather than supplied with
invented recipes. `I286-INT-TASK` and its seven next-byte variants are
protected task-transfer keys and therefore belong to S6, not S3.

The executable partition and manifest/decoder verifiers now prove the legal
universe is 286 base + 452 single-axis + 62 combined = 800 canonical keys.
S3 owns 149 base and 303 canonical keys; S6 owns 27 base and 220 canonical
keys. This correction changes no Intel timing rule or Core instruction path.

S3 adds real 80286 execution for the remaining non-control inputs: direct
odd-word `IMUL imm16,m16`, and EA-BID plus odd-word for word `MUL`, `IMUL`,
`DIV`, `IDIV`, `MOV r,m`, `MOV m,r`, `MOV m,imm`, `PUSH m`, and `POP m`.
Each recipe addresses an odd 16-bit memory operand through base+index+
displacement, checks the expected Appendix-B ticks, classified origin and
retirement observation, and carries both EA and odd-word formula inputs.

The focused runner executes 616 probes, captures 573 unique canonical records,
and reports all 303 S3 records observed. Its final result writer still refuses
output because S4--S7 own the remaining 227 legal records.

The counted tracked source/test delta is 210 added and 12 removed lines in the
manifest runner, plus a one-line Core catalog assertion rebaseline: 198 net
lines. The retained recipes and S3-completion assertion are live; no obsolete
odd-memory fixture remains.

```text
M5:T436:S1:I286-PARTITION-COMPLETE:PASS:canonical=800:S3=303:S4=129:S5=56:S6=220:S7=92
M5:T435:S7:I286-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:canonical=800
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=573:canonical=800
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
M5:T436:S3:I286-NONCONTROL-OBSERVED:303
M5:T436:S3:I286-NONCONTROL-COVERAGE:PASS:canonical=303
M5:T436:S3:I286-MEMORY-INPUTS:PASS
```
