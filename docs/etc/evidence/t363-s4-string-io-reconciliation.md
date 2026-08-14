# T363 S4: string, repeat, and ordinary-I/O reconciliation

## Result

S4 finds the complete selected string, repeat, and ordinary port-I/O mechanism
already source-backed by the accepted T359 S4 ledger; no runtime value or
classifier change is warranted. `core_machine_string_io_source_instruction_cost()`
classifies every selected string or ordinary-I/O opcode before every other
source classifier and before the legacy receiver.

`core_machine_source_timing_repeat_string()` is the sole repeat-state
publisher. It retains a signature of `CS:EIP`, opcode, REP prefix, and
operand/address-size attributes; a changed instruction or reset cannot reuse
the continuation rate. Its count-zero, first-primitive, and continuation
publication implements the accepted setup-plus-iteration accounting without
creating a handler-local clock.

| Form partition | Owner/disposition |
| --- | --- |
| `MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, `INS`, `OUTS` primitives | Profile-local Intel rows in the private repeat ledger, including the 80386 real/protected/permission-map-or-VM86 port-string split. |
| Defined `REP`/`REPE`/`REPNE` rows | One repeat publisher selects count-zero, first, continuation and comparison-stop timing; direction and source-segment attributes change execution state, not the selected row. |
| Ordinary `E4`--`E7` and `EC`--`EF` I/O | The same classifier selects retained 8086/80186/80286 rows and 80386 real/protected/permission-map-or-VM86 rows before legacy fallback. |
| `66`/`67`, source override and legal repeat prefix | Operand/address width and applicable source operand semantics retain their cited row; unsupported repeat combinations transfer rather than publish a fabricated value. |
| 80286 `REP LODS`, denied/provider fault, segment/page/limit fault, exception delivery, port service and physical waits | Explicit T360/physical/device/cycle-exact receivers with zero successful-retirement publication; S4 does not infer a row. |

## Verification

On 2026-08-14, `core-machine-t359-s4-timing-smoke` passed. It covers every
primitive profile row, defined count-zero/one/multiple repeat formulas,
comparison early stop, 80286 `REP LODS` transfer, 80386 real/protected/
permission-map/VM86 `INS` and `OUTS`, ordinary operand/address attributes,
provider-fault nonpublication, repeat reset, and the 80186 preflight maximum.

The static sweep searched the selected string and I/O opcode families,
repeat-state helper, source classifier, classifier-order call site, focused
smoke, and T359 inventory gate. It found one selected source owner and no
handler-local elapsed tick or second publisher. `cmake --build --preset
current-gates-gcc` completed successfully, and the direct current-gate label
run passed 246/246 tests. No runnable path changed, so S4 creates no T363
developer artifact. S5 receives only 80386 secondary integer and
prefix/width timing forms.
