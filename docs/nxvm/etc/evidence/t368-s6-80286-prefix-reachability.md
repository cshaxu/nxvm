# T368 S6: 80286 Legal-Prefix Reachability

## Scope And Authority

S6 audits successful 80286 instructions whose decoded lexical prefix count is
nonzero.  The primary instruction source is Intel, *80286 and 80287
Programmer's Reference Manual*, order 210498-005 (1987), Appendix B.  It gives
the selected string/REP and ordinary instruction rows, but it does not supply
an independent scalar adjustment for every legal lexical prefix context.  The
Intel *80286 Hardware Reference Manual*, order 210760-002 (1987), supplies the
separate LOCK/bus-ownership behavior.  Neither source permits treating a
prefix byte as a universal CPU-retirement surcharge.

## Reachability And Disposition

| Prefix class | Successful 80286 disposition | Timing owner/result |
| --- | --- | --- |
| `F2`/`F3` on defined strings | `MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, `INS`, and `OUTS` reach the string/I-O owner. | Existing exact Appendix-B primitive/REP formulas; `REP LODS` remains its recorded source-undefined transfer. |
| Segment override on defined string source operands | The override selects the documented source segment; destination ES remains fixed. | Existing string owner selects the same admitted string row; no invented prefix surcharge. |
| Segment override on an ordinary primary/control memory form | The instruction can successfully retire after selecting a different logical segment. | The existing primary/control form owner remains responsible for its admitted row; Appendix B does not justify a second lexical-prefix scalar. |
| `F0` LOCK on an emulator-accepted instruction | Existing cross-profile LOCK regression shows successful compatibility execution, including memory RMW. | No CPU scalar is allocated. LOCK is a bus-ownership signal; READY/HOLD, DMA and physical transaction time remain later bus/device receivers. |
| `F2`/`F3` on a non-string compatibility path | The executor may retire compatibility behavior, but no selected Appendix-B timing formula distinguishes it. | One unallocated successful-prefix transfer; no reference-model substitution is admitted. |
| `64`/`65`/`66`/`67`, invalid/only-prefix streams, and delivered prefix faults | Not successful 80286 retirement. | No publication; S7 rechecks the negative boundary. |

The ordering is intentional. `core_machine_string_io_source_instruction_cost`
runs before the profile classifier, so defined string/repeat paths never reach
the generic prefix result. The primary and control/stack owners admit their
already selected segment-override shapes first. The remaining 80286
`prefixes != 0u` path therefore stays
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`: it is an explicit transfer, not a
one-tick clock or physical 8 MHz approximation.

## Execution Evidence

- `core-machine-t359-s4-timing-smoke` is the owner proof for defined 80286
  string/REP paths, including repeated retirement and the source-undefined
  `REP LODS` transfer.
- `core-machine-legacy-lock-s1-smoke` passed with
  `M5:T328:S1:LEGACY-LOCK:OK`, confirming the retained executor-level LOCK
  compatibility boundary and its delivered-fault cases.
- The focused 80286 ledger retains its pre-386 prefix fault/no-publication
  check.  No new scalar is asserted by this S.

## Retained Transfer

All successfully retired non-string REP compatibility contexts, ordinary
segment-override timing contexts not already represented by their instruction
form, and LOCK's observable transaction occupancy transfer together to the
queued bus-timed PC/AT operation and selected-profile phase work. A later task
may allocate them only with a source-backed, profile-specific transaction
model; it cannot change this CPU ledger by assigning a generic prefix charge.
S7 still owns x87/`WAIT`, final-default and negative-route closure. T368 and
IBM 5170 L3 remain open.
