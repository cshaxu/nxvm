# T401 S77: Four-Profile Concrete Decoded-Form Requirement Matrix

## Matrix contract

This matrix consumes the frozen T401 S1 decoder universe: 256 primary bytes,
256 `0F` bytes, eight FPU-escape families and the shared prefix/lexeme path.
A range in the first column means **every byte in that closed range**; a group
row explicitly includes only the listed ModR/M extensions, while every other
extension is an invalid-form row.  Profile membership is the current metadata
minimum plus every later selected profile.  `66`/`67`, segment, LOCK and
REP/REPNE are included only where their row says so; otherwise the S57 prefix
row owns their valid/rejected disposition.  This is a finite aggregation, not
a claim that one representative executes every member.

The semantic form authority is the Intel 8086 Family User's Manual (Tables
2-20/2-21 and mnemonic entries), Intel iAPX 86/88/186/188 User's Manual
(Table 1-16 and mnemonic entries), Intel 80286/80287 PRM (instruction
dictionary, Appendix B and mnemonic entries), and Intel 80386 PRM order
230985 (Appendix A opcode map, Chapter 17 instruction entries and
17.2.2.3 timing entries).  The current retained scans are linked by the T401
ledger.  A row's source locus names the exact manual opcode/mnemonic entry;
T357/T359/T360 remain the numerical timing authority and its explicit
nonphysical transfers.

