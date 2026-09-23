# T368 S5: 80286 System-Timing Reconciliation

## Scope And Authority

S5 resolves only successful, unprefixed 80286 retirement for `LLDT`, `LTR`,
`SGDT`, `SIDT`, `LGDT`, `LIDT`, and `CLTS`.  The primary authority is Intel,
*80286 and 80287 Programmer's Reference Manual*, order 210498-005 (1987),
Appendix B, [instruction timings](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf).
It supplies exact scalar rows: `LLDT` and `LTR` register `17`, memory `19`;
`SGDT` and `LGDT` memory `11`; `SIDT` and `LIDT` memory `12`; and `CLTS` `2`.
No reference emulator was needed or used as timing authority.

## Reconciliation

| Form | Successful execution context proved | Appendix-B ticks | Publisher gate |
| --- | --- | ---: | --- |
| `LLDT r16`, `LTR r16` | Protected mode, CPL 0, valid LDT/TSS selector | 17 | `0F 00 /2,/3`, non-memory, protected CPL 0 |
| `LLDT m16`, `LTR m16` | Protected mode, CPL 0, valid selector read from DS-based memory | 19 | `0F 00 /2,/3`, memory, protected CPL 0 |
| `SGDT m`, `LGDT m` | Real mode successful memory store/load | 11 | `0F 01 /0,/2`, memory, real mode or protected CPL 0 |
| `SIDT m`, `LIDT m` | Real mode successful memory store/load | 12 | `0F 01 /1,/3`, memory, real mode or protected CPL 0 |
| `CLTS` | Real mode, TS initially set | 2 | `0F 06`, real mode or protected CPL 0 |

The classifier remains after the instruction-state refresh and before the sole
elapsed-tick publication.  It observes the recorded pre-instruction mode/CPL
and decoded memory flag, so only the listed successful contexts receive an
Appendix-B value.  Privilege failures, descriptor failures, task delivery,
prefixes, non-memory table forms, and all bus/device/physical-time effects are
not allocated here.

The ledger regression uses one fresh protected fixture per `LLDT/LTR` form so
`LTR`'s busy-TSS state cannot affect another row.  Its isolated GDT provides a
present LDT descriptor at selector `28h` and available TSS at `30h`; it proves
the selector state after the one timed instruction.  The table/control rows
also prove their post-instruction GDTR, IDTR, or TS state.

## Verification And Sweep

- `core-machine-80286-instruction-timing-ledger-smoke` passed with marker
  `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`.
- Existing semantic owners passed: `core-machine-segment-selector-smoke`
  (`M5:T301:SEGMENT-SELECTOR:OK`), `core-machine-lgdt-lidt-smoke`
  (`M5:T319:S1:LGDT-LIDT:OK`), and `core-machine-clts-s62-smoke`
  (`M5:T316:S62:CLTS:OK`).  Their expected negative diagnostic output remains
  part of those regressions.
- The S4 residual sweep searched `INS_0F_00`, `INS_0F_01`, `INS_0F_06`, the
  80286 classifier and table/selector control regressions.  These seven
  successful source rows are the S5 receiver; no other `0F` family is silently
  allocated.
- The static sole-publisher verifier and documentation/diff checks are required
  for acceptance.  The unrelated platform request-bridge compile break still
  prevents a full-gate claim.

## Retained Boundary

S6 owns legal-prefix reachability and S7 owns x87/`WAIT`, default fallback and
negative-route reconciliation.  T368 remains a CPU retirement ledger task;
it does not close IBM 5170 L3, bus availability, device service timing, or
physical cycle accuracy.
