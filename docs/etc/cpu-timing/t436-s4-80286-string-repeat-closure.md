# T436 S4 80286 String And Repeat Closure

S4 owns 32 base string/string-I/O and REP forms plus 97 legal contexts:
seven primitive word odd-address forms, nine REP word odd-address base forms,
54 REP phase forms, and 27 word-odd REP phase combinations. The executable
partition therefore assigns S4 exactly 129 canonical successful retirements.

The runner already exercised primitive strings and every REP phase, but it had
not retained a distinct observation for each ordinary REP base key. S4 replaces
the word-only base helper with a common `CX=1` base execution for all 18 REP
forms and retains the existing odd-word variant for the nine word forms. This
adds the missing 18 real records without inventing a phase or a repeat count.

The S4 completion assertion requires a classified `string-io` retirement for
every owned key. Every REP record must expose repeat and repeat-phase inputs;
every non-zero word transfer must expose odd-word input, while zero-count REP
correctly has no memory-transfer odd-word requirement. Each primitive and REP
recipe independently checks its Appendix-B tick result and timing origin.

The rebuilt runner captures 591 unique 80286 records and proves S4's complete
129-record partition. The final 800-key writer remains unavailable because
S5--S7 retain the remaining 209 records.

The counted tracked test-source delta is 109 added and 18 removed lines (91
net). The generalized base helper replaces the word-only helper; no duplicate
REP execution path is retained.

```text
M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=591:canonical=800
M5:T436:S2:I286-INCOMPLETE-RESULT-REFUSED:PASS
M5:T436:S4:I286-STRING-OBSERVED:129
M5:T436:S4:I286-STRING-COVERAGE:PASS:canonical=129
M5:T436:S4:I286-REPEAT-INPUTS:PASS
```
