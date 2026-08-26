# T485 S9 8088 String Source Evaluator

`M5:T485:S9:8088-STRING-SOURCE-EVALUATOR:OK`

## Retained owner path

The 8088 selector now tries the existing string evaluator before the existing
primary and compatibility routes.  That evaluator continues to own the one
Table-2-21 string ledger, opcode-to-form classifier, repeat phase and repeat
continuation state.  A private `core_machine_source_transfer_plan` is created
only for an 8088 selected string form and consumed in the same evaluation.  It
holds the existing form, an exact word-transfer count and completeness; it is
not machine state or a public contract.

The shared plan type also makes the existing primary evaluator's completeness
check explicit.  No timing table, decoded form, repeat state, API or
compatibility writer was duplicated.

## Exact result

For 8088 strings, the retained 8086 Table-2-21 base is used and the private
plan adds `4 * word_transfers` only after an executed word primitive.  Thus
MOVS/CMPS word forms add 8; STOS/LODS/SCAS word forms add 4; every byte form
and a zero-count REP adds 0.  The existing two-clock segment term remains
owned by the same evaluator and applies to MOVS, CMPS and LODS.

The focused retirement smoke directly proves every byte/word primitive:

| Form | Byte clocks | Word clocks |
| --- | ---: | ---: |
| MOVS | 18 | 26 |
| CMPS | 22 | 30 |
| STOS | 11 | 15 |
| LODS | 12 | 16 |
| SCAS | 15 | 19 |

It also proves segment-override MOVSW `28`, REP MOVSW first/continuation
`34/25`, and zero-count REP MOVSW `9`.  The existing 8088 repeated-NOP probe
still selects compatibility with a source-unallocated result, proving the
string receiver did not broaden the unsupported surface.

## Boundaries and simplicity accounting

The changed tracked source/test paths are `cpu_timing.c`,
`cpu_timing_model.c` and the focused retirement smoke.  The result keeps one
selector, evaluator, source table, classifier and repeat-state owner; the
small shared plan replaces the earlier primary-only shape rather than adding a
second plan representation.  Port I/O, control/stack, dynamic, fault/interrupt,
prefetch, external-cycle, board wait and physical-axis work remain unallocated
or transferred to their own source batches.

The focused smoke and the complete 299-test current gate pass on 2026-08-26.
