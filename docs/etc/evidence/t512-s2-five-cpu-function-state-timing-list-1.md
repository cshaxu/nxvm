# T512 S2 Five-CPU Function, State And Timing List 1

`M5:T512:S2:FIVE-CPU-LIST-1:ACCEPTED`

## Finite Coverage Rule

This is List 1, not a current-code claim. Its finite instruction universe is
the Cartesian expansion of the decoder-inventory producers and the manually
defined forms below. The inventory producers are deliberately executable:
their ModR/M bitmasks are the compact exact representation of every accepted
opcode/ModR/M pair; no prose range may silently widen them.

| Profile | Exact inventory producer and current cardinality | Manual source for form/state | Timing source disposition |
| --- | --- | --- | --- |
| 8086 | `core_machine_8086_timing_manifest_runner.c`; 57,926 opcode/ModR/M candidates | Intel 1981, Tables 2-20/2-21 and instruction chapters | Table 2-21 exact/formula/range entries are Manual-L3 facts. |
| 8088 | Same base opcode set; distinct profile row | Intel 1981, Tables 2-20/2-21 | Base fact plus explicit Table 2-21 `+4` clocks per word transfer; never inferred from 8086. |
| 80186 | `core_machine_80186_decoder_inventory_runner.c`; 247 primary opcodes, 61,530 candidates | Intel 1985, Table 1-16 and instruction chapters | A stated range remains a range, not an invented exact value. |
| 80286 | `core_machine_80286_decoder_inventory_runner.c`; primary inventory plus `0F {00,01,02,03,06}` masks | Intel 1987 Programmer's and Hardware Reference Manuals | Semantics/forms are Manual-L3. Available originals contain **no per-instruction clock table**; all time fields are `SOURCE-UNAVAILABLE`. |
| 80386DX | `core_machine_80386_decoder_inventory_runner.c`; 253 primary opcodes, 63,021 candidates, and recorded `0F` masks | Intel 1990, Chapters 2-12 and 17 | Chapter 17 exact/formula/context fields are Manual-L3 facts. |

Each producer tests its accepted ModR/M masks. Its generated JSON is a test
result, not a checked-in second decoder. S3 must consume the producer, the
prefix rules below, and every manual family row exactly once.

## Form Families

An `r/m` row expands over the producer's accepted ModR/M masks and every
manual-supported operand/address-size, register/memory, segment-override,
immediate and repeat context. A source-defined form is authoritative even if
current decode admits less or more; that is an S3 gap, not a List-1 change.

| Key | Complete form family and profile introduction | Manual locator |
| --- | --- | --- |
| F01 | `AAA AAD AAM AAS DAA DAS CBW/CWDE CWD/CDQ`; accumulator conversion/adjust forms | 1981 Table 2-21; 1985 Table 1-16; 1987 dictionary; 1990 Ch. 17 |
| F02 | `ADC ADD AND OR SBB SUB XOR CMP TEST`: register/register, register/memory, memory/register, accumulator/immediate and group-immediate forms | Profile instruction table/Ch. 17 |
| F03 | `INC DEC NEG NOT XCHG MOV LEA LDS LES`, plus 80386 `LFS LGS LSS MOVSX MOVZX` and legal segment moves | Profile instruction table; protected-mode chapters |
| F04 | `PUSH POP PUSHF POPF`, calls/returns/interrupt frames, and later `PUSHA POPA PUSH imm ENTER LEAVE`, including all operand-size variants | 1981 Table 2-21; 1985 Table 1-16; 1987 protected-mode chapters; 1990 Chs. 6, 9, 17 |
| F05 | Near/far `CALL JMP RET IRET`, all short conditional branches, `JCXZ/JECXZ`, `LOOP/LOOPE/LOOPNE`, 80386 near `0F 80-8F` branches and `SETcc` | Profile control-transfer chapters/Ch. 17 |
| F06 | `MUL IMUL DIV IDIV`; later immediate/multi-operand `IMUL`; 80386 `BSF BSR BT BTC BTR BTS SHLD SHRD` | Profile instruction table/Ch. 17 |
| F07 | Group-2 shifts/rotates: one, `CL`, and from 80186 immediate count; all legal r/m and operand-size forms | Profile instruction table/Ch. 17 |
| F08 | `MOVS CMPS STOS LODS SCAS`, and from 80186 `INS OUTS`; byte/word/dword and each legal `REP/REPE/REPNE` form | Profile instruction table/Ch. 17 |
| F09 | `IN OUT` immediate/DX forms, `WAIT`, `ESC`, `HLT`, `NOP`, `XLAT/XLATB` | Profile instruction table/Ch. 17; device completion/BUSY duration is not an instruction-time fact. |
| F10 | Flag controls/transfers: `CLC CLD CLI CMC STC STD STI LAHF SAHF`; F04 owns `PUSHF/POPF` image/load | Manual FLAGS and instruction descriptions |
| F11 | 80186 additions: `PUSHA POPA BOUND PUSH imm IMUL imm ENTER LEAVE INS OUTS` | Intel 1985 Table 1-16 and per-instruction descriptions |
| F12 | 80286 protected forms: `ARPL LAR LSL VERR VERW`, descriptor-table, task-register and machine-status instructions; protected segment/control transfer, gates, tasks and privilege paths | Intel 1987 Programmer's Reference Manual, Chs. 3-8 and Appendix-B form dictionary |
| F13 | 80386 system forms: `0F 20-26` control/debug/test moves; `CLTS`; 32-bit descriptors/tables, paging, VM86, bit operations and 32-bit operand/address forms | Intel 1990 Chs. 2-12, 17 |
| F14 | x87 escape bytes `D8-DF` are CPU-issued escape forms only. Coprocessor operation is an FPU task and does not turn an unavailable FPU operation into CPU `#UD`. | 1981 ESC definition; later ESC/WAIT material |

