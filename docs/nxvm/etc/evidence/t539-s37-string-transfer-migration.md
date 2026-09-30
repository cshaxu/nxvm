# T539 S37: MOVS/LODS Test Owner Migration

## Original-case receiving map

| Retired source / original family | Sole receiver |
| --- | --- |
| LODS: four profiles x AC/AD (8), 386 width/address/direction/segment forms (9), REP zero/one/three and width/address/direction forms (16), illegal prefixes/LOCK (28) | `cpu_lods_smoke.c` |
| LODS: two protected single forms and REP first-item commit then source-limit fault (3) | `cpu_lods_smoke.c` for hidden-cache/rollback semantics; `machine_lods_board_smoke.c` for actual guest descriptors and delivered fault |
| LODS: byte/word IRQ and REP restart after its first element (3) | `machine_lods_board_smoke.c`, using real PIC, interrupt stack frame and public machine observations |
| MOVS: four profiles x A4/A5 (8), 386 width/address (3), segment override and DF (6), REP count/width/address/direction (20), illegal prefixes/LOCK (11) | `cpu_movs_smoke.c` |
| MOVS: two protected source/target limit forms (2) | `cpu_movs_smoke.c` for hidden-cache/rollback semantics; `machine_movs_board_smoke.c` for guest-loaded descriptors and delivered fault |
| MOVS: single and REP-first-element IRQ restart (2) | `machine_movs_board_smoke.c`, using real PIC, stack frame and public memory observations |

The two old mixed-owner tests are deleted. Four replacement tests are
registered once in the NXVM repository-only unit partition. CPU receivers
link only `x86-cpu`; board receivers do not read private CPU/RAM fields. The
original files contained no explicit timing-assertion row; no timing formula
or instruction implementation changed. During a terminal board fault, the
public memory-read API is not available in the faulted lifecycle. CPU-local
tests retain the original memory-rollback check; board tests retain the real
descriptor and fault-delivery observation. No production API was added.

The test-only public board-limit fixture now accepts an exact DS limit for
the LODS REP partial-commit case; its existing boolean wrapper preserves all
other callers. No product executable input or Shared source/test changed.

## Verification and boundary

- Original context count: LODS 67 and MOVS 52, total 119. Three LODS and two
  MOVS protected contexts have complementary CPU/board assertions rather
  than duplicated production execution paths.
- Eleven tracked code/test/build/gate paths add 1,065 and remove 1,327
  lines, net minus 262 (`git diff --cached --numstat`, excluding these two
  documentation paths). Four replacement tests pass on x64 and x86. Complete
  repository-only unit suites pass 408/408 per width after the final LODS
  vector and fault-rollback assertions.
- Both widths' 66-target specialized gate aggregate passed, including
  T317/T332/T337/T344, CPU/PIC authority, direct-compilation matrix (407
  rows), and documentation governance. Six unchanged Shared manifests passed
  in the x64 unit suite and explicit manifest selection.
- Remaining STOS/SCAS/CMPS and port cases stay assigned to S38-S39. S37
  changes no source algorithm, public ABI, firmware, profile, INI or EXE input.

P1 implementation is pushed as `3c826c32d`. Its actual-commit P2 review and
S37 acceptance are recorded in `docs/nxvm/states/CURRENT.md`.
