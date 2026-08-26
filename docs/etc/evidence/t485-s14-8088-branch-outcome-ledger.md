# T485 S14 8088 Branch Outcome Ledger

`M5:T485:S14:8088-BRANCH-OUTCOME-LEDGER:READY`

Visual review of rendered Intel Table 2-21 pages `2-56` through `2-60`
establishes a complete, zero-transfer source set:

| Form | Table clocks | Published outcome fact | Current disposition |
| --- | --- | --- | --- |
| `70h`--`7Fh` Jcc | taken `16`, fallthrough `4` | post-retirement IP differs from sequential IP | ready; no 8088 source selector currently consumes it |
| `E3h` JCXZ | taken `18`, fallthrough `6` | post-retirement IP differs from sequential IP | numerical path exists, but 8088 has no source-form attribution |
| `E2h` LOOP | taken `17`, fallthrough `5` | same | numerical path exists, but 8088 has no source-form attribution |
| `E1h` LOOPE/LOOPZ | taken `18`, fallthrough `6` | same | numerical path exists, but 8088 has no source-form attribution |
| `E0h` LOOPNE/LOOPNZ | taken `19`, fallthrough `5` | same | numerical path exists, but 8088 has no source-form attribution |

`core_machine_control_stack_source_instruction_cost()` already owns the
post-retirement loop/JCXZ result calculation.  `core_machine_legacy_source_instruction_cost()`
contains the matching 8086 Jcc outcome logic, but the 8088 selector never
reaches it.  The next receiver must consolidate those equivalent outcome
calculations at the existing control/stack owner, assign the existing source
form there, and select it before compatibility.  It must not re-execute flags,
guess a target, model prefetch or publish physical timing.
