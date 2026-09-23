# T338 S1: 8086/80186 Form And LOCK Allocation

## Authority and method

Intel 8086/80186 instruction availability is the form authority. The current
metadata function and primary dispatch table are the mechanical repository
inventory, not a substitute for Intel behavior. The audit searched
`core_machine_cpu_instruction_metadata_get`, the primary `insTable` setup,
the grouped opcode handlers, `PREFIX_LOCK`, `UndefinedOpcode`, all
`tests/machine` owners, T316's ordinary-execution matrix, and T328's retained
legacy-LOCK evidence.

The shared legacy LOCK rule is already closed by T328: 8086 and 80186 treat
`F0` as a bus prefix of a valid following instruction. It has no 80386 opcode
whitelist. Therefore the table below names the relevant memory-capable forms
and points to one `PREFIX_LOCK` owner; it does not falsely create a second
per-handler LOCK policy. Invalid/reserved primary forms still produce `#UD`
through their normal profile/decode route.

## Complete primary-form allocation

| Encodings and mechanism owner | 8086 disposition | 80186 disposition | Existing evidence and T338 owner |
| --- | --- | --- | --- |
| `00`--`05`, `08`--`0D`, `10`--`15`, `18`--`1D`, `20`--`25`, `28`--`2D`, `30`--`35`, `38`--`3D`: ADD/OR/ADC/SBB/AND/SUB/XOR/CMP register-memory, accumulator-immediate forms | In scope; defined arithmetic FLAGS, memory publication, source/destination alias, and fault ordering require complete form proof. | Same inherited forms. | Existing T316 S2--S20 and T322 evidence is bounded. **T338 S2 ALU/FLAGS/condition family** owns all form/ModRM/defined-FLAGS closure. |
| `06/07`, `0E`, `16/17`, `1E/1F`: ES/CS/SS/DS stack transfers | In scope with 8086 stack-image quirks and POP-SS inhibition. | Same inherited forms. | Existing T316 S46 bounded proof. **T338 S3 data/stack/control family** owns the complete legacy form matrix; protected selector rules transfer to T339. |
| `26/2E/36/3E`, `F2/F3`: segment and REP prefixes | In scope, including repeated-prefix order and string-following semantics. | Same inherited forms. | T316 S64 proves semantic classes. **T338 S3** owns complete legacy-form allocation; shared prefix scan remains unchanged unless a reproduced common defect requires a sweep. |
| `27/2F/37/3F`: DAA/DAS/AAA/AAS | In scope; decimal/ASCII adjustment result and defined FLAGS require full proof. | Same inherited forms. | No complete profile-family proof. **T338 S2** owns. |
| `40`--`4F`: INC/DEC r16 | In scope; CF preservation and defined arithmetic FLAGS. | Same inherited forms. | T316 S40 and prior ordinary evidence are bounded. **T338 S2** owns. |
| `50`--`5F`: PUSH/POP r16 | In scope; 8086 PUSH SP versus 80186 original-SP distinction, POP destination timing, stack faults and publication. | In scope; original-SP semantics. | T316 S44 found/fixed both shared handler variants. **T338 S3** owns profile form closure and must retain the distinct Intel quirk. |
| `60/61`: PUSHA/POPA | `#UD` before publication. | In scope; word image/original SP and POPA skip slot. | T316 S42 fixed the 80186 gate. **T338 S4 80186 extensions** owns all accepted/rejected form rows. |
| `62`: BOUND r16,m16&16 | `#UD` before source access. | In scope; signed bounds and `#BR` producer/restart. | T316 S54 fixed 80186 gate, dword-pair route, and real `#BR` delivery. **T338 S4** owns exact legacy-form closure. |
| `63`, `64`--`67` | 80286/80386-only or later prefix. | Same rejection. | Transfer once to T339/T340; T338 proves only profile rejection where a legacy form has an attribute byte. |
| `68`: PUSH imm16; `69`: IMUL r16,r/m16,imm16; `6A`: PUSH sign-extended imm8; `6B`: IMUL r16,r/m16,imm8 | `#UD` before immediate/source/stack publication. | In scope. | T316 S45/S56 fixed profile gates and immediate sign/overflow behavior. **T338 S4** owns full 80186 profile/form closure. |
| `6C`--`6F`: INS/OUTS byte/word string I/O | `#UD` before I/O/memory effects. | In scope with REP/DF and port callback ordering. | T316 S38 provides bounded form proof. **T338 S4** owns full legacy acceptance/rejection allocation; protected I/O permissions transfer to T339/T340. |
| `70`--`7F`: Jcc rel8 | In scope; all sixteen condition predicates, taken/not-taken IP, defined FLAGS preservation, and fetch-fault restart. | Same inherited forms. | Existing control evidence is bounded. **T338 S2** owns. |
| `80/81/83`: Group 1 immediate ALU r/m8/r/m16 | In scope; `/0`--`/7`, sign-extended `83`, defined FLAGS and memory commit/fault ordering. | Same inherited forms. | Existing S2--S20 examples are bounded. **T338 S2** owns. |
| `82`, `D6`, `F1` | Intel-reserved/invalid primary form; `#UD` is the sole legacy disposition. | Same. | Existing metadata/dispatch rejects. **T338 S2** records complete rejection probes; no handler implementation. |
| `84/85`: TEST; `86/87`: XCHG | In scope; TEST defined FLAGS/no destination; XCHG memory ordering/publication. | Same inherited forms. | T316 existing smokes bounded. **T338 S2** owns TEST and **S3** owns XCHG memory/LOCK relation. |
| `88`--`8B`, `B0`--`BF`, `C6/C7`: MOV GPR/immediate/memory | In scope, including byte/high-byte and word publication and segment/EA selection. | Same inherited forms. | T316 S31 proves a bounded ordinary matrix. **T338 S3** owns legacy complete-form reconciliation; 80386 width/address stays T340. |
| `8C/8E`: MOV Sreg; `8D`: LEA; `8F /0`: POP r/m16 | Legacy ES/CS/SS/DS transfer and LEA are in scope; selector protection is not. | Same inherited forms. | T316 S32/S46 and T302 proof is bounded. **T338 S3** owns real legacy behavior; protected selector validation transfers to T339. |
| `90`--`97`, `98/99`, `9C/9D`, `9E/9F` | NOP/XCHG AX, CBW/CWD, PUSHF/POPF, LAHF/SAHF in scope with defined image/FLAGS. | Same inherited forms. | T316 S39/S47 and existing conversion evidence bounded. **T338 S2** owns FLAGS/conversion; **S3** owns stack-image closure. |
| `9A`, `C2/C3`, `CA/CB`, `E8`--`EB`: near/far CALL/JMP/RET | Real-mode in scope with 16-bit stack/return image. | Same inherited forms. | T303 and T316 control slices are bounded. **T338 S3** owns real instruction forms; protected gates/returns transfer to T339/T340. |
| `9B`, `D8`--`DF` | External coprocessor CPU-side boundary only. | Same. | T316 S65/T317 prove CPU-side WAIT/ESC; x87 execution remains external. |
| `A0`--`A3`, `A8/A9`: moffs MOV and TEST accumulator | In scope. | Same inherited forms. | T316 S30/S31 bounded proof. **T338 S3** owns MOV forms; **S2** owns TEST. |
| `A4`--`A7`, `AA`--`AF`: MOVS/CMPS/STOS/LODS/SCAS | In scope with DF, REP/REPE/REPNE, source/destination segment and restart rules. | Same inherited forms. | T316 S33--S37 are bounded individual-form slices. **T338 S3** owns 8086/80186 form matrix; no 80386 width claim. |
| `C0/C1`: Group 2 immediate-count shifts/rotates | `#UD` before operand access. | In scope; `/0`--`/7`, count semantics and defined FLAGS. | Existing rotate/shift evidence is partial. **T338 S4** owns. |
| `C4/C5`: LES/LDS | In scope in real mode, memory-only far pointer load. | Same inherited forms. | T316 S41 bounded proof. **T338 S3** owns legacy form matrix; protected descriptor semantics transfer to T339. |
| `C8/C9`: ENTER/LEAVE | `#UD` before stack effects. | In scope; nesting display and allocation/LEAVE stack-address behavior. | T316 S43 fixed 80186 guard/construction. **T338 S4** owns. |
| `CC`--`CF`: INT3/INT/INTO/IRET | In scope for real vectors, frame/saved-IP and flags. | Same inherited forms. | T316 S50/S51 and T337 prove delivery composition. **T338 S3** owns real instruction-form matrix; protected/VM86 transfers remain T339/T340. |
| `D0`--`D3`: Group 2 shifts/rotates; `D4/D5`: AAM/AAD; `D7`: XLAT | In scope, including count source, divide-like decimal exception, and DS/BX/AL selection. | Same inherited forms. | Existing rotate evidence is bounded; AAM/AAD/XLAT lack form closure. **T338 S2** owns. |
| `E0`--`E3`: LOOP/LOOPE/LOOPNE/JCXZ; `E4`--`E7`, `EC`--`EF`: ordinary IN/OUT | In scope. | Same inherited forms. | T316 S55 bounds IN/OUT; loop forms lack complete family proof. **T338 S2** owns loop predicates; **S3** owns ordinary real I/O forms. Protected IOPL/TSS rows transfer later. |
| `F0`: LOCK | Prefix itself has the shared T328 owner. | Same. | **Closed shared policy**: valid next instruction transparency, no 80386 whitelist. Each T338 S names affected memory forms but may not introduce duplicate policy. |
| `F4`--`FD`: HLT/CMC/group arithmetic/flag control | In scope except external privilege behavior. | Same. | T316 S39/S40/S49 and arithmetic evidence bounded. **T338 S2** owns defined FLAGS; **S3** owns HLT real behavior. |
| `F6/F7`: Group 3 TEST/NOT/NEG/MUL/IMUL/DIV/IDIV; `FE`: INC/DEC r/m8; `FF`: INC/DEC/CALL/JMP/PUSH groups | In scope; each ModRM extension must be allocated and prove defined FLAGS/exception/publication. | Same inherited forms. | Existing arithmetic/stack/control smokes are examples only. **T338 S2** owns arithmetic extensions (`F6/F7`, `FE`); **S3** owns control/stack `FF /2--/6`; far protected semantics transfer later. |

