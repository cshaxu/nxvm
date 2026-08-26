# T485 S10 8088 Control And Stack Transfer Ledger

`M5:T485:S10:8088-CONTROL-STACK-TRANSFER-LEDGER:READY`

The rendered 1981 Intel Table 2-21 gives the 8088 rule used by every row:
add four clocks per 16-bit word transfer.  Its CALL rows give near/direct 1,
far/direct 2, near-memory 2, near-register 1 and far-memory 4 transfers;
JMP gives direct/register 0, near-memory 1 and far-memory 2; PUSH/POP/PUSHF/
POPF give the shown 1 or 2 transfers; RET gives near 1 and far 2.  The same
table gives INT3/INT, IRET and HLT, but their delivery/halting outcomes remain
their own source batch.

## Finite current evaluator ledger

| 8086/8088 form family | Existing evaluator fact | 8088 disposition |
| --- | --- | --- |
| CALL near/far direct, near/far register or memory | opcode/ModRM form, memory role and exact Table 2-21 base | ready: 1/2/1/2/4 transfers |
| JMP direct/register/memory | same decoded form and memory role | ready: 0/0/1/2 transfers |
| RET near and immediate-pop | direct opcode, one stack word | ready: 1 transfer |
| RET far and immediate-pop | direct opcode, two stack words | ready, but current evaluator incorrectly gates it to 80186+; repair belongs to the same receiver |
| PUSH/POP register or segment, PUSHF/POPF | direct opcode and one stack word | ready: 1 transfer |
| PUSH memory / POP memory | ModRM memory role plus existing EA fact | ready: 2 transfers |
| INT3, INT imm, IRET, INTO, externally delivered interrupt/NMI, HLT | delivery, vector, flag or halting outcome affects the selected row | retain outside this batch |
| PUSHA/POPA, PUSH immediate, ENTER/LEAVE, BOUND | not 8086/8088 instructions | non-applicable |

## One receiver

`core_machine_control_stack_source_instruction_cost()` is the existing sole
decoder-derived control/stack evaluator.  Its `8086` source table supplies the
shared base rows; the needed 8088 work is a local complete transfer plan and
the existing evaluator's selection before compatibility.  The plan must add
`4 * transfers` from the table, not reuse byte traces or machine state.

The later implementation may extend the existing memory-addition helper for
the 8088 source rule and remove the erroneous FAR-RET profile gate.  It must
not alter execution semantics, add a control decoder, or admit delivery,
prefetch, external-cycle or physical-axis claims.
