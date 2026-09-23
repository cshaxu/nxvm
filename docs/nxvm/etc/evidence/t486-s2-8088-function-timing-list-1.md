# T486 S2 8088 Function And Timing List 1

`M5:T486:S2:8088-FUNCTION-TIMING-LIST-1:ACCEPTED`

This finite ledger covers the whole Table 2-21 surface. `p` is the printed
page of Intel's 1981 *iAPX 86,88 User's Manual*. `EA` is Table 2-20's exact
effective-address term. `T` is the table's word-transfer count. Every
successful table row has 8088 source timing `base + 4*T`; a dash has `T=0`.
This is Manual-L3 source data only, never a prefetch, bus-cycle or wall-clock
claim.

## Shared operand-row grammars

| ID | Complete source row set |
| --- | --- |
| F1 | ADC, ADD, AND, OR, SBB, SUB and XOR: `r,r=3/-`; `r,m=9+EA/1`; `m,r=16+EA/2`; `r,imm=4/-`; `m,imm=17+EA/2`; `acc,imm=4/-`. |
| F2 | CMP: `r,r=3/-`; `r,m=9+EA/1`; `m,r=9+EA/1`; `r,imm=4/-`; `m,imm=10+EA/1`; `acc,imm=4/-`. TEST: `r,r=3/-`; `r,m=9+EA/1`; `acc,imm=4/-`; `r,imm=5/-`; `m,imm=11+EA/1`. |
| F3 | ROL/ROR/RCL/RCR/SAL-SHL/SHR/SAR: `r,1=2/-`; `r,CL=8+4/bit/-`; `m,1=15+EA/2`; `m,CL=20+EA+4/bit/2`. |
| F4 | MUL: `70-77/-`, `118-133/-`, `(76-83)+EA/1`, `(124-149)+EA/1`; IMUL: `80-98/-`, `128-154/-`, `(86-104)+EA/1`, `(134-160)+EA/1`; DIV: `80-90/-`, `144-162/-`, `(86-96)+EA/1`, `(150-168)+EA/1`; IDIV: `101-112/-`, `165-184/-`, `(107-118)+EA/1`, `(171-190)+EA/1`. The four forms are r8, r16, m8, m16. |
| F5 | Conditional branches JA/JNBE through JS: `short-label=16 or 4/-` (taken or not-taken). JCXZ is `18 or 6/-`. |
| F6 | CMPS `22/2`, repeat `9+22/rep,2/rep`; LODS `12/1`, repeat `9+13/rep,1/rep`; MOVS `18/2`, repeat `9+17/rep,2/rep`; SCAS `15/1`, repeat `9+15/rep,1/rep`; STOS `11/1`, repeat `9+10/rep,1/rep`. |

## Every Table 2-21 mnemonic entry

The left column is the stable entry key: Intel's printed mnemonic, including
the printed alias pairs. A row containing several keys applies the stated
complete form grammar independently to every named key.

