# T485 S16 8088 Delivery And Halting Source Ledger

`M5:T485:S16:8088-DELIVERY-HALTING-LEDGER:ACCEPTANCE-CANDIDATE`

Visual review of the 1981 Intel iAPX 86/88 User's Manual, Table 2-21,
printed pages `2-55` and `2-56` (PDF pages `74` and `75`) establishes the
complete finite batch.  The table footnote gives the 8088 rule: add four
clocks for every listed 16-bit word transfer.

| Form | Table base and transfers | Exact 8088 result | Current Core input | Disposition |
| --- | --- | --- | --- | --- |
| `F4` HLT | `2`, none | `2` | opcode only | source-complete |
| `CC` INT3 | INT imm8 type `3`: `52`, `5` | `72` | opcode only | source-complete |
| `CD ib` INT | INT imm8 type other than `3`: `51`, `5` | `71` | opcode plus immediate form | source-complete |
| `CE` INTO, OF set | `53`, `5` | `73` | published pre-retirement OF | source-complete |
| `CE` INTO, OF clear | `4`, no delivery | `4` | published pre-retirement OF | source-complete |
| `CF` IRET | `24`, `3` | `36` | opcode only | source-complete |

`core_machine_control_stack_source_instruction_cost()` is the sole current
8088 selector owner.  It presently marks exactly these forms source-unallocated
before compatibility.  It already owns the 8086 base ledger and the completed
pre-retirement flag snapshot used by `INTO`; no vector identity, delivered
interrupt/NMI state, target calculation, flag re-execution, prefetch or
physical-time fact is required by these software-instruction rows.

The receiver is one extension of the existing immediate private transfer plan:
INT3/INT/overflow-true INTO use five word transfers, IRET uses three, HLT and
overflow-false INTO use none.  It must retain the same control/stack evaluator
and generic source-form attribution; it must not include INTR/NMI, which the
manual explicitly labels non-instructions.

The S17 implementation proof must execute each listed outcome at the
control/stack origin and verify the exact results above.  This audit imports no
manual content, firmware, media or third-party source.
