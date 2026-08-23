# M5 T442: Core CPU And Physical-Memory Boundary Correctness

T442 is admitted from the owner-approved queue candidate retained as its
[proposal companion](M5-T442-core-cpu-memory-boundary-correctness-proposal.md).
S1 freezes the confirmed CPU lexical-guard and physical-mapping span boundary,
then repairs them at their respective Core owners without adding a memory layer.

## S1 Result

The one lexical owner now distinguishes 8086 primary `0F`, 80186 rejection,
and 80286/80386 extended `0F`; the one RAM mapping owner rejects an overflowing
physical span before mapping publication. Focused proof, sweep dispositions,
minimalism accounting, and the 0442 artifact are retained in the
[S1 evidence](../etc/evidence/t442-s1-core-cpu-memory-boundary-correctness.md).

## Identifier Record

The implementation (`f882abe7`) and closure (`6746d400`) were pushed before
their commit subjects were found not to carry the required immutable S/P
identifier. This record preserves those source commits and supplies the T442
S1 P1 identifier without rewriting published history.
