# T363 S2: arithmetic and data reconciliation

## Result

S2 finds that the complete primary arithmetic, FLAGS, data, ModR/M and EA
mechanism is already source-backed by the accepted T359 S2 ledger and its
successors; no runtime value or classifier change is warranted. This is a
complete reconciliation result, not implicit acceptance of a fallback.

`core_machine_source_timing_primary_shape()` classifies the encoding once, and
`core_machine_primary_source_instruction_cost()` selects the profile-local row
after successful refresh. It distinguishes r/m write/read, accumulator and
register immediates, memory, width, EA and documented odd-word transfers.

| Form class | Owner/disposition |
| --- | --- |
| ALU, CMP, TEST, XCHG, INC/DEC, MOV, LEA, adjustment and conversion | Exact Intel rows retained from T359 S2 for all four profile branches. |
| 80386 MUL/IMUL | Existing dynamic owner applies the documented early-out formula. |
| Legacy Group-3 and immediate IMUL ranges | T361/T362 dynamic owner and constrained table; no qualifying fallback remains. |
| 80286 Group-3/immediate IMUL and 80386 DIV/IDIV | Exact successful rows in the primary classifier; divide faults publish no retirement tick. |
| x87, bus/prefetch/cache/wait, device service, fault/delivery | Explicit non-S2 receivers; no clock is inferred. |

The static sweep covers all S2 form enum/shape/profile cases, dynamic owners,
fallbacks, smoke corpus and CMake. It found no second publisher or
handler-local clock. Unsupported prefix contexts retain the visible receiver.

## Verification

On 2026-08-14, `core-machine-t359-s2-timing-smoke` and
`core-machine-legacy-timing-normalization-s2-smoke` passed. They cover profile
rows, memory/EA, width, SETcc, Group-3, dynamic formula, odd-word ownership,
preflight and fault nonpublication. No runnable path changed, so no T363
developer artifact is created. S3 receives control/stack/real-control forms.
