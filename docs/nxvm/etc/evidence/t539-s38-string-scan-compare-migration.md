# T539 S38: STOS/SCAS/CMPS Test Owner Migration

## Original-case receiving map

| Retired source / original family | Sole receiver |
| --- | --- |
| STOS: four profiles x AA/AB (8), 386 width/address/segment forms (6), REP zero/one/three and width/address/direction forms (16), illegal attributes/LOCK (17) | `cpu_stos_smoke.c` |
| STOS: two protected single forms and REP first-item commit then destination-limit fault (3) | `cpu_stos_smoke.c` for hidden-cache/rollback semantics; `machine_stos_board_smoke.c` for guest descriptors and delivered fault |
| STOS: byte/word IRQ and REP restart (3) | `machine_stos_board_smoke.c`, using real PIC, stack frame and public memory observations |
| SCAS: four profiles x AE/AF (8), 386 width/address/segment forms (6), flags (4), REPE/REPNE zero/one/three and termination cases (24), illegal attributes/LOCK (26) | `cpu_scas_smoke.c` |
| SCAS: protected single and REP limit forms (2) | `cpu_scas_smoke.c` for hidden-cache/rollback semantics; `machine_scas_board_smoke.c` for guest descriptors and delivered fault |
| SCAS: single and REP IRQ restart (2) | `machine_scas_board_smoke.c`, using real PIC and interrupt frame |
| CMPS: four profiles x A6/A7 (8), 386 width/address forms (4), segment override (2), flags (4), REPE/REPNE termination (24), REP width/address/direction (4), illegal attributes/LOCK (26) | `cpu_cmps_smoke.c` |
| CMPS: source/destination single and REP protected-limit faults (4) | `cpu_cmps_smoke.c` for hidden-cache/rollback semantics; `machine_cmps_board_smoke.c` for distinct guest-loaded DS/ES descriptors and delivered fault |
| CMPS: single and REP IRQ restart (2) | `machine_cmps_board_smoke.c`, using real PIC and interrupt frame |

These families total STOS 53, SCAS 72 and CMPS 78: 203 original contexts.
The three mixed-owner sources are deleted; six replacements are registered
once in the repository-only unit partition. CPU tests link only `x86-cpu`;
board tests use public machine operations rather than private CPU/RAM state.
Protected-limit contexts deliberately have complementary chip and board
assertions, not parallel production execution paths. The original sources
had no explicit timing assertions. Historical T316/T401 success markers
remain on their CPU receivers.

## Verification and boundary

The change touches tests and NXVM build/static-gate registration only. It
does not alter instruction algorithms, production APIs, Shared code/tests,
firmware, profile, INI or EXE inputs. The existing public-only REP-CMPS test
retains its owner. Port-I/O cases remain assigned to S39. The direct-private
`.c` test inventory falls from 64 to 61; the common fixture header remains.

The thirteen code/test/build/gate paths add 1,682 and remove 2,034 lines,
net minus 352 (`git diff --cached --numstat`, excluding the two documentation
paths). All six new tests are in the unit suite. All three CPU receivers use
the same `x86-cpu`-only contract and GNU warnings-as-errors rule; their board
partners run genuine guest setup and IRQ delivery.

Complete x64/x86 builds pass. The final complete repository-only unit suites
pass 411/411 per width. The specialized gate aggregates pass 66 targets on
x64 and 68 on x86, including T317/T332/T337/T344, CPU/PIC authority and the
T344 direct-compilation matrices (410 x64 rows, 409 x86 rows). The x64
manifest selection passes 11/11, including the unchanged six Shared corpus
manifests. Documentation governance and `git diff --cached --check` pass.
The full build regenerated one tracked x86 EXE even though S38 changes no
executable inputs; that build side effect was restored to its initially clean
tracked version and is not part of S38.

The pushed P1 actual-commit review and S38 acceptance are recorded in
`docs/nxvm/states/CURRENT.md`.