## Mechanism ownership and future implementation slices

| T338 continuation | Complete bounded objective | Mechanism owner and hard boundary |
| --- | --- | --- |
| S2: legacy ALU, FLAGS, condition, adjustment, shift, multiply/divide | All 8086 inherited arithmetic/condition forms listed above, plus 80186-only rejections where relevant; all ModRM extensions, defined FLAGS, exception/restart and source/destination commit proof. | Arithmetic handlers with `_a_*`, grouped-opcode decode, `_kaf_set_flags`, and `UndefinedOpcode`. Do not assert Intel-undefined FLAGS or modify stack/control/descriptor paths. |
| S3: legacy data, strings, stack, control, and ordinary I/O | All 8086 data movement, strings, stack, real control/interrupt, and ordinary port-I/O forms; exact 8086 quirks and 80186 inherited acceptance. | Operand/memory/string/stack/control/I/O owners compose with T337 delivery. Do not widen protected selectors, gates, IOPL/TSS, VM86, or 80386 width. |
| S4: 80186 primary extensions | `60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9` exact acceptance, rejections, state/fault/REP behavior. | 80186 handlers and metadata gates. No 80286 ARPL or 80386 attributes. |
| S5: T338 closure audit | Reconcile every table row against completed S2--S4 evidence; audit the legacy LOCK linkage and transfer remaining protected/80386/external rows. | Documentation/evidence only; cannot implement missing behavior under an audit label. |

## Required common evidence for later S

Every S2--S4 form row records profile acceptance on both 8086 and 80186,
defined state only, real-mode fault/restart contract, and memory/I/O
nonpublication when applicable. It must name its `F0` relationship to T328;
the valid legacy rule does not make an invalid opcode or invalid ModRM legal.
`66/67` are 80386 attributes: their legacy rejection is evidence only, not a
reason to implement 32-bit behavior in T338.
