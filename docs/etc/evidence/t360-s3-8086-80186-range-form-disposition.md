# T360 S3: 8086/80186 Range-Form Disposition

## Primary-source finding

The selected 8086 and 80186 multiply/divide timing rows are ranges, not
operand-to-clock algorithms.  The primary editions retained by T360 S1 are:

| Profile | Manual table | Selected range forms | Source disposition |
| --- | --- | --- | --- |
| 8086 | 9800722-03, Tables 2-20/2-21 | `F6`/`F7` `/4` `MUL` and `/5` `IMUL`, byte/word and register/memory forms | The table records ranges, including `MUL r8 70--77` and `MUL r16 118--133`, but no input-bit, operand-value, or microcode-step rule that maps one execution to a precise clock count. |
| 80186 | 210912-001, Table 1-16 | `F6`/`F7` `/4` `MUL`, `/5` `IMUL`, `/6` `DIV`, `/7` `IDIV`; `69 /r iw` and `6B /r ib` three-operand `IMUL` | The parenthesized 80186 figures remain ranges, including `IMUL imm8 22--24`, `IMUL imm16 29--32`, register ranges, and memory `+EA` ranges. The table supplies no operand-to-clock selection rule. |

The project cannot derive an exact source-clock result from those intervals.
Using the minimum, maximum, midpoint, a host measurement, an emulator, or a
later-profile formula would be a new unsourced timing model. None is admitted.

## Source and consumer sweep

| Sweep target | Result |
| --- | --- |
| Decoder shape | `core_machine_source_timing_primary_shape()` classifies the exact Group-3 extensions and `69`/`6B` immediate-IMUL encoding. No other selected primary shape is a manual range form. |
| 8086 source route | `core_machine_primary_source_instruction_cost()` returns no source result for all selected range shapes on `CORE_MACHINE_CPU_PROFILE_8086`; the sole profile fallback, `core_machine_8086_source_instruction_cost()`, retains `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. |
| 80186 source route | The same primary owner returns no source result for all selected range shapes on `CORE_MACHINE_CPU_PROFILE_80186`; `core_machine_80186_source_instruction_cost()` retains `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. |
| Distinct later-profile owners | 80286 fixed Group-3 rows remain in the 80286 source branch; the 80386 dynamic multiplier formula remains in `core_machine_80386_dynamic_multiply_cost()`. Neither is imported into an earlier profile. |
| Existing evidence and tests | T359 S2 records all selected ranges and their transfer. `core-machine-8086-instruction-timing-ledger-smoke` and `core-machine-80186-instruction-timing-ledger-smoke` retain their explicit unallocated-form fallback proof. No range form has a source-backed exact tick expectation. |
| Documentation and debt | T357/T359 ledgers, T360 S1, Queue, and TODO were swept for all selected opcode families. The single TODO below owns a later formula-or-probe admission; no silent timing allocation remains. |

The mechanical search was `rg -n "MUL|IMUL|DIV|IDIV|69|6B|UNALLOCATED|dynamic_multiply|source_instruction_cost" src/core/machine/machine.c docs tests` followed by direct review of all timing-owner hits. Other uses of these opcodes as semantic tests are not timing consumers.

## Exact transfer

The current one-tick unallocated value is a visible transfer marker, not a
claim that the corresponding hardware instruction takes one clock. The future
receiver must supply an Intel-primary operand-to-clock rule, or a separately
approved hardware-observation contract that can state what the primary manual
does not. It must then update only the appropriate profile-local owner, prove
that capture occurs before no publication boundary, and sweep all Group-3 and
immediate-IMUL variants. Until then, source allocation remains deliberately
absent.

## Verification boundary

S3 changes no instruction semantics, runtime timing number, CMake target,
artifact, ABI, or test logic. It makes the existing source absence explicit and
adds only its precise future admission path.
