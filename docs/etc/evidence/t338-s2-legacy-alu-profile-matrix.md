# T338 S2: 8086/80186 Legacy ALU, FLAGS, And Condition Matrix

## Scope And Mechanism Sweep

This record closes only T338 S2's allocation from the S1 ledger. The owner
smoke is `core-machine-legacy-alu-s2-smoke`; it uses the established
owner-local real-mode fixture and intentionally does not introduce a common
test framework. The source sweep covered the primary dispatch entries and
`INS_80`, `INS_81`, `INS_83`, `INS_C0`, `INS_C1`, `INS_D0`--`INS_D3`,
`INS_F6`, `INS_F7`, `INS_FE`, all `_a_*` arithmetic helpers, `_kaf_set_flags`,
`UndefinedOpcode`, and the existing INC/DEC, rotate, local-FLAGS, LAHF/SAHF,
and direct-FLAGS owners.

The owner smoke supplies the low-profile form matrix below. Existing focused
owners remain regression evidence for the wider fault and 80386-only paths;
they are not treated as 8086/80186 proof. No production defect was reproduced.
The two development failures were test-oracle mistakes: ADC/SBB AF/OF uses the
original source operand, and Group 3 has distinct AL/AX and AX/DX result
destinations. Both were corrected in the owner smoke without weakening a CPU
assertion.

## Form And Profile Evidence

| Allocated family | 8086 proof | 80186 proof | Defined state and boundary |
| --- | --- | --- | --- |
| `00`--`05` through `38`--`3D` | 8 operations, both directions, byte/word, register and r/m forms, plus every accumulator-immediate encoding | Same inherited matrix | Result or memory publication; ADD/ADC/SUB/SBB/CMP CF/OF/AF/SF/ZF/PF and logic defined flags; nonparticipants and EIP. |
| `80/81/83 /0`--`/7` | byte/word, register/r/m, `83` sign extension | Same inherited matrix | Same arithmetic publication and defined-flag matrix; CMP does not publish its destination. |
| `70`--`7F` | Every predicate taken and not taken | Same inherited matrix | Target/fall-through EIP, FLAGS and GPR preservation. |
| `E0`--`E3` | LOOPNE/LOOPE/LOOP/JCXZ taken and not taken | Same inherited matrix | Low-word count update with upper E[C]X retention, EIP, FLAGS. |
| `84/85`, `A8/A9`, `F6/F7 /0` | byte/word register and immediate TEST forms | Same inherited matrix | CF/OF clear; PF/ZF/SF from result; no destination publication. |
| `27/2F/37/3F`, `D4/D5`, `D7` | DAA/DAS/AAA/AAS, AAM/AAD base 10, XLAT | Same inherited matrix | Defined adjustment flags only; AL/AX result and DS:BX+AL lookup publication. |
| `40`--`4F`, `FE /0,/1` | Every register INC/DEC plus byte r/m | Same inherited matrix | CF retention and defined arithmetic flags; register/memory publication. |
| `F6/F7 /2`--`/7` | NOT/NEG/MUL/IMUL/DIV/IDIV byte and word; zero-divisor `#DE` IVT delivery | Same inherited matrix | Correct AL/AX or AX/DX result locations; divide restart IP, IP/CS/FLAGS frame, nonpublication, and post-gate IF/TF behavior; defined CF/OF only where Intel defines them; no undefined-flag claim. |
| `98/99`, `9E/9F`, `F5/F8/F9/FC/FD` | CBW/CWD, LAHF/SAHF, CMC/CLC/STC/CLD/STD | Same inherited matrix | Exact defined GPR/FLAGS mutation and nonparticipant preservation. |
| `D0`--`D3 /0`--`/7` | byte/word, count 1 and CL count; `/6` #UD | Same inherited matrix | Defined result/CF where applicable, reserved nonpublication and restart; undefined flags not asserted. |
| `C0/C1 /0`--`/7` | `#UD` before operand publication | byte/word immediate count forms, `/6` #UD | 80186 acceptance; 8086 rejection, defined result/CF only. |
| `82`, `D6`, `F1`, `F6/F7 /1`, `66/67` variants | Complete real `#UD` terminal/nonpublication probes | Same | EIP restart, GPR/FLAGS preservation, and no destination/source publication. |

## Legacy LOCK Linkage

Every S2 memory-capable valid form (`00`--`3D`, `80/81/83`, TEST, `FE`,
`F6/F7`, and Group 2) consumes the single T328 legacy rule. On 8086/80186,
`F0` is transparent bus-prefix handling for a valid following instruction; it
does not invoke a 80386 whitelist. The T328 smoke proves register, memory,
REP-string, and port-I/O transparency plus invalid-opcode retention. This S2
does not duplicate or alter `PREFIX_LOCK`; invalid/reserved forms remain
invalid after the prefix scanner.

## Verification

`core-machine-legacy-alu-s2-smoke` emits `M5:T338:S2:LEGACY-ALU:OK`. It is a
current-gate target. The S closure additionally runs the current artifact and
metadata verifiers, documentation governance, diff check, and complete current
gate. The rebuilt `vm-0-5-0338` developer artifact SHA-256 is
`E9626E829FE8F9A1BE7A25219D48295D704C5831F64C4D2D50709671CB144F13`.
This document retires into the T338 history record at task closure.
