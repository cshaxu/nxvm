# T435 S9 - 80186 Profile Closure Evidence

## Scope

S9 implements the 80186 timing program against the accepted S1/S2 corpus.  It
does not reopen CPU instruction semantics already accepted by T401: the
[timing program](../../proposals/m5-core-cpu-instruction-timing-program.md)
requires decode, semantic execution and fault boundaries as non-regression
conditions, and assigns a newly discovered semantic defect to a separate
corrective admission.

The retained [T401 four-profile ledger](t401-s1-four-profile-cpu-audit-ledger.md)
contains the 80186-specific semantic evidence for BOUND, immediate IMUL,
PUSHA/POPA, PUSH immediate, INS/OUTS, ENTER/LEAVE and the shared instruction
families.  S9's successful-retirement recipes execute the decoder-owned forms
and reject an unclassified or unallocated retirement; no new semantic defect
was observed while supplying timing inputs.

## Timing closure

The [80186 manifest](../cpu-timing/t435-s2-80186-timing-manifest.json)
materializes 279 base keys, 248 legal single-axis contexts and 89 legal
combinations: 616 keys total.  The focused runner covers all of them with
real retirement observations, including distinct first, continuation and
zero-count REP phases.

`t435-s9-80186-timing-results.json` is generated only after all 616
observations exist.  Each record preserves source rule/level/context and
emits ticks, formula inputs, source form, timing origin and classified
disposition.  It is accepted by:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186TimingResults.ps1 -ResultPath docs/etc/cpu-timing/t435-s9-80186-timing-results.json
```

with `80186 timing results verified: conforming_keys=616`.

For REP zero-count records, an odd-word input is deliberately absent: no
16-bit transfer occurs, while a legal segment prefix remains an input.  This
is enforced by the profile verifier and is not an L1 fallback.

Markers: `M5:T435:S9:80186-PROFILE-CLOSURE:OK`;
`M5:T435:S9:80186-RESULTS-616:OK`.
