# T368 S1: 80286 Successful-Retirement Route Inventory

## Scope and publisher

This is the complete dispatch-layer inventory for the remaining 80286
successful-retirement timing task.  It starts from the only publisher:
`core_machine_run` refreshes an instruction, rejects synchronous fault
delivery without retirement, calls `core_machine_instruction_cost`, and only
then adds the returned value to `elapsed_ticks`, traces `CPU_RETIRE`, and
advances the scheduler.  Thus a classifier return of
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` is an eligible successful-retirement
one-tick value, not a fault, device, or bus-time value.

## Ordered 80286 route ledger

`core_machine_instruction_cost` is an ordered classifier chain.  The table
lists every 80286-relevant layer, whether it can publish, and its S2
disposition.  A later layer is reachable only after each earlier layer returns
zero.

| Order | Classifier / input class | Current 80286 result | S2 treatment |
| --- | --- | --- | --- |
| 1 | `core_machine_string_io_source_instruction_cost`: MOVS/CMPS/STOS/LODS/SCAS/INS/OUTS including defined REP forms, plus ordinary IN/OUT | The 80286 repeat contract and 80286 source ledger supply existing values; undefined REP form falls through rather than publishing. | Audit the Intel context and coverage of each string/port form; preserve the dedicated repeat-state publisher. |
| 2 | 80386 dynamic multiply / secondary / privileged helpers | Guarded to 80386. | Non-reachable for 80286; no receiver allocation. |
| 3 | Legacy dynamic arithmetic helper | Guarded to 8086/80186. | Non-reachable for 80286; no receiver allocation. |
| 4 | `core_machine_primary_source_instruction_cost`: ALU, compare/test, XCHG, INC/DEC, MOV, LEA, adjust/conversion, Group 3 and immediate IMUL shapes | The 80286 switch already publishes source-model values including documented EA/odd-word additions. | Preserve as accepted source families; S2 must statically account for every source-timing shape and identify any shape that returns zero. |
| 5 | `core_machine_control_stack_source_instruction_cost`: calls/jumps/returns, pushes/pops, INT/IRET, ENTER/LEAVE, HLT and related control shapes | Existing 80286 ledger plus next-instruction term owns applicable successful contexts.  T366 S10 verifies every extracted control form has a ledger entry. | Preserve accepted rows; audit source-context exclusions (privilege, delivery, task/descriptor and unsupported forms) so they cannot be mistaken for a one-tick success. |
| 6 | `core_machine_80286_source_instruction_cost`: Jcc, selected system forms, FLAGS/XLAT, segment forms, BOUND/ARPL, direct MOV, Group 2 and immediate MOV | Selected rows from T366 and existing ledgers publish exact values.  Prefix, unsupported/system-context, invalid-encoding and default branches can return `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`. | Primary remaining receiver: classify every reachable success in these branches by exact Intel row or one explicit source-context transfer. |
| 7 | Compatibility cost | Never selected for a valid 80286 profile because layer 6 returns a result for every residual opcode. | Non-reachable for 80286; no receiver allocation. |

## Remaining one-tick classes

The prior T366 S10 audit identified broad anchors.  S1 refines their required
context rather than treating them as one opcode bucket:

| Anchor | Potential successful context requiring S2 distinction | Not an admission |
| --- | --- | --- |
| Nonzero prefix count in the final 80286 classifier | Segment, LOCK, REP and operand/address prefixes after higher-priority string/primary/control routes decline. | Faulting or undefined prefix combinations; these do not retire. |
| `0F` system branch | Secondary opcode, ModR/M extension, real/protected mode, CPL, descriptor outcome and memory/register form not already covered by T366 S26--S31. | Any synchronous exception/delivery path, which publishes no faulting-round ticks. |
| BOUND, ARPL, segment move, LES/LDS, Group 2 | Register-vs-memory and legal encoding/mode condition that reaches the residual branch. | Register BOUND/LES/LDS, invalid segment encodings, `/6` Group 2 and other invalid forms unless an execution trace proves successful retirement. |
| Final default except `B0`--`BF` | Every successful primary opcode not preempted by layers 1--5 and not individually selected in layer 6. | Merely decoded bytes or unimplemented/faulting encodings; S2 must prove success before giving a timing disposition. |
| Lookup fallback | A future or drifted `CORE_MACHINE_SOURCE_TIMING_*` form absent from the 80286 ledger. | Current control/stack forms: T366 S10 already proves they are present. |

## Finite next mechanism plan

S2 must first produce a source/form/context reachability table for all residual
prefix and final-80286 routes.  It must prove each candidate is either a
successful execution with an Intel-primary Appendix-B/other authoritative
row, or a non-retiring/unsupported path.  S3 then admits only the finite
source-backed successful family exposed by that table, with semantic/timing
tests; further subtasks repeat this mechanism order until no reachable
successful fallback remains.  A source range may use a same-profile model only
under the Queue policy; a source that cannot distinguish context transfers once
to the named receiver rather than receiving a scalar.

## Verification

- Read the entire ordered `core_machine_instruction_cost` chain and its
  80286-reachable helpers in `src/core/machine/machine.c`.
- Verified the sole successful publisher and synchronous-fault boundary in
  `core_machine_run`.
- Re-ran the T366 S10 static inventory logic by reviewing its anchors and
  control-ledger extraction contract in
  `cmake/verify_t366_s10_80286_unallocated_publishers.cmake`.
- This S is documentation/inventory only.  Documentation governance and
  `git diff --check` are required; it makes no source-value or runtime change.

## Non-claims and transfers

S1 neither declares the residual forms complete nor converts any retirement
cost into 8 MHz physical time.  It does not allocate fetch, memory/I/O wait,
DMA arbitration, device service, firmware, bus or 5170 L3 behavior.  Those
remain in their separately queued receivers after the complete successful
retirement ledger closes.
