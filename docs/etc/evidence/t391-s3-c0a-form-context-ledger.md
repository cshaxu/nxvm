# T391 S3: C0A Form And Context Ledger

`M5:T391:S3:C0A-LEDGER:OK`

## Frozen batch

T391 S2 defines M40-C0A as exactly the three classified successful retirements
between the protected-to-real C0 transition and the first post-C0 immediate
port-61h read. The copied observer retains opcode/escape/group, ModR/M form
and extension, control outcome, next-lexeme components, repeat, mode,
privilege, operand/address size, LOCK, source form, origin, disposition and
source ticks. It retains no raw trace, PC, asset identity, path, digest,
firmware byte or media data.

| Unit | Captured form and required context | Existing source row and sole owner | Regression boundary | Disposition |
| --- | --- | --- | --- | --- |
| C0A-001 | `0F 01 /3` LIDT, memory operand, real-address CPL0 success, no VM86 or rejected prefix/LOCK path; classified at 11 ticks. | Intel 80386 PRM section 17.2.2.3, `0F 01 /3` memory-only LIDT row, 11 clocks; private `core_machine_80386_privileged_source_instruction_cost()` owner. T359 S6 records this exact real/protected-CPL0 row. | Existing privileged-form and table-register semantic smokes; S2 capture records the selected successful row. | Accepted source-row/context reconciliation. Remains deterministic/nonphysical. |
| C0A-002 | `FF /4` near indirect JMP, register ModR/M, successful taken control transfer, real-address context, two following-lexeme components, classified at `7+2=9` ticks. | Intel 80386 PRM Table 8-1 near `JMP r/m16/32` register row, `7+m`; private `core_machine_control_stack_source_instruction_cost()` owner obtains `m` from the nonpublishing preview. T359 S3 records the row. | Existing control/stack timing-owner smoke and lexical-preview smoke; S2 capture retains register form, taken outcome and `m=2`. | Accepted source-row/context reconciliation. This is not the prior `FF /5` far-memory JMP receiver and remains deterministic/nonphysical. |
| C0A-003 | `E4 ib` immediate-port input, real-address CPL0 success, no repeat/LOCK/permission path, endpoint read at port 61h, classified at 12 ticks. | Intel 80386 PRM Chapter 17 immediate `IN` row, 12 clocks; private ordinary-I/O timing owner selects `CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE`. T357 S3 records the exact row. | Existing instruction-timing ledger smoke and S2 canonical C0A capture smoke. | Accepted source-row/context reconciliation. Remains deterministic/nonphysical. |

## Result and transfer

The complete three-unit C0A batch is reconciled without an inferred source
row, a new observer field, or a production change. All units already have one
private Core timing owner and existing focused regression boundary. The result
is startup-semantic readiness only: it makes a separately admitted C1 stage
eligible to use this finite predecessor checkpoint. It neither selects
physical retirement nor proves board, device, firmware, media, or L3 timing.

The next T391 receiver is a bounded C1 admission/inventory that must define
its own finite semantic endpoint and use the existing copied observation. Any
physical-retirement work remains behind its separate queue prerequisite.
