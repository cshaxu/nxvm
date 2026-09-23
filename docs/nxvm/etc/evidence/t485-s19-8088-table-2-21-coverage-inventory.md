# T485 S19 8088 Table 2-21 Coverage Inventory

`M5:T485:S19:8088-TABLE-2-21-COVERAGE-INVENTORY:ACCEPTED`

## Frozen universe and source check

The universe is every successful 8088 instruction row in the rendered Intel
*iAPX 86 and 88 User's Manual* (1981), Table 2-21 (printed pages 2-51 through
2-68), plus the table's prefix rows.  The table gives a per-row clock value,
its `Transfers` count, and the 8088 rule of four additional clocks for every
16-bit word transfer.  It is not an external-cycle, prefetch, device-service,
or whole-machine physical-time contract.

The sweep compares the actual 8088 selector order (string, primary,
control/stack, compatibility) with the accepted S7--S18 receivers.  A form
is marked covered only when that selector can produce a source-attributed
result today; a source table entry that belongs to an 8086-only evaluator is
not coverage.

| Table-2-21 family | Complete current 8088 disposition | Evidence or next receiver |
| --- | --- | --- |
| ALU, CMP, TEST, INC/DEC, XCHG, immediate MOV, LEA, adjust, conversion, NOT/NEG | Exact primary receiver for the decoded non-segment forms and complete transfer plan | S7 primary evaluator |
| MOVS, CMPS, STOS, LODS, SCAS, with REP variants | Exact string receiver, including byte/word transfer and repeat phase | S9 |
| IN/OUT byte and word | Exact port-I/O receiver, one word transfer for word forms | S13 |
| CALL, JMP, RET, PUSH, POP, PUSHF, POPF | Exact control/stack receiver with complete transfer plan | S11 |
| Jcc, JCXZ, LOOP, LOOPE, LOOPNE | Exact control/stack receiver using published post-retirement outcome | S15 |
| HLT, INT3, INT ib, INTO, IRET | Exact control/stack receiver; INTR/NMI are not instruction rows | S17 |
| Group-3 MUL, IMUL, DIV, IDIV | Intel supplies ranges, not a selection formula; no exact receiver is lawful | S18 range boundary |
| NOP; CLC/CLD/CLI/CMC/STC/STD/STI; LAHF/SAHF | Exact scalar, zero-transfer source rows exist, but the current 8088 selector has no fixed-simple receiver | Next finite fixed-simple batch |
| XLAT | Exact source row with one byte transfer, but it needs its own decoded memory/segment input check rather than being conflated with scalar flag rows | Separate XLAT input audit |
| MOV r/m,r, MOV r,r/m, MOV moffs; MOV segment; LDS/LES | Exact source rows, but the current 8088 primary evaluator does not select these forms and their segment/pointer transfer plan is not yet admitted | Next finite move/segment batch |
| ROL/ROR/RCL/RCR/SHL/SHR/SAL/SAR | Count, width, memory and transfer facts require one source-qualified Group-2 plan; no 8088 receiver today | Separate Group-2 source/plan batch |
| LOCK; REP/REPE/REPNE; SEGMENT override prefix rows | REP is covered only where the string receiver owns its legal executed primitive; SEGMENT is covered only where the owning primary/string receiver has the legal memory-form input. LOCK's source prefix term and legal-base qualification have no 8088 receiver. A prefix alone is not a successful instruction row | Prefix-qualified batch; do not add it to compatibility |
| ESC and WAIT | Their row cost depends on the FPU command/wait contract; table text alone does not authorize treating CPU retirement as FPU service time | FPU/8087 boundary batch |
| Invalid encodings, faults, INTR, NMI and single-step | The table explicitly marks INTR, NMI and single-step as not instructions; faults/invalid encodings have no successful instruction row. Their delivery timing is a separate Core/board contract | Explicit exclusion; no compatibility claim |

## Result and ordered next work

The covered and residual rows exhaust this table universe.  The immediate
implementation receiver is the finite fixed-simple batch because every listed
row has a scalar Intel value and no new machine state.  It may consolidate at
the existing primary evaluator and must not create a flag-timing or opcode
parallel path.  XLAT, move/segment, Group-2, prefix, and FPU batches remain
separate because their decoded input, transfer, validity, or collaborator
boundaries differ.

No range endpoint, emulator-derived timing, source-unallocated compatibility
value, or profile/VM time behavior is promoted by this inventory.
