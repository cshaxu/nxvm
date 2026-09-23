# T388 S2: Successful Sentinel Matrix

`M5:T388:S2:SUCCESSFUL-SENTINEL-MATRIX:OK`

## Publication Rule

A source-cost helper is consulted only after the executor has refreshed a
successful instruction. `core_machine_instruction_cost()` then returns one
cost to `core_machine_publish_elapsed_ticks()`. A zero cost is rejected by the
publisher and is not successful elapsed-time publication. Faulted, rejected,
invalid-profile and delivered synchronous paths stop before that call. A
successful `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` value is therefore a real,
currently reachable deterministic timeline input, but is not a processor-clock
unit.

## Exhaustive Sentinel Sources

| Source anchor | Exact successful form/context class | Primary status | Physical-timebase disposition |
| --- | --- | --- | --- |
| `core_machine_source_timing_lookup` | A selected ledger caller requests a form absent from its immutable per-profile table. | Every presently selected lookup form is covered by T359/T360/T366 static ledger checks; a future absent row has no source value. | Guarded prohibition. T388 S3 must make lookup absence distinguishable from a physical cost before any profile receives a physical-clock contract. |
| `core_machine_legacy_source_instruction_cost`: non-segment prefix branch | 8086/80186 successful non-string forms with one or more prefix bytes other than the presently captured one segment override. | The retained 8086/80186 primary tables do not supply a complete prefix/context formula for these forms. | Explicit prohibition; T388 S3 needs a profile-local prefix/context capture plus primary row or qualified observation contract. |
| `core_machine_legacy_source_instruction_cost`: final fallback | 8086 and 80186 successful forms not selected by the earlier source owners. This includes 8086 `MUL`/`IMUL`, and 80186 `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate `IMUL` range rows. | T361 records only 8086 Tables 2-20/2-21 and 80186 Table 1-16 ranges; no primary operand formula selects a value. | Explicit prohibition. A future owner-approved 86Box/MAME/PCjs observation may be used only under a processor/fixture/calibration/error contract; no endpoint or midpoint is valid. |
| `core_machine_control_stack_source_lookup` defaults | Null machine or non-8086/80186/80286/80386 profile. | Not a valid configured successful profile path. | Non-retiring/non-profile path; no physical receiver. |
| `core_machine_string_io_source_instruction_cost` fallback | A string/REP or ordinary I/O form is not accepted by its selected repeat/port table and falls through to a profile classifier. The known 80286 `REP LODS` case is included. | T359/T360 preserve the missing Appendix-B formula/context rather than invent one. | Explicit prohibition; T388 S3 requires exact profile form/context evidence or a qualified observation contract. |
| `core_machine_80286_source_instruction_cost`: prefix branch | Any successful 80286 form with nonzero prefix count not already returned by the string/I/O, primary or control/stack owner. | T366 S10 and T368 S7 retain it as a source/bus-phase transfer; no complete prefix formula is admitted. | Explicit prohibition; T388 S3 must capture only primary-defined contexts and retain the rest outside physical-clock eligibility. |
| `core_machine_80286_source_instruction_cost`: guarded `0F`, `62`, `63`, `8C`, `8E`, `C4/C5`, shift and default branches | Successful legal variants whose ModR/M, mode, privilege, memory, next-byte or repeat context is not matched by an exact current classifier. | T359/T360/T363 retain system/descriptor and incomplete contextual rows as named receivers. Invalid variants are non-retiring and must not be counted as this row. | Explicit prohibition pending profile-local context capture and primary-ledger classification; execution-feasibility regressions are required before a branch may be narrowed. |
| `core_machine_80286_source_instruction_cost`: `WAIT`/ESC | Selected Model-339 no-FPU `9B` and ESC forms successfully complete with no coprocessor timing value. | T368 S7 proves the no-FPU route and transfers x87 timing. | Explicit prohibition and x87 receiver; not an 80286 scalar. |
| `core_machine_80386_source_instruction_cost`: prefix branch | Successful 80386 form with nonzero prefix count not consumed by an earlier owner. | Existing selected rows do not authorize a universal prefix formula. | Explicit prohibition pending 80386 lexical/context classification. |
| `core_machine_80386_source_instruction_cost`: Jcc preview branch | Taken short `Jcc` where `core_machine_cpu_execution_preview_lexeme()` does not yield an available target lexeme. | The selected source row needs target lexical component count; it is not recoverable from the current fallback. | Explicit prohibition pending a nonpublishing, failure-safe target-lexeme capture or qualified observation. |
| `core_machine_80386_source_instruction_cost`: final/default branch | Every otherwise successful 80386 opcode/context not accepted by string/I/O, dynamic multiply, secondary, privileged, primary or control/stack owner. | T359/T360/T363 record source rows only for selected forms; a generic default has no numerical authority. | Explicit prohibition pending exact profile form/context classification. |

## Source-Text Anchor

The legacy direct-sentinel condition is exactly
`prefixes != 0u && !segment_override`; its source line remains distinct from
primary-owner early returns that may already classify a valid segment override.
The constrained 80286 branch anchors are `case 0x0fu:`, `case 0x62u:`,
`case 0x63u:`, `case 0x8cu:`, `case 0x8eu:` and
`case 0xc4u: case 0xc5u:`; the 80386 context anchor is
`core_machine_cpu_execution_preview_lexeme`.

## Eligibility Result

No selected 8086, 80186, 80286 or 80386 profile is yet eligible to describe its
*complete active successful-retirement* axis as a physical oscillator. Existing
Model-339 ratios remain deterministic-event contracts only; Model-40's neutral
ratios remain correct. This is not a CPU semantic rejection: rows that fault or
are profile-rejected publish nothing, while successful unallocated rows retain
deterministic progress but cannot drive a physical claim.

## Bounded Next Core Receiver

T388 S3 must add one Core-owned eligibility boundary at the sole publication
site. It must preserve deterministic elapsed progression, make an unallocated
successful cost unable to be mistaken for a physical CPU-clock contribution,
and expose no second scheduler or VM-owned reclassification path. Before it
can mark any profile physically eligible, it must complete the corresponding
profile-local primary/qualified-observation matrix and prove reset, overflow,
fault, fallback and all clock-consumer behavior. S3 may initially establish a
truthful ineligible state and regressions; it must not pause guest devices on an
unknown instruction or relabel the fallback as one oscillator tick.

## Static Recurrence Boundary

`verify-t388-successful-sentinel-matrix` binds the sole publisher, dispatch
order, every direct source fallback family, the 80286 no-FPU route and the
80386 target-preview condition to this matrix. It is a drift detector, not
source authority and not a physical timing proof.