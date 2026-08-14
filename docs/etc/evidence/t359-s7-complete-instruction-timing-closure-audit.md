# T359 S7: Complete Instruction-Timing Corpus Closure Audit

## Decision

T359 closes its bounded instruction-retirement corpus. It does **not** claim
that every executable x86 byte now has an exact clock count, nor that NXVM is
cycle exact. Each dispatchable primary or `0F` form in the S1 inventory is now
either owned by one exact-source classifier or bound to a named later receiver.
The remaining visible one-tick result is a transfer sentinel, not an Intel
timing claim.

## Mechanical closure sweep

The audit reran `verify-t359-instruction-timing-inventory` against the one
256-entry primary dispatch, one 256-entry `0F` dispatch, metadata classifier,
and `core_machine_instruction_cost()` in `machine.c`. That publisher is called
only after a successful instruction refresh by `core_machine_run()`. It
consults these private successful-retirement owners in this order:

1. S4 string/repeat/ordinary-I/O;
2. S2 dynamic 80386 multiply;
3. S5 secondary integer;
4. S6 privileged forms;
5. S2 primary data/ALU;
6. S3 control/stack; and
7. the retained profile fallback, whose unallocated result is explicit.

No decoder, instruction handler, selector/table/control helper, exception
delivery helper, or device provider writes elapsed instruction time. A
rejected or faulting instruction does not reach this publisher; the run loop
returns before tick publication. Thus no S7 result changes architectural
execution, ABI, device scheduling, or physical timing.

## Receiver reconciliation

| S1 receiver family | Exact T359 owner and record | Closure disposition |
| --- | --- | --- |
| Primary ALU, TEST/XCHG, adjustments, conversion, INC/DEC, Group 3, immediate IMUL, MOV/LEA, and SETcc | [S2 ledger](t359-s2-four-profile-arithmetic-data-source-ledger.md) | Selected fixed/formula rows remain at the sole publisher. |
| Calls, jumps, returns, stacks, ENTER/LEAVE, LOOP/JCXZ, HLT, real-mode software interrupt and same-level return | [S3 ledger](t359-s3-four-profile-control-stack-source-ledger.md) | Selected successful control/stack rows remain at the sole publisher. |
| Prefix state, strings, REP/REPE/REPNE and ordinary IN/OUT | [S4 ledger](t359-s4-four-profile-string-io-source-ledger.md) | Selected primitive/repeat and ordinary-I/O rows remain at the sole publisher. |
| 80386 near Jcc, bit/double-shift, two-operand IMUL, scan, MOVX and their selected lexical variants | [S5 ledger](t359-s5-80386-secondary-source-ledger.md) | Selected successful secondary rows remain at the sole publisher. |
| ARPL; table/selector/MSW; LAR/CLTS; CR/DR/TR; FS/GS stack; LSS/LFS/LGS | [S6 ledger](t359-s6-privileged-source-ledger.md) | Only explicitly fixed 80386 successful rows remain at the sole publisher. |
| Invalid profile/opcode/extension and illegal LOCK forms | S1 inventory and the owning form smoke | No successful-retirement clock; decoder/metadata rejection is retained. |
| ESC/FWAIT coprocessor execution | S1 external classification and [Broaden present x87 TODO](../../states/TODO.md) | External to T359: CPU-side interface is retained, numerical x87 timing is not claimed. |

The S3 implementation commits `a33d9d31`/`c69852cd`, accepted at `89ec6b81`,
are now recorded in the task history as well as their retained ledger. This
repairs a documentation-chain omission without altering S3's accepted
mechanism or evidence.

## Exact transfers

| Unallocated class | Required later receiver | Why T359 cannot allocate it |
| --- | --- | --- |
| 8086/80186 range-only MUL/IMUL/DIV/IDIV and 80186 immediate-IMUL rows; 80286 Appendix-B normalization; 80286 REP LODS; descriptor-granular LSL; any table/prose conflict or incomplete selected source variant | [T360 four-profile Intel timing source reconciliation](../../history/M5-T360-four-profile-timing-source-reconciliation-proposal.md) | The primary material supplies a range, an unresolved table/prose relationship, or a state distinction not captured at the nonpublishing retirement boundary. No midpoint or borrowed 80386 row is valid. |
| Faults, rejected prefix/LOCK forms, exception/IRQ/NMI, task gate/switch, cross-privilege return/entry, and VM86 frame delivery | [Cycle-exact selected profile](../../proposals/m5-cycle-exact-selected-profile.md) | These are delivery or transition mechanisms, not successful instruction retirement. |
| Memory/I/O waits, HOLD/DMA ownership, prefetch/cache/alignment, pin phases | [Bus-timed PC/AT operation](../../proposals/m5-bus-timed-pcat-operation.md) then cycle-exact selected profile | They require a physical transaction contract rather than a processor table row. |
| Device command/service latency | [PC/AT device service-timing corpus](../../proposals/m5-device-service-timing-corpus.md) | Device execution is distinct from CPU instruction retirement. |
| x87 numeric execution/timing | [Broaden present x87 TODO](../../states/TODO.md) | T359 excludes coprocessor-internal numerical behavior. |
| VME/PVI virtual-8086 extensions | [VME/PVI TODO](../../states/TODO.md) | They are post-80386 CPU scope, not a timing-corpus gap. |

## Closure evidence

The S1 inventory, all S2--S6 ledgers, and the checked source owner establish a
disjoint allocation: a successful form reaches one private classifier or the
visible transfer fallback. S7 found no second elapsed-tick publisher, no
handler-local timing assignment, and no unclassified S1 dispatch family. It
therefore closes T359 as a source-accounted instruction-retirement program and
hands only the exact classes above to T360 and later M5 candidates. It does
not close L3 timing; the Queue's bus-timed, service-timing, cycle-exact, and
M5 L3 closure candidates remain required.
