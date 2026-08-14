# T359 S2: Four-Profile Arithmetic And Data Timing Source Ledger

## Scope and assumptions

This workbook is the source ledger for the S2 primary arithmetic, defined-FLAGS,
data, and ModRM/effective-address mechanism.  It records core execution clocks
only: every cited table assumes prefetched/decoded instructions, no wait states,
no HOLD, no exception, and (where stated) aligned memory.  It does not allocate
device service, bus reservation, prefetch starvation, or exception-delivery
time.

| Profile | Primary authority | Form notation and assumption |
| --- | --- | --- |
| 8086 | *Intel 8086 Family User's Manual* (1979), Tables 2-20 and 2-21, pp. 2-50--2-67 | Table 2-21 base plus Table 2-20 EA; add two clocks for a segment override and four clocks for each odd word transfer. |
| 80186 | *iAPX 86/88, 186/188 User's Manual* (210912-001), Table 1-16, pp. 1-24--1-40 | Parenthesized value is 80186.  Table 1-15 supplies the inherited 8086 EA component; the table retains the documented odd-word rule. |
| 80286 | *Intel 80286 and 80287 Programmer's Reference Manual* (210498-005), Appendix B | `reg,mem=` rows distinguish ModRM register and memory forms.  The existing Appendix-B `NOP = 3` precedence remains in force. |
| 80386 | *Intel 80386 Programmer's Reference Manual*, section 17.2.2.3 and the exact opcode pages | `r/m` rows use register/memory values respectively; the manual's `m`, `n`, and `pm=` notation remains profile-form specific. |

## Fixed four-profile mechanism families

| Owner family | Primary opcodes/forms | Required classifier distinction |
| --- | --- | --- |
| ALU two-operand | `00`--`3D`, `80`/`81`/`83` `/0`--`/7` | destination `r/m` read-write versus register destination from memory; accumulator immediate; register versus memory; 16/32 operand width only where the source has a distinct row. |
| Defined-FLAGS data | `84`/`85`, `A8`/`A9`, `86`/`87`, `91`--`97`, `40`--`4F`, `FE`/`FF` `/0`--`/1`, `F6`/`F7` `/2`--`/3`, `98`/`99`, `D4`/`D5`, `27`/`2F`, `37`/`3F` | Do not assert undefined FLAGS; timing selection is independent of their final bit values. |
| Ordinary data transfer | selected `88`--`8B`, `A0`--`A3`, `B0`--`BF`, `C6`/`C7`, `8D` | register/memory direction, moffs read/write, immediate destination, and LEA's address-form-only cost. |
| 80386 conditional byte result | `0F 90`--`9F` | register/memory destination; condition value must not change the source-row cost. |

## Primary-row normalization

The classifier must normalize the *encoding shape* once, then select a
profile-local source row.  It must not equate source rows merely because an
implementation handler is shared.  In particular, an r/m destination is a
read-modify-write form, while a register destination from memory is a read
form; 80386 also assigns opposite direction costs to some logical/subtract
families.

| Normalized encoding shape | 8086 primary row | 80186 parenthesized row | 80286 Appendix-B row | 80386 PRM row / disposition |
| --- | --- | --- | --- | --- |
| ADD/ADC/AND `r/m,r` | `16+EA` | `10+EA` | `2,mem=7` | `2/7` |
| ADD/ADC/AND `r,r/m` | `9+EA` | `10+EA` | `2,mem=7` | `2/6` |
| OR/SBB/SUB/XOR `r/m,r` | `16+EA` | `10+EA` | `2,mem=7` | `2/6` |
| OR/SBB/SUB/XOR `r,r/m` | `9+EA` | `10+EA` | `2,mem=7` | `2/7` |
| ADD/ADC/AND/OR/SBB/SUB/XOR accumulator immediate | `4` | `3--4` by width | `3` | `2` |
| ADD/ADC/AND/OR/SBB/SUB/XOR register immediate | `4` | `4` | `3` | `2` |
| ADD/ADC/AND/OR/SBB/SUB/XOR `r/m,imm` | `17+EA` | `16+EA` | `3,mem=7` | `2/7` |
| CMP `r/m,r` / `r,r/m` | `3` or `9+EA` as applicable | `3` or `10+EA` as applicable | `2,mem=7` / `2,mem=6` | `2/5` / `2/6` |
| CMP accumulator/register immediate | `4` | `3--4` by width | `3` | `2` |
| CMP `r/m,imm` | `10+EA` | `16+EA` | `3,mem=6` | `2/5` |
| TEST `r/m,r` | `3` or `9+EA` as applicable | `3` or `10+EA` as applicable | `2,mem=6` | `2/5` |
| TEST accumulator/register immediate | `4` / `5` by width | `3--4` by width | `3` | `2` |
| TEST `r/m,imm` | `11+EA` | `10+EA` | `3,mem=6` | `2/5` |
| XCHG register/register, accumulator/register, register/memory | `3`, `3`, `17+EA` | `4`, `3`, `17+EA` | `3`, `3`, `3,mem=5` | `3`, `3`, `3/5` |
| register INC/DEC, byte/word | `3` / `2` | `3` | `2` | `2` |
| `FE`/`FF` `/0,/1` r/m INC/DEC | `15+EA` | `15+EA` | `2,mem=7` | `2/6` |
| `F6`/`F7` `/2,/3` NOT/NEG | `3`, `16+EA` | `3`, `3+EA` | `2,mem=7` | `2/6` |
| DAA/DAS/AAA/AAS/AAM/AAD/CBW/CWD | `4/4/8/8/83/60/2/5` | `4/4/8/7/19/15/2/4` | `3/3/3/3/16/14/2/2` | `4/4/4/4/17/19/3/2` |
| LEA | `2+EA` | `6+EA` | `3`, plus the Appendix-B base/index/displacement addition | `2` |
| SETcc | not available | not available | not available | `4/5` |

