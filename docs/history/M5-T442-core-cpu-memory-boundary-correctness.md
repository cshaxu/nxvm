# M5 T442: Core CPU And Physical-Memory Boundary Correctness

T442 is admitted from the owner-approved queue candidate
[Core CPU and physical-memory boundary correctness](../proposals/m5-core-cpu-memory-boundary-correctness.md).
S1 freezes the confirmed CPU lexical-guard and physical-mapping span boundary,
then repairs them at their respective Core owners without adding a memory layer.

## S1 Result

The one lexical owner now distinguishes 8086 primary `0F`, 80186 rejection,
and 80286/80386 extended `0F`; the one RAM mapping owner rejects an overflowing
physical span before mapping publication. Focused proof, sweep dispositions,
minimalism accounting, and the 0442 artifact are retained in the
[S1 evidence](../etc/evidence/t442-s1-core-cpu-memory-boundary-correctness.md).