| Complete concrete member set | Profile/form scope and Intel source locus | Current owner and focused proof | Timing disposition | Conclusion / repair or transfer |
| --- | --- | --- | --- | --- |
| `00`--`05`, `08`--`0D`, `10`--`15`, `18`--`1D`, `20`--`25`, `28`--`2D`, `30`--`35`, `38`--`3D`; every legal ModR/M direction and accumulator immediate | 8086+; Intel ALU mnemonic entries and 80386 Appendix A. | Core primary ALU owner; S56 and legacy-ALU matrix. | S75 primary source row or visible nonphysical observation. | Accepted semantic matrix; uncited dynamic context retains nonphysical observation. |
| `06/07`, `0E`, `16/17`, `1E/1F` | 8086+ segment PUSH/POP; Intel PUSH/POP entries. | Core stack/selector owner; S41. | S75 control-stack or fallback observation. | Accepted. |
| `26/2E/36/3E`, `64/65`, `66/67`, `F0`, `F2/F3` | Legacy segment/repeat forms 8086+; FS/GS and size attributes 80386 only; Intel prefix entries/Appendix A. | Shared prefix/lexeme owner; S5/S57. | No inherited tick; S75 applies the successor's source/nonphysical disposition. | Accepted form classification; no prefix-cycle claim. |
| `27`, `2F`, `37`, `3F`, `D4`, `D5` | 8086+ DAA/DAS/AAA/AAS/AAM/AAD entries. | Primary adjustment owner; S34. | S75 primary source/nonphysical. | Accepted. |
| `40`--`4F`; `50`--`5F`; `8F /0` | 8086+ register INC/DEC and GPR PUSH/POP, plus POP r/m; Intel entries. | Arithmetic/stack owners; S11/S40. | S75 primary/control-stack source or observation. | Accepted; `8F /1`--`/7` invalid. |
| `60/61`, `68`, `6A`, `6C`--`6F`, `C0/C1`, `C8/C9` | 80186+ PUSHA/POPA, PUSH immediate, INS/OUTS, immediate Group-2 and ENTER/LEAVE; 80186 Table 1-16. | Core stack/string/shift owners; S21/S24/S31/S50/S51. | S75 source/nonphysical. | Accepted; rejected on 8086. |
| `62`, `63`, `69`, `6B` | BOUND 80186+, ARPL 80286+, immediate IMUL 80186+; corresponding Intel entries. | Core BOUND/ARPL/IMUL owners; S28--S30. | S75 primary/privileged source or observation. | Accepted profile gates. |
| `70`--`7F`, `E0`--`E3`, `E8/E9/EB`, `9A/EA` | 8086+ conditional/direct near/far control forms; Intel Jcc/LOOP/CALL/JMP entries. | Core control-transfer owner; S43/S53--S55. | S75 control-stack/source or observation. | Accepted; physical fetch/prefetch remains external. |
| `80/81/82/83 /0`--`/7` | 8086+ Group-1 immediate ALU; `82` accepted as explicit reference-derived 8086 alias. | Primary Group-1 owner; S7. | S75 primary source/nonphysical. | Accepted; all selectors explicit. |
| `84/85`, `A8/A9`, `F6/F7 /0` | 8086+ TEST r/m,reg/accumulator/group immediate forms; Intel TEST entries. | Primary TEST/Group-3 owner; T316 S7, S9. | S75 primary source/nonphysical. | Accepted; Group-3 `/1` invalid. |
| `86/87`, `90`--`97` | 8086+ XCHG accumulator and r/m forms; Intel XCHG entry. | XCHG owner; S12/S46/S49. | S75 primary source/nonphysical. | Accepted. |
| `88`--`8B`, `A0`--`A3`, `B0`--`BF`, `C6/C7 /0` | 8086+ GPR/moffs/immediate MOV; Intel MOV entry and Appendix A. | MOV owner; S13/S14/S47/S58. | S75 primary/profile source or observation. | Accepted; `C6/C7 /1`--`/7` invalid. |
| `8C`, `8E`, `8D`, `C4/C5` | 8086+ segment MOV, LEA, LES/LDS; Intel MOV/LEA/LES/LDS entries. | Selector/data owner; S33/S44/S48/S49. | S75 source/nonphysical. | Accepted; documented memory-only/segment restrictions. |
| `98/99`, `9C/9D`, `9E/9F`, `F5`, `F8/F9`, `FC/FD`, `FA/FB`, `F4` | 8086+ conversion, FLAGS, CLI/STI and HLT entries. | Core flags/control owner; S36--S39/S42/S45. | S75 primary/control-stack source or observation. | Accepted; mode/privilege fault forms do not retire. |
| `A4`--`A7`, `AA`--`AF` | 8086+ MOVS/CMPS/STOS/LODS/SCAS, primitive and legal repeat variants; Intel string entries. | Shared string owner; S15--S19. | S75 string-I/O source/nonphysical. | Accepted semantic/restart contract; service/wait timing external. |
| `E4/E5`, `E6/E7`, `EC/ED`, `EE/EF` | 8086+ immediate/DX IN/OUT, with 80386 width and privilege/IOPL variants; Intel IN/OUT entries. | Shared port-I/O owner; S52 and retained protected/TSS-I/O proofs. | S75 string-I/O source for selected rows; every other legal context is nonphysical observed. | Accepted semantic/permission surface; device service and port wait timing external. |
| `C2/C3`, `CA/CB`, `CC`, `CD`, `CE`, `CF` | 8086+ near/far RET, INT3/INT/INTO and IRET entries. | Core control/delivery owner; S22/S23/S25--S27. | S75 control-stack source/nonphysical; delivery cycles excluded. | Accepted. |
| `D0`--`D3 /0`--`/5,/7`; `D4/D5`; `D7` | 8086+ Group-2 legal rotates/shifts, AAM/AAD and XLAT entries. | Shift/adjust/XLAT owner; S8/S34/S35. | S75 primary/profile source or observation. | Accepted; Group-2 `/6` invalid. |
| `FE /0,/1`; `FF /0`--`/2,/4,/6`, `/3,/5` memory-only | 8086+ Group-4/5 INC/DEC/control/PUSH forms; Intel Group entries. | Core Group-4/5 owner; S10. | S75 primary/control-stack source or observation. | Accepted; all other extensions/restricted register forms invalid. |
| `D6`, `F1`; all metadata-invalid primary/group extensions above | Selected profiles as metadata dictates; Intel Appendix A/reserved form rule. | Metadata + UndefinedOpcode owner; S3--S5/S7--S10/S58. | No successful retirement. | Rejected; no timing row. |
| `0F` on 8086; `0F` on 80186 | Historical POP-CS handling/80186 rejection; 8086 and 80186 opcode references. | Primary escape scanner/executor; S5. | No secondary timing row. | Rejected from selected secondary scope. |
| `0F 00 /0`--`/5`, `01 /0`--`/4,/6`, `02`, `03`, `06` | 80286+ selector/table/system forms; 80286 dictionary and 80386 Appendix A entries. | Descriptor/system owner; Batch A, S70/S71. | S75 privileged/80286 source or observation. | Accepted; `/6,/7` and listed reserved extensions reject. |
| `0F 20`, `22 /0,/2,/3`; `21`, `23 /0`--`/3,/6,/7`; `24`, `26 /6,/7` | 80386 CR/DR/TR MOV forms; Appendix A/MOV control/debug/test entries. | System register owner; S66/S67/S69. | S75 privileged/secondary source or observation. | Accepted; CR original-80386 MOD option remains private; DR4/5 and TR0--5 reject. |
| `0F 80`--`8F`, `90`--`9F` | 80386 near Jcc and SETcc; Chapter 17 entries. | Control/primary owner; S59/S60. | S75 secondary/primary source or observation. | Accepted. |
| `0F A0/A1/A8/A9`, `A3/AB/B3/BB`, `BA /4`--`/7`, `A4/A5/AC/AD`, `AF`, `BC/BD`, `B6/B7/BE/BF`, `B2/B4/B5` | 80386 FS/GS, bit, double-shift, IMUL2, bit-scan, MOVX and LSS/LFS/LGS; Appendix A/instruction entries. | Current 80386 secondary owners; S41/S61--S65/S68. | S75 secondary/privileged source or observation. | Accepted; `BA /0`--`/3` and unlisted secondary bytes reject. |
| every other `0F` byte | Selected profile metadata and Appendix A. | Metadata/UndefinedOpcode owner; S3/S72. | No successful retirement. | Rejected, later-CPU or unsupported; no timing row. |
| `9B`, `D8`--`DF` | WAIT and all ESC ModR/M forms with optional coprocessor; Intel CPU/FPU interface entries. | WAIT/FPU_ESCAPE and provider boundary; S6/T316 S65. | External numerical/provider timing. | CPU-side contract accepted; no x87 completeness or timing claim. |

## Cardinality and current-source checks

- The complete byte spaces are mechanically fixed by 256 explicit primary and
  256 explicit secondary dispatch assignments. `verify-t359-instruction-timing-inventory`
  checks both counts, metadata owner, unique successful-retirement publisher
  and visible unallocated receiver.
- S73 consumes every primary metadata-valid byte into the first 20 rows; S72
  consumes every selected valid `0F` byte into the secondary rows. The present
  matrix refines those receivers by exact group-extension, form and profile
  membership rather than adding an implementation path.
- Every source-backed numerical entry remains at its existing T357/T359/T360
  Intel-manual owner. Every otherwise successful selected row publishes the
  S75 nonphysical observation; invalid/faulting forms do not receive an
  invented successful-retirement clock.

## S77 provisional conclusion

The matrix makes every frozen T401 decoded form addressable to primary manual
form authority, current Core owner, focused proof and source/nonphysical/
external disposition. It makes no physical board timing claim. A separate S78
review must independently compare this matrix with metadata, dispatch and the
original T401 proposal before T401 can close.