`3--4` records an authority-defined width distinction and is deliberately not a cost
constants.  The classifier resolves each admitted encoding by its decoded
width before publishing a successful-retirement value.  The notation remains
only to preserve the source table's byte/word distinction; it is not a
synthesized range or a runtime approximation.

## Dynamic-form disposition

The source rows below are genuine S2 forms but are not represented by a static
cost constant.  They require a pre-execution operand capture or an explicit
source formula.  A range alone is not an exact L3 allocation.

| Profile/form | Primary row | Disposition |
| --- | --- | --- |
| 8086 `F6`/`F7` `MUL` and `IMUL` | Table 2-21 gives ranges (`MUL r8 70--77`, `MUL r16 118--133`; `IMUL` rows likewise ranges) without an input-to-clock algorithm | Transfer to T360 primary-source reconciliation; do not synthesize a bit-count formula. |
| 80186 `F6`/`F7` `MUL`, `IMUL`, `DIV`, `IDIV`; `69`/`6B` immediate `IMUL` | Table 1-16 gives parenthesized ranges, including `MUL r8 26--28`, `MUL r16 35--37`, `IMUL imm8 22--24`, and `IMUL imm16 29--32`, without a formula | Transfer to T360 primary-source reconciliation; static min/max would be an approximation. |
| 80286 `F6`/`F7` `MUL`, `IMUL`, `DIV`, `IDIV`; `69`/`6B` immediate `IMUL` | Appendix B gives fixed `reg,mem=` rows: `13/16`, `21/24`, `14/17`, `22/25`, `17/20`, `25/28`, and `21/24` respectively | S2 owns these exact rows through the normalized Group-3 or immediate-IMUL shape. |
| 80386 `F6`/`F7` `MUL`/`IMUL`, `69`/`6B` immediate `IMUL` | Exact IMUL/MUL pages give early-out formula: zero multiplier is 9 clocks; otherwise `max(ceiling(log2(m)), 3) + 6`, plus 3 for memory multiplier | S2 owns a private pre-execution multiplier capture and formula, provided it shares the successful-retirement publication boundary and does not alter execution semantics. |
| 80386 `F6`/`F7` `DIV`/`IDIV` | Appendix-B rows are `14/17`, `22/25`, `38/41` and `19/22`, `27/30`, `43/46` for byte/word/dword register/memory operands | S2 owns successful rows; the 46-clock IDIV-memory row sets the source preflight ceiling. |
| Divide forms that fault | all profile rows | No successful-retirement time is published.  Fault/delivery timing remains a later physical/exception receiver. |

## Allocation correction

S1's row-level inventory assigned `0F 90`--`9F` SETcc to S2, while one ordered
summary also listed SETcc under S4.  S2 is authoritative for this form because
it shares the defined conditional-FLAGS/data-destination mechanism; S4 excludes
it.  This correction changes no S1 selected receiver and is recorded here so
the later packet cannot double-own SETcc.

## Mechanism and focused evidence

`core_machine_primary_source_instruction_cost` is the one primary encoding
shape owner. It selects the profile-local row after successful instruction
refresh, so r/m read, r/m write, and register forms cannot drift merely
because their implementation handler is shared.
`core_machine_80386_dynamic_multiply_cost` is its only dynamic peer. It uses
decoder-captured `crm` or `cimm`, never rereads architectural state, and
implements the PRM early-out formula only where the 80386 source supplies one.
The 80386 Appendix-B zero-clock prefix rows admit only the source-listed
operand/address-size, segment-override, and `LOCK` bytes here; existing decode
and LOCK legality stay the sole semantic authority.

`core-machine-t359-s2-timing-smoke` proves representative register, direct
memory, EA, width-prefix, SETcc, selected MOV, NOT/NEG, static Group-3,
dynamic multiply, and elapsed-provider paths. Its 80386 32-bit IDIV-memory
row proves both the 46-clock ceiling and budget preflight: 45 blocks
retirement without publication, while 46 permits exactly one retirement. The
owner also distinguishes odd-address word reads from read-modify-write forms:
the 8086, 80186, and 80286 rows add one or two documented odd-word transfers,
respectively, rather than treating every memory form as a read-modify-write.
The retained T357 ledger smoke proves a fault restart publishes zero
instruction and elapsed clocks and that reset clears elapsed state.

The transferred 8086/80186 range-only rows require the named T360
source-reconciliation receiver before any non-approximated timing value is
published.

## Verification record

The retained T357 ledger smoke and the owner S2 marker both pass.  The
inventory verifier, documentation governance check, and three disjoint
current-gate slices (tests 1--80, 81--160, and 161--241) pass; together they
cover the complete 241-test current gate.  The rebuilt developer artifact is
`build/output/nxvm_0_5_0359.exe`, source worktree pending the S2 P1 commit,
with SHA-256
`2D91DCD23488FD86E64B1EB5126C27DE293183619F7841C96E447FBA63E15904`.
