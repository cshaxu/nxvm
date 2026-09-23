# T359 S3: Four-Profile Control And Stack Timing Source Ledger

## Scope and publication boundary

This ledger allocates only successful, non-privileged instruction retirement.
`core_machine_control_stack_source_instruction_cost` runs after refresh at the
sole timing publisher.  It reads decoder-captured instruction outcome and the
nonpublishing lexeme preview; it neither changes stack, segment, exception, nor
provider state.  A fault, a rejected prefix/LOCK form, and every transferred
transition therefore publish no invented instruction clocks here.

| Profile | Authority | Reading rule |
| --- | --- | --- |
| 8086 | *Intel 8086 Family User's Manual*, Tables 2-20/2-21 | Table row plus the documented EA, segment-override, and odd-word additions. |
| 80186 | *iAPX 86/88, 186/188 User's Manual*, Table 1-16 | Parenthesized 80186 value; inherited EA/odd-word rule remains explicit. |
| 80286 | *Intel 80286/80287 PRM*, Appendix B | Add one clock for each byte of the following instruction where Appendix B says so; the preview supplies only that byte count. |
| 80386 | *Intel 80386 PRM*, Table 8-1 and instruction pages | `m` is the next-instruction lexeme component count, not a synthetic range. |

## Normalized successful forms

| Shape | 8086 | 80186 | 80286 | 80386 |
| --- | ---: | ---: | ---: | ---: |
| near CALL direct/register/memory | 19 / 16 / `21+EA` | 14 / 13 / `19+EA` | 7 / 7 / 11, plus next byte | `7+m` / `7+m` / `10+m` |
| far CALL direct/memory, same privilege only | 28 / `37+EA` | 23 / `38+EA` | real 13/16, protected 26/29, plus next byte | real `17+m`/`22+m`, protected `34+m`/`38+m` |
| near JMP direct/register/memory | 15 / 11 / `18+EA` | 14 / 11 / `17+EA` | 7 / 7 / 11, plus next byte | `7+m` / `7+m` / `10+m` |
| far JMP direct/memory, same privilege only | 15 / `24+EA` | 14 / `26+EA` | real 11/15, protected 23/26, plus next byte | real `12+m`/`17+m`, protected `27+m`/`31+m` |
| near RET / immediate | 8 / 12 | 16 / 18 | 11 / 11, plus next byte | `10+m` / `10+m` |
| PUSH register/memory/immediate; POP register/memory | 11/`16+EA`/n/a; 8/`17+EA` | 10/`16+EA`/10; 10/`20+EA` | 3/5/3; 4/5 | 2/5/2; 4/5 |
| PUSHA / POPA / PUSHF / POPF | n/a / n/a / 10 / 8 | 36 / 51 / 9 / 8 | 17 / 19 / 3 / 5 | 18 / 24 / 4 / 5 |
| ENTER level 0 / level 1 / LEAVE | n/a | 15 / 25 / 8 | 11 / 15 / 8; level >1=`12+4n` | 10 / 12 / 4; level >1=`15+4(n-1)` |
| LOOP family taken/not; JCXZ taken/not | 17--19/5--6; 18/6 | 15--16/5--6; 16/5 | 8/4; 8/4 | `11+m`/5; `9+m`/5 |
| HLT | 2 | 2 | 2 | 5 |
| INT3 / INT imm / INTO taken/not / same-level IRET | 52 / 51 / 53/4 / 24 | 45 / 47 / 48/4 / 28 | 23 / 23 / 24/3 / real 17, protected 31; next-byte rule where listed | real 33 / 37 / 35/3 / 22; protected same-level INT 59 and IRET 38 |

`INTO` uses the non-taken row when the pre-instruction OF is clear.  The
classifier uses post-refresh EIP to distinguish a successful branch from its
architected fall-through and to preview the transferred path's next lexeme
where the source row requires it; it preserves 16-bit wrap for 16-bit code.

## Disposition and transfers

| Disposition | Forms |
| --- | --- |
| S3 allocation | The successful real-mode forms above; legal operand/address prefixes only where the profile accepts them. |
| Zero publisher allocation | Faults, rejected prefixes/LOCK, failed stack access, and rejected returns. Their delivery is not instruction-retirement time. |
| Transfer to S4 | String/repeat and ordinary I/O forms. |
| Transfer to S5 | 80386 secondary/prefix-width corpus not normalized above. |
| Transfer to S6 / physical receiver | Protected far CALL/JMP, protected software `INT`/`INTO`/`IRET`, call gates, cross-privilege or VM86 return/entry, task/NMI/generic exception delivery, and any form whose source row includes an unmodeled physical transition. The 80386 protected same-level `INT` row is 59 clocks, beyond the existing 46-clock source preflight ceiling; raising a global ceiling would incorrectly reject unrelated lower-cost instructions, so it is deliberately not allocated by S3. |
| Transfer to T360 | Any later discovered range-only or conflicting source row; no range midpoint is allocated. |

## Mechanism and proof

The private classifier normalizes opcode, ModRM extension, memory shape,
prefix legality, mode, privilege outcome, branch outcome, and stack form once.
It consumes existing 8086/80186 EA and odd-word additions, 80286 preview byte
count, and 80386 lexeme components.  The owner smoke proves direct and
indirect transfer, register/memory stack forms, all four profile rows,
PUSHA/POPA/ENTER/LEAVE, taken/not-taken loop control, HLT, real-mode
`INT`/`INT3`, taken and non-taken `INTO`, real-mode same-level `IRET`, provider elapsed publication,
and no one-tick receiver for those representatives.  The retained T357 ledger
smoke remains the fault/reset zero-publication proof.

## Retained timing-contract reconciliation

The S3 integration rebuilt timing owners against the source publisher rather
than accepting stale executable output. The following fixture corrections are
part of this ledger's regression proof, not runtime behavior changes.

| Retained owner | Corrected contract |
| --- | --- |
| `machine_time_smoke` | A 80286 source-profile run needs a budget at least its 28-clock preflight ceiling even when the next NOP costs three clocks; the fixture supplies a mapped third NOP and asserts the source-derived nine-clock total. |
| `machine_timing_checkpoint_smoke` | Its reset mapping is bounded to the 16 bytes before 16-bit IP wrap. It proves deterministic repeated NOP retirement without reading unrelated bytes after the code-segment wrap. |
| Real-mode tick and instruction timing owners | `INT` observes the transferred vector path's following lexeme where Appendix B requires it; 80286 `HLT` is two clocks; and the mixed ALU/Jcc sequence uses its exact 2+7 source rows. |
| 80186/80286/80386 ledger owners | A formerly unallocated `XOR` now belongs to S2's source ledger. The fixtures use an actually unallocated shift form, and their explicit preflight budgets meet the established profile maxima. |
| DMA/RTC and firmware timer owners | Device progression observes source-derived NOP/HLT elapsed clocks. The firmware single-step retains its one-instruction pause proof and requires positive elapsed progress instead of the obsolete one-tick assumption. |

## Verification record

The S3 owner marker, inventory verifier, documentation governance, current
artifact verification, and full current-gate pass. The complete current gate
is 242/242 tests. The rebuilt developer artifact is
`build/output/nxvm_0_5_0359.exe`, SHA-256
`B0B8BC36D67A9204E66366C1776EA96E424EFA111A1E1283529042C94E31FFC5`.
