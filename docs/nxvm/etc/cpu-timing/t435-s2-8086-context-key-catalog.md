# T435 S2 - 8086 Timing Context-Key Catalog

## Purpose and unit

This catalog supplies the atomic coverage unit missing from the S1 narrative
ledger. It does not revise a manual value, L3/L2 classification, or the
selected Group-3 model. It makes the S1 claim "every form/context" finite and
auditable for the 8086 implementation audit.

A key is `base-form ; context`. A base form distinguishes operand direction,
register/memory location, byte/word width where the manual gives a separate
row, conditional outcome, and separately encoded segment-register variant. It
does not multiply register identity or Table 2-20 addressing spelling: those
are formula inputs to the same key. Context is one of the bounded semantic
axes below. Invalid encodings, faults, interrupts and external device
completion remain outside the successful-retirement universe.

## Frozen base-form universe

The accepted S1 table normalizes to 228 L3 base keys and 16 L2 base keys. The
L3 count deduplicates the repeated `TEST accumulator,immediate` mention,
expands `MOV` segment transfer by direction and register/memory location (four
keys), and expands `PUSH`/`POP` segment by their four/three executable
opcodes. The 16 L2 keys are the four Group-3 families crossed with
r8/r16/m8/m16. Thus the pre-prefix base universe is 244 keys.

| key family | L3 base keys | L2 base keys | normalization rule |
| --- | ---: | ---: | --- |
| scalar, ALU, compare/test, increment, exchange, adjust and conversion | 74 | 0 | every displayed operand/outcome form is one key; duplicate `TEST accumulator,immediate` is one key |
| transfer and stack | 27 | 0 | expands segment `MOV` 4 and segment `PUSH`/`POP` 4/3 |
| control, branch, interrupt and port | 61 | 0 | expands every displayed branch/count/outcome form |
| Group-2 | 28 | 0 | seven operations times r,1/r,CL/m,1/m,CL |
| string and repeat | 24 | 0 | five primitive families times byte/word; seven legal repeat families times byte/word |
| flag/state, `WAIT`, `ESC` | 14 | 0 | `ESC` is register and memory |
| Group-3 range model | 0 | 16 | four families times r8/r16/m8/m16 |
| **total** | **228** | **16** | **244** |

## Bounded context axes and current route

| context key | finite admissible domain | current 8086 route | current disposition |
| --- | --- | --- | --- |
| `NONE` | all 244 base keys with no timing prefix | primary, control-stack, legacy dynamic or legacy fallback | 56 L3 keys are absent/conflicting and 16 L2 keys are absent/conflicting, as enumerated in the [implementation audit](t435-s2-8086-implementation-audit.md#atomic-base-form-reconciliation-and-count) |
| `SEGMENT` | one ES/CS/SS/DS override on a base key whose S1 formula consumes a source-effective-address operand | primary, legacy fallback, control-stack and legacy Group-3 add 2 for selected memory paths; string selection omits it | selected non-string paths conform; already-missing base paths remain missing; source-string keys require the missing +2 term |
| `LOCK` | one `F0` only on documented memory read-modify-write forms: arithmetic/logical destination, `INC`/`DEC`, `NEG`/`NOT`, and memory `XCHG` | current timing has no complete legal-RMW lock term program | the 19 legal L3 `LOCK` context keys are nonconforming; register, string/repeat, compare/test and shift/rotate forms create no key |
| `REP` | `REP MOVS`, `REP STOS`, `REP LODS`, each byte/word | string repeat contract | setup plus per-successful-primitive formula is selected; segment/odd-word subcontexts are assessed independently |
| `REPE` / `REPNE` | `CMPS` and `SCAS`, each prefix sense and byte/word | string repeat contract | setup, iteration and termination input are selected; segment/odd-word subcontexts are assessed independently |
| `ODD-WORD` | each word-transfer base or repeat key for which S1 Table 2-21 assigns +4 | primary/control/legacy paths call the odd-word helper; string selection has no such addition | selected non-string paths conform; word-string keys lack the term |

`SEGMENT`, `LOCK`, `REP` and `ODD-WORD` are axes, not standalone
retirements. A combined key is legal only when every named axis is legal for
its base form. This is a finite coverage grammar, not an unbounded list of
redundant prefix byte streams; the decoder's 15-byte containment limit is not
a timing-coverage dimension.

## Current prefix-context gaps

| class | affected finite key set | defect | repair owner |
| --- | --- | --- | --- |
| `LOCK(B)` | 19 legal memory-RMW base forms | no legal-RMW +2 prefix program | T435 8086 timing-program implementation |
| `SEGMENT(S)` | source-string `MOVS`, `CMPS`, `LODS`, primitive and legal repeat forms | string timing bypasses the segment +2 formula input | T435 8086 timing-program implementation |
| `ODD-WORD(W)` | word `MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, primitive and legal repeat forms | string timing has no odd-address transfer input | T435 8086 timing-program implementation |
| `SEGMENT` on an already failed memory base key | every such memory key | no selector exists to receive the additional term | T435 8086 timing-program implementation with the base-form repair |

No 8086 form/context may be declared conforming merely because its unprefixed
representative has a value. Closure requires every key produced by this grammar
to select its S1 constant/formula or labelled L2 model and no successful key to
publish `SOURCE_UNALLOCATED`.

Markers: `M5:T435:S2:8086-CONTEXT-CATALOG:OK`;
`M5:T435:S2:8086-PREFIX-COVERAGE:OK`.
