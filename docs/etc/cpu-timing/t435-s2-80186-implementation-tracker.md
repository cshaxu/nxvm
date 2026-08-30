# T435 S2 - 80186 L3/L2 Implementation Tracker

## Controlling artifact

The [80186 manifest](t435-s2-80186-timing-manifest.json) is the sole
per-key closure ledger.  It expands 279 base instruction keys from the
accepted [80186 manual ledger](t435-s1-80186-ledger.md), then 248 legal
single-axis and 89 legal combined/phase context keys under the shared
[context-legality contract](t435-s2-context-legality.md). This document is the
human implementation map; it
cannot change a key's source rule, level, status, batch or regression ID.

Run:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186TimingManifest.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186DecoderLedger.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-80186TimingResults.ps1 -ResultPath <real-result.json>
```

before every 80186 tracker-changing P.  The verifier rejects duplicate keys,
missing required fields, invalid level/status values, wrong profile and count
drift.  It is a manifest-integrity gate, not runtime evidence.  The later
focused test must consume every generated `key_id` and publish target ticks or
formula inputs, retirement origin and `source_timing_unallocated` under the
shared [manifest result contract](t435-s2-manifest-result-contract.md).
The result verifier admits exactly the 616 generated 80186 records and checks
their manifest provenance, unique real observation, non-terminal publication,
and every required legal context input. It deliberately has no baseline result
file: a missing or partial file is evidence of an open key, never a synthetic
pass.

P23 corrects the prior `G2-*` selector typo: the 21 legal odd-addressed
16-bit memory Group-2 keys are now materialized as the named
`ROL`/`ROR`/`RCL`/`RCR`/`SHL`/`SHR`/`SAR` forms. They retain `missing-input`
until one real result proves their Table 1-16 `+4` term.

P34--P65 add `core-machine-80186-timing-manifest-runner`. All 279 canonical
base recipes now pass actual retirement observation, and its metadata coverage
gate rejects a missing base key or a duplicate-only recipe set. P55 corrects
the former noncanonical `I186-LEA` / `I186-ENTER-LGT1` spellings, makes the
runner fail with observed facts, aligns each recipe's Core origin, and fixes
the 80186 `ENTER` level byte plus `JCXZ` taken and `LOOPNE` not-taken timing
rows. P57 adds a mapped real-mode return frame and closes the missing 80186
`RETF` / `RETF imm16` control-stack source rows (22 / 25 clocks). P58 adds
real-mode IVT and stack preparation for `INT3`, `INT imm8`, and both `INTO`
outcomes, plus all four scalar `IN`/`OUT` forms. P59 adds a successful bounded
range, both `ESC` forms, zero-wait `WAIT`, and the `HLT` retired-then-waiting
boundary. P60 proves all fourteen non-repeat string byte-and-word primitives.
P61 proves every `REP` base key's first, continuation and zero-count phase.
P62 assigns those three phase observations to all 54 separately generated
`REP-PHASE-*` metadata keys and rejects a missing key. Recipes check ticks,
origin, classified disposition and, for repeat, phase and input bits. P63
proves all 19 legal memory-RMW `LOCK` keys and checks the common selector's
manual `+2` prefix term. P64 derives and proves all 19 `LOCK-SEGMENT` keys,
with both manual terms present. Odd-word and string-prefix combinations remain
deliberately open; the runner writes no final result document and every
manifest status remains open until all 616 keys have a real recipe.

P65 corrects the `ODD-WORD` selector to remove eight impossible pure-register
forms (`MOV-RR`, `MOV-RI`, two segment-register forms, and four Group-3
register forms). This reduces the canonical universe from 624 to 616 only by
enforcing the pre-existing legality contract: an odd-word context names a
16-bit memory transfer, never a register operation.

P66 turns every one of the 87 legal single-axis `ODD-WORD` keys into a real
successful-retirement observation.  The runner now proves the Table 1-16
`+4` term for direct memory, word read-modify-write (two transfers), far
pointers, stack-memory forms, word string primitives, and first-iteration REP
word strings.  It also closes two production defects exposed by that evidence:
word Group-2 RMW and Group-3 arithmetic had omitted the second/required odd
transfer, and `INSW`/`OUTSW` had not classified their string transfer or
published the odd-word formula input.  This does **not** close the remaining
segment-plus-odd and repeat-combination keys.

P67 turns every one of the 88 legal single-axis `SEGMENT` keys into a real
successful-retirement observation.  The runner derives the normal memory
recipes, uses the source-consuming segment string forms, and supplies separate
memory-form recipes for `CALL/JMP r/m16`: their base keys also admit registers,
but their segment contexts legally name only the memory realization.  The
Table 1-16 `+2` term passes for all 88; no production selector change was
needed.

P68 closes every remaining legal context combination: the runner now observes
all 54 repeat-phase keys, 19 `LOCK-SEGMENT` keys, 7 `SEGMENT-ODD-WORD` keys,
and all 63 legal REP phase combinations.  Its repeat harness independently
checks first, continuation and zero-count executions; the latter deliberately
retains a segment term but not a transfer-dependent odd-word term.  All 616
manifest keys therefore have runner evidence, although the final result
document still must be generated before the profile's final predicate can be
claimed.

P69 makes that final predicate executable.  The runner refuses to write
`t435-s9-80186-timing-results.json` unless all 616 manifest records have a
real observation, then serializes each retirement's ticks, formula inputs,
published form ID, origin and classified disposition.  The profile-specific
result verifier accepts the artifact with `conforming_keys=616`.  Its
zero-count rule deliberately does not require `ODD_WORD`: no transfer occurs
in that phase, so the manual's transfer-dependent term is absent.

## Per-key implementation map

| manifest template | level and authoritative rule | present state | close in |
| --- | --- | --- | --- |
| `I186-ADJ-*`, `I186-INC/DEC-*`, `I186-NEG/NOT-*`, `I186-XCHG-*`, `I186-LEA/LDS/LES-*` | L3 Table 2-9 exact | every base key has real retirement proof; legal prefix/transfer contexts remain open | B0+B1+B4 |
| `I186-ALU-*`, `I186-CMP-*`, `I186-TEST-*` | L3 exact except the explicit AI rows below | every base key has real retirement proof; legal prefix/transfer contexts remain open | B0+B1+B3+B4 |
| `I186-ALU-*-AI`, `I186-CMP-AI`, `I186-TEST-AI`, `I186-MOV-{RI,MI}` | L2:midpoint, manual range and `ceil` rule in S1 | every fixed-midpoint base key has real retirement proof; legal contexts remain open | B0+B2+B4 |
| `I186-MOV-*`, stack, call/jump/return, branches, interrupt/port/ESC/WAIT/HLT | L3 exact/formula | every base form has real retirement proof; legal prefix/transfer contexts remain open | B0+B1+B3+B4 |
| `I186-XLAT` | L3 Table 2-9 exact 11 | base result is proven; no legal context is admitted | B1+B3+B4 |
| `I186-{MUL,IMUL,IDIV}-*`, `I186-IMUL-IMM-*` | L2:midpoint accepted S1 values | every fixed-midpoint base form has real retirement proof; legal transfer contexts remain open | B2+B3+B4 |
| `I186-DIV-*` | L3 exact | every base scalar has real retirement proof; legal transfer contexts remain open | B0+B1+B3+B4 |
| `I186-{ROL,ROR,RCL,RCR,SHL,SHR,SAR}-*` | L3 formula; count modulo 32 | every base form has real retirement proof; count boundary and prefix/transfer contexts remain open | B1+B3+B4 |
| `I186-STRING-*`, `I186-REP-*` | L3 primitive/repeat formula | every primitive base and all REP phases have real retirement proof; legal prefix/transfer contexts remain open | B0+B1+B3+B4 |
| `I186-BOUND` | L2 midpoint 34 within Table 2-9 range 33--35 | successful result proves the disclosed range-model selection; prefix contexts remain open | B0+B4 |
| `I186-{ENTER-*,LEAVE}`, `I186-FLAG-*` | L3 exact/formula | every base form has real retirement proof; no independent legal context is admitted | B0+B1+B3+B4 |
| generated `-ODD-WORD` (87 legal single-axis keys) | S1 Table 1-16 `+4` odd-addressed 16-bit transfer | every key has a real successful-retirement observation | B3+B4 |
| generated `-SEGMENT` (88 legal single-axis keys) | S1 Table 1-16 segment-prefix `+2` | every key has a real successful-retirement observation | B3+B4 |
| generated `-LOCK`, `-REP-PHASE-*` and all legal combined suffixes | S1 Table 1-16 context rule | every legal context key has a real successful-retirement observation | B3+B4 |

The template form is deliberately compact, but braces in the manifest expand
one concrete key per opcode/form/outcome/width/context.  A test result for one
member never closes a sibling.

## Non-negotiable final predicate

The profile closes only when every base and context key has one source rule,
one L3/L2:midpoint target, one current route, one batch, one regression ID and
a focused result marking it `conforming`.  No key may retain `wrong-value`,
`unallocated`, `missing-input` or `missing-test`; no successful retirement may
take the anonymous terminal; and no pre-existing timing selector may coexist
with the B0 publisher.

Markers: `M5:T435:S2:80186-IMPLEMENTATION-TRACKER:OK`;
`M5:T435:S2:80186-TRACKER-CLOSURE-PREDICATE:OK`.
