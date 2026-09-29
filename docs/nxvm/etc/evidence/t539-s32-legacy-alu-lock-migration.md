# T539 S32: legacy ALU and LOCK test ownership

## Original-case receiving map

The two original mixed-owner sources contain 775 execution contexts. The
original profile, opcode, operand, flags, exception and bus observations have
one receiver each; the product CPU and board implementations do not change.

| Original group | CPU-only | Public board |
| --- | ---: | ---: |
| ALU binary forms | 288 | 0 |
| ALU accumulator immediates | 72 | 0 |
| ALU Group 1 immediates | 96 | 0 |
| ALU conditional branches | 64 | 0 |
| ALU LOOP family | 16 | 0 |
| ALU TEST forms | 12 | 0 |
| ALU decimal adjust and XLAT | 28 | 0 |
| ALU Group 3 | 24 | 0 |
| ALU INC/DEC and shift extensions | 38 | 0 |
| ALU flags and sign forms | 18 | 0 |
| ALU Group 2 | 66 | 0 |
| ALU Group 2 immediate extensions | 18 | 0 |
| ALU reserved encodings and attributes | 14 | 0 |
| ALU real divide-vector delivery | 0 | 2 |
| LOCK transparent real-mode forms | 9 | 0 |
| LOCK external port output | 0 | 3 |
| LOCK legacy #UD | 2 | 0 |
| LOCK 80286 protected IOPL / #GP frame | 0 | 3 |
| LOCK 80386 legal / invalid regression | 2 | 0 |
| Total | 767 | 8 |

Both CPU receivers link only `x86-cpu` and compile with warnings as errors.
Invalid-opcode cases retain their terminal vector condition in the CPU fixture.
The LOCK REP MOVS case retains its original two-step execution budget. The
board ALU receiver checks vector 0 and its frame through public machine
operations. The LOCK board receiver checks real port writes and uses guest
LGDT/LIDT/LMSW instructions plus public debug register patches to construct
the 80286 protected state; it checks the delivered #GP frame without writing
`executor_cpu` or any other private machine state.

T344's constructor inventory now explicitly classifies the public ALU board
constructor (116 instead of 115); the historical 108-source inventory is
unchanged. T332 classifies both board receivers as public board fixtures. T337
classifies both CPU receivers as the #UD terminal owners. No production/API,
firmware, INI, media, Shared corpus, MyNES or executable input changes.

## Verification

Complete x64 and x86 builds pass. Both complete repository-only unit suites
pass 399/399, versus the 397/397 S31 baseline plus two CPU receivers. All 66
current specialized gates pass on each width, including T317 strict coverage,
T332 lifecycle, T337 configure-time #UD inventory, T344 historical shapes,
CPU/PIC authority, documentation governance and T344's 398-row direct strict
matrix (365 strict, 33 deferred). The six Shared source/test manifests remain
unchanged and pass in the unit suite. `git diff --check` is clean. No tracked
binary is changed; S32 is a test-owner migration and does not require a new
product executable.
