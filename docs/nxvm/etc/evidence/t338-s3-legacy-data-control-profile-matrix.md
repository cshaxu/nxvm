# T338 S3: 8086/80186 Data, String, Stack, Control, And I/O Matrix

## Scope And Mechanism Sweep

This record is the acceptance matrix for the S1-allocated T338 S3 boundary.
It does not turn an existing 80386 smoke into low-profile evidence: every row
below names the owner that executes its default 16-bit form with both the 8086
and 80186 CPU profiles, or the profile-specific rejection owner.

The production-path sweep covers primary dispatch, operand decode and
publication (`_m_read_rm`, `_m_write_rm`), string iteration, `_e_push`,
`_e_pop`, `_e_call_near`, `_e_call_far`, `_e_ret_near`, `_e_ret_far`,
`_e_iret`, `_ser_int_real`, `_p_input`, `_p_output`, `ExecFinal`, and the
shared T328 legacy `LOCK` route. Genuine protected selector, gate, IOPL/TSS,
VM86, paging, and 80386 width paths are transfers, not claimed here.

## Form/Owner Matrix

| S3 form family | 8086/80186 owner proof | Required boundary |
| --- | --- | --- |
| `88`--`8B`, `B0`--`BF`, `C6/C7` | `core-machine-gpr-mov-smoke` | Byte/high-byte and word register/memory/immediate publication, DS/SS selection, invalid extensions, and legacy attribute rejection. |
| `A0`--`A3` | `core-machine-moffs-smoke` | All four read/write forms, accumulator partial publication, segment selection, and source/destination nonpublication. |
| `86/87`, `90`--`97` | `core-machine-xchg-smoke` | Register/memory exchange, accumulator forms including NOP, state preservation, and T328 linkage. |
| `06/07`, `0E`, `16/17`, `1E/1F`, `8C/8E`, `8D`, `8F /0`, `C4/C5` | `core-machine-legacy-sreg-stack-smoke`, `core-machine-sreg-mov-smoke`, `core-machine-lea-smoke`, and `core-machine-les-lds-s41-smoke` | Real selector/image, POP SS inhibition, effective address, memory-only far load, and protected selector transfers kept outside S3. |
| `26/2E/36/3E`, `F2/F3`, `A4`--`A7`, `AA`--`AF` | `core-machine-movs-smoke`, `core-machine-stos-smoke`, `core-machine-lods-smoke`, `core-machine-scas-smoke`, and `core-machine-cmps-smoke` | Default byte/word forms, DF, REP/REPE/REPNE count/restart, source/destination segments, memory publication, and low-profile `66/67` rejection. |
| `50`--`5F`, `9C/9D`, `FF /6` | `core-machine-gpr-push-pop-smoke` and `core-machine-pushf-popf-s47-smoke` | 8086 PUSH SP decremented value versus 80186 original SP, stack image, POP timing, and flags image; protected stack policy remains outside. |
| `9A`, `C2/C3`, `CA/CB`, `E8/E9`, `EA/EB`, `FF /2`--`/5`, `FF /7` | `core-machine-control-transfer-smoke` | Direct/indirect and near/far real transfer, return stack image, source target selection, `FF /7` `#UD` full nonpublication, and low-profile 16-bit execution. |
| `CC/CD/CE/CF`, `F4` | `core-machine-software-int-s50-smoke`, `core-machine-iret-s51-smoke`, and `core-machine-hlt-s49-smoke` | IVT frame/IP/FLAGS, INTO condition, IRET frame consumption, halt/wake, and real delivery ordering. |
| `E4`--`E7`, `EC`--`EF` | `core-machine-port-io-s55-smoke` | Immediate/DX port selection, AL/AX publication, callback errors, no side effect on rejection, and no IRQ shadow. |

## Profile And Prefix Boundary

All rows above use the existing T328 definition for `F0`: on valid 8086/80186
following forms it is the bus prefix rather than the 80386 whitelist; it never
makes an invalid opcode or invalid ModRM form valid. `66` and `67` are rejected
on the two legacy profiles before source, destination, stack, or I/O
publication. `6C`--`6F`, `60`--`62`, `68`--`6B`, `C0/C1`, and `C8/C9` are
80186-only allocations owned by T338 S4, not omissions from this table.

## S3-Specific Control Completion

The inherited control-transfer owner formerly used a real-mode fixture fixed
at 80386. S3 parameterizes its pure 16-bit real fixture and executes direct
and indirect far transfers on 8086 and 80186. It additionally proves near
`CALL`/`RET`, `RET iw`, near/short `JMP`, `FF /2`, `FF /4`, and the reserved
`FF /7` path on both profiles. The `FF /7` probe preflights terminal real
`#UD` and compares the complete CPU state before and after execution.

No production defect was reproduced in that sweep. The 80386-only attribute
and protected-form vectors remain separately owned and are not repurposed as
low-profile proof.

## Verification

The 19 owner targets named by this matrix passed as an exact current-gate
selection. The complete current-gate suite passed 218/218 tests. Documentation
governance and `git diff --check` also passed. Rebuilding the current developer
artifact made no source-artifact change: `vm-0-5-0338` remains
`E9626E829FE8F9A1BE7A25219D48295D704C5831F64C4D2D50709671CB144F13`.
