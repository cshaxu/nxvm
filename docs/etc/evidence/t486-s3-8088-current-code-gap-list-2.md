# T486 S3 8088 Current Code Gap List 2

`M5:T486:S3:8088-CURRENT-CODE-GAP-LIST-2:ACCEPTED`

This ledger consumes every stable key and form grammar in T486 S2 List 1.
`cpu_timing.c` has exactly one successful-8088 selection chain: string/I-O,
primary, control/stack, then compatibility. `cpu_instructions.c` remains the
sole decoder/executor. A compatibility result is not a Manual-L3 result.

| List-1 key set | Current sole path and focused proof | Complete List-2 disposition |
| --- | --- | --- |
| AAA, AAD, AAM, AAS; ADC, ADD, AND, OR, SBB, SUB, XOR; CMP, TEST; INC, DEC, NOT, NEG; CBW, CWD; DAA, DAS; XCHG | Primary shape classifier and `core_machine_primary_source_instruction_cost()` classify their non-prefix source forms, including EA and 8088 word-transfer terms. The retirement smoke proves representative register/memory ADD and scalar forms. | Present Manual-L3 owner. Add the full source-form matrix in the implementation batch; do not create a per-opcode evaluator. |
| MOV ordinary register/r/m/immediate/moffs forms; LEA | The same primary shape classifier supplies the documented base, EA and transfer terms. | Present Manual-L3 owner. Add the complete matrix only; no new MOV path. |
| MOVS, MOVSB/MOVSW, CMPS, STOS, LODS, SCAS and their legal REP forms; IN and OUT | `core_machine_string_io_source_instruction_cost()` uses the complete 8088 transfer plan. The retirement smoke covers byte/word strings, segment MOVSW, repeated MOVSW and all immediate/DX IN/OUT widths. | Present Manual-L3 owner for every defined string/repeat and scalar port-I/O row. Controller service duration is deliberately outside CPU timing. |
| CALL, JMP, RET, PUSH, POP, PUSHF, POPF; Jcc aliases, JCXZ, LOOP, LOOPE, LOOPNE; HLT, INT, INTO, IRET | `core_machine_control_stack_source_instruction_cost()` plus the private 8088 control/stack transfer plan. The smoke covers representative stack/memory/control cases, all Jcc opcodes and both branch outcomes, loop forms, HLT and software delivery. | Present Manual-L3 owner for the listed non-segment forms. Add full form coverage to the source-to-retirement matrix; retain one control/stack owner. |
| LAHF, SAHF, NOP, CLC, CLD, CLI, CMC, STC, STD, STI | Primary's explicit 8088 scalar branch; all ten scalars have smoke coverage. | Present Manual-L3 owner and complete focused proof. |
| LDS, LES; MOV to/from segment registers; PUSH/POP segment register | Semantic execution exists, but the 8088 selector reaches neither a complete primary transfer classification nor an 8088 control/stack source form for these List-1 rows. | One primary/control-stack integration batch, selected by actual encoding and existing private transfer-plan helpers. No segment-specific parallel evaluator. |
| XLAT | Semantic execution exists. The primary evaluator selects XLAT only for 80386, not 8088. | One primary-owner form with its documented DS memory transfer. |
| ROL, ROR, RCL, RCR, SAL/SHL, SHR, SAR | Semantic Group-2 execution exists, but no 8088 selector consumes F3's count/width/memory grammar. | One primary-owner Group-2 implementation batch using decoded count and ModRM facts. |
| LOCK | Decoder records the prefix; primary/control rows may select their base form. `core_machine_cpu_timing_apply_8086_lock()` deliberately excludes 8088, so the documented two-clock prefix term is absent. | Add the one selector-level 8088 LOCK term after legal primitive selection; it must not be a separate retirement or a duplicated decoder rule. |
| SEGMENT | The primary and string/control helpers add the documented segment term only where their existing source form has a memory operand. | Present for those selected memory forms. The integration matrix must prove every legal List-1 memory primitive and reject non-applicable forms rather than timing the prefix independently. |
| REP; REPE/REPZ; REPNE/REPNZ | String-I/O owns legal string combinations. A non-string prefix reaches compatibility, as demonstrated by the `rep nop` unallocated-profile case. | Preserve legal string ownership. The implementation batch must keep non-string combinations source-unallocated; it may not assign a standalone prefix cost. |
| MUL, IMUL, DIV, IDIV | Primary recognizes the Group-3 form but intentionally returns no exact 8088 result; S2's Intel rows are ranges. | Manual ranges remain recorded source facts. Current compatibility is not Manual-L3; retain a source-unallocated/L2 boundary unless a separately admitted, source-qualified selection rule closes it. No midpoint. |
| ESC, WAIT | ESC is the x87 collaboration boundary. WAIT's `3+5n` needs the defined external wait-iteration count; no 8088 selector supplies an external-service contract. | Retain source-unallocated/L2 boundary at the existing CPU-to-x87 interface. Do not convert FPU service into CPU timing or create an x87 owner here. |
| INTR, NMI, SINGLE STEP | The manual calls these non-instructions; their delivery belongs to board/interrupt integration. | Excluded from successful-instruction timing. No compatibility, CPU tick or decoder claim. |

## One implementation boundary

The single code-changing batch is limited to the existing Core timing chain:
complete source-form regression coverage, the missing segment-register/pointer
forms, XLAT, Group-2 and the legal 8088 LOCK integration. The retained
Group-3 range and x87 boundaries must be visibly source-unallocated rather
than silently upgraded by compatibility timing. No board, VM, profile, device,
prefetch, FPU-service or second timing owner is part of this T.
