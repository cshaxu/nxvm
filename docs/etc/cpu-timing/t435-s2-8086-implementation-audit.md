# T435 S2 - 8086 Timing Implementation Audit

## Scope and method

This audit consumes the accepted [8086 evidence ledger](t435-s1-8086-ledger.md)
and compares it to the actual 8086 execution route. It is an audit only: no
runtime value, decoder behaviour, or timing-plan interface changes in this
block.

This is the accepted pre-S4 audit baseline. Its route descriptions and
nonconforming dispositions are historical evidence, not current implementation
status. The complete current disposition is the 649-key S4 runtime artifact
[`t435-s4-8086-timing-results.json`](t435-s4-8086-timing-results.json),
verified by `Verify-8086TimingResults.ps1`.

The sole selection/publication route is:

```text
decoded instruction -> core_machine_instruction_cost()
  -> string I/O -> 80386-only dynamic/secondary/privileged -> primary
  -> control-stack -> legacy 8086 fallback -> source-unallocated
  -> core_machine_retirement_observation_publish()
```

The route is in `src/core/machine/machine.c`. The one-tick terminal is
`core_machine_source_timing_mark_unallocated()` and marks
`source_timing_unallocated`; it is observable at retirement and is deliberately
accepted by an existing 8086 smoke. Therefore it is an audited nonconformance,
not a harmless fallback.

## Concrete implementation mapping

| S1 ledger family/context | actual selector and accounting | audit disposition |
| --- | --- | --- |
| basic `NOP`, `CLC`; `MOV` r/r, r/m, m/r, moffs, immediate | 8086 source ledger plus `core_machine_legacy_source_instruction_cost()` | conforming only for these selected forms; baseline and memory smoke coverage exists |
| ALU, `CMP`, `TEST`, `INC/DEC`, `NEG/NOT`, `XCHG`, `LEA`, adjust, conversion and Group-3 decode shapes | `core_machine_primary_source_instruction_cost()` plus 8086 EA, segment and odd-word helpers | partial: the shared shape selector implements a material subset, but no ledger-complete form/context proof exists |
| direct/indirect `CALL/JMP`, near `RET`, ordinary stack forms, `INT`/`IRET`, `JCXZ`/`LOOP*`, and branch outcomes | `core_machine_control_stack_source_instruction_cost()` and primary/control helpers | current base-form selector values conform; complete form/path regression coverage remains missing |
| `IN`/`OUT`, ordinary string forms, legal REP variants | port and string helpers before primary selection; 8086 repeat contract | partial: selected ports and `MOVS`/`REP MOVS` smoke coverage exists; all five string families and every legal repeat termination are unproven |
| Table 2-20 EA and segment override | `core_machine_8086_timing_effective_address()` and `core_machine_8086_timing_has_segment_override()` | partially conforming: helper encodes the Table 2-20 buckets and segment +2; focused tests cover direct/indexed/override examples, not the complete EA cross-product |
| odd-addressed word transfers | `core_machine_8086_timing_odd_word()` multiplied by selected transfer count | partial: read/write examples exist, but source-form transfer cardinality is not proven for every ledger row |
| `MUL` and `IMUL` L2 rows | `core_machine_legacy_dynamic_arithmetic_model_cost()` | conflict: current 8086 values are fixed lower endpoints (70/118 and 80/128, with memory additions), not `L2-86BOX-8086-G3` operand-dependent modelling |
| `DIV` and `IDIV` L2 rows | no 8086 dynamic case reaches a source rule; the legacy dynamic helper returns false for both | absent: successful forms reach later unallocated selection |
| Group-2 rotate/shift, including `D0 C0` | no 8086 source case after shared-primary non-match | absent: the existing smoke explicitly executes `D0 C0` at one tick |
| `AAA`/`AAS`, segment `MOV`, `LDS`/`LES`, segment stack forms, far `RET`, Group-2, unlisted flag/state, `WAIT`, `ESC` | absent primary/control/fallback selector, except `AAA`/`AAS` which select an incorrect 8-tick value | exact base-form failures enumerated below; no current regression may claim ledger conformance |
| invalid encodings, faults, `INTR`/`NMI`, prefetch, READY/HOLD, 8087 completion | not successful retirement per S1 boundary | correctly outside this audit's instruction scalar; no repair assigned here |

## Regression evidence actually present

`tests/machine/core_machine_8086_instruction_timing_ledger_smoke.c` proves
only the following representative values: `NOP`, `CLC`, immediate and
register `MOV`; selected direct/indexed/odd/overridden memory `MOV`; one taken
and not-taken `Jcc`; `MOVSB` and a three-element `REP MOVSB`; four scalar port
forms; reset/budget/overflow behaviour. It also deliberately asserts that the
unallocated `D0 C0` form completes in one tick. This is useful evidence that
the current route is deterministic, but it is direct evidence of the closure
gap and cannot qualify a ledger-complete 8086 program.

`tests/core/machine_retirement_observation_s3_smoke.c` separately proves that
the 8086 unallocated condition is published with the 8086 fallback origin.
Thus no path hides the current failure from observation, but the physical
retirement guard does not turn it into a ledger-compliant timing result.