The base 8086/8088 set is F01-F10 and F14. 80186 adds F11, 80286 adds F12,
and 80386DX adds F13 and F03/F05/F06/F07/F08 size extensions. `POP CS (0F)`
is 8086-only; from 80286 onward `0F` is escaped opcode space.

## Prefix, Operand And Fault Matrix

| Context | Manual List-1 rule |
| --- | --- |
| Segment override | Operand-selection fact, not separate retirement; legal only for a manual-valid memory operand. 80386 also has FS/GS. |
| `REP/REPE/REPNE` | Modifies only a manual-valid string form and its condition/iteration count. `INS/OUTS` start at 80186. |
| `LOCK` | A bus-lock prefix, distinct from operation semantics, applies only to a manual-allowed locked memory RMW form. The 8086 source boundary differs from later restrictions and requires its own S3 reconciliation. |
| `66/67/64/65` | Operand/address-size and FS/GS prefixes are 80386DX-only. Their absence on older profiles is source-defined. |
| Undefined encoding | No successful form: use the profile's source-defined `#UD` path. |
| Operand, segment, privilege, page or divide failure | Not successful retirement. It retains a manual-described fault/abort/restart context and S3 owner disposition. |

## Architectural State And Delivery Matrix

| State/delivery row | Required source result |
| --- | --- |
| General, segment, IP and stack state | A successful F01-F14 form changes only named architectural state; memory, descriptor checks and commit ordering belong to the form context. |
| FLAGS defined bits | `CF PF AF ZF SF TF IF DF OF` are defined only where the manual says. Reserved/undefined bits receive no fabricated readback. |
| FLAGS image versus load | `PUSHF`/interrupt delivery publish an image; `POPF`/`IRET` load defined fields subject to mode/privilege. 8086/8088/80186 do not acquire later IOPL/NT behavior. |
| Real mode delivery | `INT`, `INTO`, software exceptions, `INTR/NMI`, stack frame and `IRET` follow each profile's vector/frame/IF/TF ordering. |
| 80286 protected delivery | Descriptor validation, privilege stack/gate/task paths, restart and shutdown are F04/F05/F12 contexts, never profile-side emulation. |
| 80386 protected/VM86/paging delivery | Paging, privilege, debug, task and VM86 are F04/F05/F12/F13 contexts. Fault/trap/abort retains documented restart/return behavior. |
| Asynchronous interrupt/NMI | Delivery rows, not ordinary opcodes; audit their acceptance point and frame state separately from retirement. |

## Timing Classification

`Manual-L3` means only that the original manual supplies a number, formula,
bounded range, or stated unavailable condition. It is not a claim current Core
realizes that value. Exact arithmetic from a manual formula stays Manual-L3; a
manual range stays a range. Prefetch, READY, HOLD, arbitration, device response
and physical board cycles remain outside CPU instruction fields unless an
original source explicitly composes them.

The S1 corpus proves that the available 80286 originals contain no execution
time table. Every F01-F14 80286 time field is therefore `SOURCE-UNAVAILABLE`,
with receiver **T512 S3 source/code disposition**. It must not be relabelled
Manual-L3 from `210498-005` Appendix B; an emulator literal would be Other-L2
at most.

## S3 Consumption Predicate

S3 may accept a row only when it maps one inventory form/context through one
Core decode, state-construction, commit/rollback, delivery, retirement and
regression owner. It must classify every expansion `conforming`, `missing`,
`conflicting`, `unsupported`, or `source-unavailable`. A group heading, smoke,
or previous timing ledger is not a disposition.
