# T359 S4: Four-Profile String, Repeat, And Ordinary-I/O Timing Ledger

## Authority and boundary

This ledger uses the Intel *8086 Family User's Manual* Tables 2-20/2-21, the
Intel *iAPX 86/88/186/188 User's Manual* timing table, the Intel *80286 and
80287 Programmer's Reference Manual* Appendix B, and the Intel 80386 PRM
section 17.2.2.3 / instruction timing pages.  The rows are core clocks under
each manual's stated no-wait, no-HOLD, prefetched-instruction assumptions.
They do not allocate physical transfer, port-device service, exception or
interrupt-delivery cycles.

The executor retires one string primitive per refresh.  A source formula
`setup + iteration * count` therefore gives the count-zero retirement
`setup`, the first successful primitive `setup + iteration`, and every
continued primitive `iteration`.  A comparison that stops after `N`
successful primitives publishes `setup + N * iteration`; a failed primitive
does not publish a timing row.

## Four-profile rows

| Form | 8086 primitive / REP | 80186 primitive / REP | 80286 primitive / REP | 80386 primitive / REP |
| --- | --- | --- | --- | --- |
| `MOVS` | 18 / `9 + 17*n` | 14 / `8 + 8*n` | 5 / `5 + 4*n` | 7 / `5 + 4*n` |
| `CMPS` (`REPE`/`REPNE`) | 22 / `9 + 22*n` | 22 / `5 + 22*n` | 9 / `5 + 9*n` | 9 / `5 + 9*n` |
| `STOS` | 11 / `9 + 10*n` | 10 / `6 + 9*n` | 3 / `4 + 3*n` | 4 / `5 + 5*n` |
| `LODS` | 12 / `9 + 13*n` | 12 / `6 + 11*n` | 5 / transferred below | 5 / `5 + 6*n` |
| `SCAS` (`REPE`/`REPNE`) | 15 / `9 + 15*n` | 15 / `5 + 15*n` | 8 / `5 + 8*n` | 7 / `5 + 8*n` |
| `INS` | unavailable | 14 / `8 + 8*n` | 5 / `5 + 4*n` | real 15 / `13 + 6*n`; protected 9 / `7 + 6*n`; permission-map or VM86 29 / `27 + 6*n` |
| `OUTS` | unavailable | 14 / `8 + 8*n` | 5 / `5 + 4*n` | real 12 / `12 + 5*n`; protected 6 / `6 + 5*n`; permission-map or VM86 26 / `26 + 5*n` |

The `w`/operand-size forms select the same cited string row.  On 80386,
address-size selects the count and index width without creating a second clock
row.  Source segment overrides select only the source operand of `MOVS`,
`CMPS`, `LODS`, and `OUTS`; `ES` remains fixed for destination operands.
Defined `DF` and comparison-stop outcomes alter progress, not the selected
row.  The implementation stores the opcode, repeat prefix, operand/address
size, and `CS:EIP` in its private repeat signature; a reset or a different
instruction cannot reuse a previous iteration's continuation charge.

## Ordinary port I/O

The same S4 classifier owns successful ordinary `E4`--`E7` and `EC`--`EF`
rows.  It retains the accepted T357 rows: 8086 `IN imm/DX = 10/8`,
`OUT imm/DX = 10/8`; 80186 `10/8`, `9/7`; 80286 `5/5`, `3/3`; and 80386
real `12/13`, `10/11`, protected `6/7`, `4/5`, permission-map or VM86
`26/27`, `24/25`.  The 80386 operand/address attributes select the defined
width/address semantics while retaining the corresponding source row.

## Defined transfers

`REP LODS` has no exact 80286 Appendix-B formula in the selected primary
source table.  It remains a visible unallocated successful-retirement
transfer to [T360 source reconciliation](../../history/M5-T360-four-profile-timing-source-reconciliation-proposal.md);
this S does not manufacture a value.  The same transfer applies to an
architecturally accepted but source-undefined repeat-prefix combination.

Provider failures, denied I/O, segment/page/limit faults, delivered
exceptions, physical memory or I/O waits, port service latency, HOLD/DMA,
prefetch, cache, and pin phases publish no S4 successful-retirement row and
remain owned by the later physical/device/cycle-exact Queue receivers.

## Mechanism and proof

`core_machine_string_io_source_instruction_cost()` is consulted before every
other selected source classifier and before the legacy one-tick receiver.
It maps all selected string and ordinary-I/O opcodes to one private
primitive/repeat/provider disposition.  `core_machine_source_timing_repeat_string()`
is the only S4 repeat state publisher.  Legacy tables remain only as the
receiver for non-S4 forms and the explicit transferred 80286 `REP LODS` row.

`core-machine-t359-s4-timing-smoke` proves every selected primitive row and,
for every defined four-profile REP formula, count-zero/one/multiple first and
continuation retirement.  It separately proves `REPE CMPS` early stop, the
source-undefined 80286 `REP LODS` transfer, 80386 real `REP INS`, ordinary
operand/address-attribute `IN`/`OUT`, 80386 protected, permission-map and
VM86 `INS`/`OUTS`, provider-fault zero publication, repeat reset, and the
80186 source-maximum preflight boundary.  The retained string, port-I/O,
protected-I/O and four-profile timing-ledger smoke owners retain their
independent semantic, fault/nonpublication, provider, segment, `DF`, restart
and reset proof.