## Atomic base-form reconciliation and count

The earlier family-level table is not a closure metric.  This section
supersedes its aggregate "remaining accepted rows" wording.  A *base-form
key* is one S1 Table 2-21 instruction/operand/outcome key with no separately
counted prefix cross-product.  Width is retained where S1 distinguishes it;
the `MOV` segment row is expanded by direction and register/memory form.  A
key is counted below only when the current 8086 route is either unallocated or
returns a value that conflicts with the accepted S1 disposition.  Missing
regression breadth alone is not counted as an implementation failure.

| S1 level | nonconforming base-form keys | calculation | current route | disposition |
| --- | ---: | --- | --- | --- |
| L3 | 56 | `AAA`/`AAS` 2 + `MOV` segment direction/form 4 + `LDS`/`LES` 2 + `PUSH` segment 4 + `POP` segment 3 + `RET` far/far+imm 2 + Group-2 7 x 4 forms 28 + flag/state 8 + `WAIT` 1 + `ESC` r/m 2 | primary returns 8 instead of 4 for `AAA`/`AAS`; every other listed key has no 8086 selector and reaches the legacy one-tick unallocated terminal | not L3 |
| L2 | 16 | `MUL`, `IMUL`, `DIV`, `IDIV`, each r8/r16/m8/m16 | `MUL`/`IMUL` 8 keys use fixed lower endpoints rather than `L2-86BOX-8086-G3`; `DIV`/`IDIV` 8 keys are unallocated | not L2 |

At mnemonic-family granularity, the same result is 25 L3-affected families
(`AAA`, `AAS`, `MOV` segment, `LDS`, `LES`, `PUSH` segment, `POP` segment,
`RET` far, seven Group-2 families, eight flag/state instructions, `WAIT`, and
`ESC`) and four L2 families.  The base-form metric, not that mnemonic count,
is the repair and closure metric.

The 56 is an exact count for the S1 ledger's presently enumerable base-form
keys.  It is deliberately **not** presented as a complete prefix-context
count: S1 records `SEGMENT`, `LOCK`, and repeat applicability as a grouped
rule, rather than assigning each legal prefixed instruction its own atomic
key.  Static inspection already proves a context defect: the string selector
uses the repeat contract but does not add the 8086 segment-override term, and
the non-segment prefix paths are not universally source-backed.  Therefore no
honest global L3 total including every prefix combination can yet be stated.
This is not transferred to a later task: R2/R3 must first normalize those
prefix contexts into finite keys, then the resulting table must be reconciled
to zero before 8086 implementation closure.

That normalization is now recorded in the companion [8086 context-key
catalog](t435-s2-8086-context-key-catalog.md). It fixes the atomic base
universe at 228 L3 plus 16 L2 keys and gives every prefix, repeat and
odd-word context a bounded applicability and current-route disposition.
The [implementation tracker](t435-s2-8086-implementation-tracker.md) is the
controlling S2 closure ledger: its expanded per-key status, rather than a
family aggregate, decides whether any implementation batch is complete.

## Finite repair set for the next implementation stage

| ID | required repair and owned boundary | closure proof |
| --- | --- | --- |
| R1 | Replace the 8086 Group-3 fixed endpoint implementation with an independently written `L2-86BOX-8086-G3` model; include `DIV` and `IDIV`, manual-range containment, and memory EA/override/odd-word additions; no fixed fallback is permitted | operand partitions compare to the selected model and remain inside every Intel range |
| R2 | Materialize a complete 8086 form/context program for every L3 exact/formula row and route every accepted successful form to it before retirement | generated or table-driven form coverage proves no accepted row reaches unallocated |
| R3 | Make prefix, all EA buckets, per-form word-transfer cardinality, branch/count outcomes and all legal string repeat phases explicit inputs to that program | cross-product regressions for Table 2-20, odd words, prefixes, outcomes and repeat termination |
| R4 | Remove the successful-8086 `SOURCE_UNALLOCATED` route and replace the smoke that expects `D0 C0 == 1` with ledger values and negative assertions | no successful 8086 form emits unallocated origin/form; invalid/fault paths stay outside retirement |
| R5 | Replace representative-only 8086 timing smoke coverage with ledger-keyed tests that assert source form, origin, formula inputs and published retirement ticks | every accepted S1 ledger member has one or more focused ownership regressions |

## Audit conclusion

The current 8086 implementation is not ledger-complete. It has a usable
single retirement publisher and isolated correct source rows/formula helpers,
but it also retains the forbidden anonymous one-tick successful-retirement
route. Its existing Group-3 implementation conflicts with the accepted L2
model and omits two required arithmetic families. R1--R5 are a finite,
in-scope repair set for the later implementation phase; no evidence or
architecture decision is transferred or reopened.

Markers: `M5:T435:S2:8086-IMPLEMENTATION-AUDIT:OK`;
`M5:T435:S2:8086-REPAIR-SET:OK`.
