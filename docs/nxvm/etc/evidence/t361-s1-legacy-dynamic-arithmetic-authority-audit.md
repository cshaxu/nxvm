# T361 S1: Legacy Dynamic Arithmetic Timing Authority Audit

## Primary-source result

The bounded primary review confirms the T360 transfer.  The 8086 October 1979
manual Tables 2-20/2-21 list ranges for `F6`/`F7` `MUL` and `IMUL`; the 80186
1985 manual Table 1-16 lists parenthesized ranges for `MUL`, `IMUL`, `DIV`,
`IDIV`, and immediate `IMUL`.  Neither cited table supplies an operand-bit,
quotient, multiplier, register/memory, or microcode rule that selects one
clock within each range.  The scoped source/provenance search found no
contemporary Intel formula attached to those table entries.

| Profile | Forms | Primary-source disposition |
| --- | --- | --- |
| 8086 | `F6`/`F7` `/4 MUL`, `/5 IMUL`, byte and word register/memory forms | Table ranges only; no formula. |
| 80186 | `F6`/`F7` `/4 MUL`, `/5 IMUL`, `/6 DIV`, `/7 IDIV`, byte and word register/memory forms | Parenthesized ranges only; no formula. |
| 80186 | `69` and `6B` immediate `IMUL` forms | Parenthesized ranges only; no formula. |

The 80386 multiplier formula is deliberately irrelevant: it belongs to the
later profile's distinct hardware and is not authority for either legacy
profile.

## Consumer and publication sweep

`core_machine_source_timing_primary_shape()` identifies precisely these Group-3
extensions and `69`/`6B` immediate forms. For 8086 and 80186,
`core_machine_primary_source_instruction_cost()` returns no exact source row
for every listed range form. The profile-local fallback then returns
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`; the 80286 fixed rows and
`core_machine_80386_dynamic_multiply_cost()` remain separate later-profile
owners. `core_machine_instruction_cost()` is still the only post-refresh
elapsed-tick publisher.

The sweep covered production timing owners, the two legacy timing-ledger
smokes, T357/T359/T360 evidence, Queue and TODO. No endpoint, midpoint,
benchmark-derived value, host measurement, emulator value, or hidden numeric
allocation was found.

## Required owner decision before S2

No source-backed runtime allocation is admissible. To continue T361 beyond
this audit, the owner must choose one of these bounded paths:

1. retain the no-formula transfer until a newly discovered primary Intel source
   provides a complete formula; or
2. explicitly authorize a dedicated real-hardware observation contract.

The second path must name the exact processor/stepping, oscillator and bus
configuration, instruction and memory fixtures, measurement apparatus and
calibration, sample/repetition/error policy, raw-result provenance, formula or
lookup publication rule, failure/rollback boundary, and the complete
profile-local consumer sweep. It may not use a host benchmark, a later x86,
Bochs, PCjs, or an undocumented range endpoint. The observation task is not
admitted by this record.

## Result boundary

S1 changes no timing value, runtime path, CMake target, test source, artifact,
or ABI. It makes no L3 or cycle-exact claim; it preserves the explicit transfer
until the owner selects a lawful authority path.