| Stable mnemonic keys | Entries, function, source row and page |
| --- | --- |
| AAA; AAD; AAM; AAS | `4/-`; `60/-`; `83/-`; `4/-`: ASCII adjust addition/division/multiply/subtraction; no operands; p. 2-51. |
| ADC; ADD; AND; OR; SBB; SUB; XOR | Destination/source arithmetic or logical operations; F1; pp. 2-52, 2-61, 2-62, 2-66 through 2-68. |
| CALL; CBW; CLC; CLD; CLI; CMC; CMP | CALL: near `19/1`, far `28/2`, memptr16 `21+EA/2`, regptr16 `16/1`, memptr32 `37+EA/4`; CBW `2/-`; CLC/CLD/CLI/CMC each `2/-`; CMP F2; pp. 2-52 through 2-53. |
| CMPS; CWD; DAA; DAS; DEC; DIV; ESC | CMPS F6; CWD `5/-`; DAA/DAS each `4/-`; DEC reg16 `2/-`, reg8 `3/-`, memory `15+EA/2`; DIV F4; ESC `imm,m=8+EA/1`, `imm,r=2/-`; pp. 2-53 through 2-54. |
| HLT; IDIV; IMUL; IN; INC; INT; INTO; IRET; MUL | HLT `2/-`; IDIV/IMUL/MUL F4; IN immediate `10/1`, DX `8/1`; INC reg16 `2/-`, reg8 `3/-`, memory `15+EA/2`; INT type3 `52/5`, other `51/5`; INTO `53 or 4/5`; IRET `24/3`; pp. 2-55 through 2-56 and 2-61. |
| JA/JNBE; JAE/JNB; JB/JNAE; JBE/JNA; JC; JE/JZ; JG/JNLE; JGE/JNL; JL/JNGE; JLE/JNG; JNC; JNE/JNZ; JNO; JNP/JPO; JNS; JO; JP/JPE; JS; JCXZ; JMP | All conditional keys F5; JCXZ `18 or 6/-`; JMP short `15/-`, near `15/-`, far `15/-`, memptr16 `18+EA/1`, regptr16 `11/-`, memptr32 `24+EA/2`; pp. 2-56 through 2-59. |
| LAHF; LDS; LEA; LES; LODS; LOOP; LOOPE/LOOPZ; LOOPNE/LOOPNZ; MOV; MOVS; MOVSB/MOVSW | LAHF `4/-`; LDS/LES `reg16,memptr32=16+EA/2`; LEA `reg16,mem16=2+EA/-`; LODS F6; LOOP `17 or 5/-`, LOOPE `18 or 6/-`, LOOPNE `19 or 5/-`; MOV's eleven printed forms are listed above; MOVS and the separately printed MOVSB/MOVSW F6; pp. 2-59 through 2-61. |
| NOP; NOT; OUT; POP; POPF; PUSH; PUSHF; RCL; RCR; REP | NOP `3/-`; NOT r `3/-`, m `16+EA/2`; OUT immediate `10/1`, DX `8/1`; POP r `8/1`, sreg `8/1`, m `17+EA/2`; POPF `8/1`; PUSH r `11/1`, sreg `10/1`, m `16+EA/2`; PUSHF `10/1`; RCL/RCR F3; REP `2/-`; pp. 2-62 through 2-63. |
| REPE/REPZ; REPNE/REPNZ; RET; ROL; ROR; SAHF; SAL/SHL; SAR; SCAS; SHR; STC; STD; STI; STOS; TEST; WAIT; XCHG; XLAT | REPE/REPZ and REPNE/REPNZ each `2/-`; RET `8/1`, `12/1`, `18/2`, `17/2`; ROL/ROR/SAL-SHL/SAR/SHR F3; SAHF `4/-`; SCAS/STOS F6; STC/STD/STI each `2/-`; TEST F2; WAIT `3+5n/-`; XCHG acc,r16 `3/-`, m,r `17+EA/2`, r,r `4/-`; XLAT `11/1`; pp. 2-64 through 2-67. |

## Prefix and explicitly non-instruction table entries

| ID | Disposition |
| --- | --- |
| P01 | LOCK: `2/-`, p. 2-60. Prefix for the following valid instruction, not an independent retirement; later memory-RMW-only restrictions must not be projected back onto 8086/8088. |
| P02 | SEGMENT: `2/-`, p. 2-65. Prefix; manual says its assembler representation is incorporated in the operand, not a separate instruction. |
| N01 | INTR external maskable interrupt: `61/7`, p. 2-56; manual explicitly says it is not an instruction. |
| N02 | NMI external nonmaskable interrupt: `50/5`, p. 2-60; explicitly non-instruction. |
| N03 | SINGLE STEP trap interrupt: `50/5`, p. 2-66; explicitly non-instruction. |

The printed mnemonic keys, two prefix keys and three non-instruction keys
exhaust Table 2-21. Manual ranges and external terms remain exact Manual-L3
source facts; no midpoint, estimate or L2 conversion is allowed. List 2 must
next map every key to one present owner, test and gap disposition.
