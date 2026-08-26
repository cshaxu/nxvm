# T485 S17 8088 Delivery And Halting Source Evaluator

`M5:T485:S17:8088-DELIVERY-HALTING-SOURCE-EVALUATOR:ACCEPTANCE-CANDIDATE`

The retained `core_machine_control_stack_source_instruction_cost()` consumes
the complete S16 Table-2-21 batch through its existing immediate private
transfer plan.  No delivery/vector state, decoder or timing owner was added.

| Form | Base plus transfers | Exact result | Focused proof |
| --- | --- | --- | --- |
| HLT | `2 + 4 * 0` | `2` | one retired instruction |
| INT3 and `INT 3` | `52 + 4 * 5` | `72` | both opcode encodings |
| other `INT ib` | `51 + 4 * 5` | `71` | immediate type `67h` |
| INTO, OF set/clear | `53 + 4 * 5` / `4` | `73` / `4` | published old OF only |
| IRET | `24 + 4 * 3` | `36` | complete three-word real-mode frame |

The evaluator distinguishes `CD 03` from other `CD ib` forms using its already
decoded immediate byte, so the manual's type-3 row is not accidentally lost.
All listed outcomes are classified at the control/stack origin with a source
form.  INTR and NMI remain excluded because they are not instruction rows.

The focused retirement smoke passes after rebuilding the actual executable.
The complete current gate and documentation governance are recorded before
acceptance.  Counted code paths are the private timing model and focused smoke;
code-size accounting is recorded at acceptance.
