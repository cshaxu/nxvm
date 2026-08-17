# T388 S4: Residual Form/Context Ledger

`M5:T388:S4:RESIDUAL-FORM-CONTEXT-LEDGER:OK`

## Authority And Scope

This ledger reconciles every **reachable successful** sentinel family from T388
S2 with the post-refresh classifier order. Intel-primary timing authority stays
with the retained T360 source editions: 8086 Tables 2-20/2-21, 80186 Table
1-16, 80286 Appendix B, and 80386 PRM section 17.2.2.3 plus the applicable
instruction page. The tables' stated no-wait/no-HOLD/prefetch, alignment, mode
and context assumptions remain part of each row; no value is inferred here.

T363 proves that a selected exact form already routes through a private owner
before any profile classifier. This ledger covers only the residual route that
reaches `core_machine_source_timing_mark_unallocated()` or a missing immutable
lookup. Faulted, rejected, invalid-profile and delivered paths publish no
successful retirement and are excluded.

## Exhaustive Residual Partition

| Profile/family | Actual remaining successful context | Primary disposition | S4 receiver decision |
| --- | --- | --- | --- |
| All four: immutable lookup miss | A future selected owner asks for a form absent from its private ledger. | No source row exists. | Guarded prohibition; later owner must add exact form/context evidence and a focused regression before it can select physical mode. |
| 8086/80186: legacy prefix | A non-string success has one or more prefixes other than the captured segment override. | The primary tables do not provide a universal prefix formula. | Context-capture receiver: separate legal prefix, opcode/form, operand/address and bus assumptions; do not charge a generic prefix. |
| 8086: legacy dynamic arithmetic | `F6/F7` `/4`--`/5` `MUL`/`IMUL` reaches the legacy fallback. | 1979 tables provide ranges, not an operand rule. | Range-only prohibition; T361's Intel-first observation contract is the only later candidate. |
| 80186: legacy dynamic arithmetic | `F6/F7` `/4`--`/7` and `69/6B` IMUL range forms reach fallback. | 1985 table provides parenthesized ranges, not a selecting formula. | Range-only prohibition; same T361 observation-contract receiver, never a borrowed 80386 formula. |
| All four: string/I-O fallback | A string/REP or ordinary-I/O success is not accepted by the selected repeat/port owner; this includes the retained 80286 `REP LODS` boundary. | T359/T363 allocate only the exact captured rows. | Form/context receiver: repeat state, prefix, permission and service conditions must be captured; denied/provider-fault paths remain non-retiring. |
| 80286: prefix branch | A nonzero-prefix success survives earlier exact owners. | Appendix B does not authorize a universal prefix row. | Context-capture receiver; retain source/bus-phase separation and do not use the Model-339 deterministic ratio. |
| 80286: guarded forms | `0F`, `62`, `63`, `8C`, `8E`, `C4/C5`, shift and default branches have a successful legal variant whose required ModR/M, mode, privilege, memory, next-byte or repeat context is not selected. | Existing Appendix-B rows are exact only when their stated context is available. | Per-form capture receiver; T359/T363 selected rows remain unchanged, while absent context stays prohibited. |
| 80286: `WAIT`/ESC | Model-339 no-FPU `9B`/ESC completes without an x87 timing source. | No CPU-only source row is admitted for the coprocessor route. | x87 receiver and permanent CPU-clock prohibition until a separately selected FPU contract exists. |
| 80386: prefix branch | A nonzero-prefix success survives selected owners. | PRM rows vary by legal prefix, operand/address form and mode. | Lexical/context-capture receiver; no universal prefix value. |
| 80386: taken short `Jcc` preview | The target lexeme required for the documented `m` term is unavailable. | The PRM needs that target component count. | Nonpublishing target-lexeme capture receiver; qualified observation is only a later alternative. |
| 80386: final/default branch | A success was not selected by string/I-O, dynamic multiply, secondary, privileged, primary or control/stack owner. | A generic default has no numerical authority. | Per-form classification receiver; each newly admitted form needs its profile/mode/prefix source record and regression. |

`core_machine_control_stack_source_lookup` null/non-profile defaults are not a
configured successful profile route; they are an explicit non-retiring
exclusion rather than a physical-time receiver.

## Source Anchors

The exhaustive table binds these implementation anchors without treating the
identifier as authority: `core_machine_source_timing_lookup`,
`core_machine_legacy_source_instruction_cost`,
`core_machine_string_io_source_instruction_cost`,
`core_machine_80286_source_instruction_cost`,
`core_machine_80386_source_instruction_cost`,
`core_machine_cpu_execution_preview_lexeme`, and
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. Primary anchors are `Tables 2-20/2-21`, `Table 1-16`, `Appendix B`, and `section 17.2.2.3`.
## Ordered Follow-up

1. Preserve the S3 physical rejection while future work narrows a row.
2. First admit deterministic, source-backed **context capture** for the 80386
   target-lexeme and 80286 Appendix-B distinctions; it must publish no new
   clock scalar until its complete successful forms are known.
3. Then decide each profile's remaining legal prefix/form space against its
   primary manual. A range-only legacy arithmetic row remains blocked unless
   the separate T361 observation contract is admitted and passes its own
   calibration/error requirements.
4. Only after every reachable residual row for one profile is either exact or
   prohibited may a later S consider enabling that profile's physical contract.

No current profile meets step 4. This record creates no physical frequency,
VM ratio, device-clock advancement, artifact revision or L3 claim.

## Static Recurrence Boundary

`verify-t388-residual-form-context-ledger` binds this ledger to the S2 family
anchors, the source classifiers, the private physical-eligibility boundary and
the retained primary-authority evidence. It is a drift detector, not a source
or timing proof.