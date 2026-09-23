# T435 S6 P1 - 80186 Manual Re-baseline

## Purpose

This is the first bounded result of the 80186 re-audit.  It independently
re-read the authorized Intel 1985 manual before relying on the existing S1/S2
documents.  It does not certify decoder coverage or runtime conformance; those
are separate S6 gates.

## Authority and precedence

`I186-UM-1985`, printed Table 2-9, pp. 2-13--2-18, is the primary 80186
instruction-form and scalar-timing authority.  Its heading is explicitly the
80186/80188 CPU instruction-set summary, gives the encoded forms, marks the
80186-only rows, and states the timing assumptions.  Table 1-16, pp. 1-23--
1-40, is from the 8086/88 CPU chapter.  Its parenthetical 80186 column remains
useful only for the repeat formulas, prefix cost, and odd-address footnote not
stated by Table 2-9.  A Table 1-16 scalar cannot supersede an incompatible
Table 2-9 scalar.

This is a source-precedence decision, not an attempt to conceal the manual's
internal inconsistency.  The S1 ledger contains the finite conflict table and
the selected result for each affected form.

## Corrections made

| key family | old S1/S2 basis | re-read result | S2 consequence |
| --- | --- | --- | --- |
| `PUSH-IMM16`, `PUSH-IMM8` | 14, inherited from Table 1-16 parenthetical column | Table 2-9: 10, 10 | L3 target corrected; current route remains unproven |
| `CALL-NEAR`; `JMP-{SHORT,NEAR,FAR}` | 14; 13 | Table 2-9: 15; 14 | all four L3 scalars corrected at the existing selector |
| `IMUL-IMM8` | range 22--24, midpoint 23 | Table 2-9 range 22--25, midpoint 24 | L2 target corrected |
| `BOUND` | scalar L3 35 | Table 2-9 range 33--35 | L2 midpoint target 34 restored |
| `ENTER` lexical level | later-CPU `level % 32` applied to all profiles | instruction description p. 2-6: unsigned byte, as great as 255 | 80186 retains the byte; 80286/80386 retain their documented 0--31 rule |
| `LAHF` | 4 due to transposed Table 1-16 reading | Table 2-9 and Table 1-16 parenthetical: 2 | L3 target corrected |
| `NEG-M`, `NOT-M` | grouped sequence made `NOT-R` appear to be 17 | Table 2-9: both register/memory forms 3 | S1 row made unambiguous |
| `XLAT` | absent from S1 and S2 | Table 2-9: 11 | added as an L3 key; static source inspection shows 80186 currently falls to `source_timing_unallocated` |

## Resulting controlled denominator

The manifest expands 279 base keys: 253 L3 and 26 named L2 midpoint keys.
The new `XLAT` key is included in the denominator. `BOUND` remains an explicit
Table 2-9 range with its deterministic midpoint disclosed as L2, rather than
being silently made exact or unspecified. The
manifest gate reports 14 wrong-value, 43 unallocated, and 222 missing-test
base keys.  These are deficits, not passes.

## What remains deliberately open

This P1 has not yet produced the required decoder-form inventory or a
bidirectional decoder-to-ledger verifier.  It therefore cannot claim that 279
is the final decoder denominator, nor that every current S2 route/status is
reconciled.  S6 P2 must derive the decoder inventory; S6 P3 must attach the
per-key source route/status results and make both difference sets empty.

Markers: `M5:T435:S6:I186-MANUAL-REBASELINE:OK`;
`M5:T435:S6:I186-MANUAL-CONFLICTS-RECORDED:OK`.
