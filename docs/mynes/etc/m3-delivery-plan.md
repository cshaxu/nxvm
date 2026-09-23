# M3 Delivery Plan

## Status Report

T10--T14 are M2 historical work under the owner correction recorded in
[milestone correction](milestone-correction.md). M3 begins with T15 planning and
is closed. Window, release, Console and final G/R reconciliation have direct
current evidence in T16--T19.

## Remaining Architecture And Code Boundaries

`src/app/composition.c` owns the only driver binding and teardown graph;
`src/app/config.c` owns startup facts; command/debug remain copied-protocol
consumers. `src/core/driver.c` owns Common adaptation and signal latches;
`src/core/ppu.c` owns pixels; Core display adaptation owns RGB-to-text frames;
Core controller/input adaptation owns copied held-state delivery. `test/app`,
`test/core` and `test/integration` mirror those owners. Common publishes/routes
frames and Lib presents them; neither receives NES policy or conversion code.

## Ordered Candidate Plan

1. Window-First Presentation Qualification: observable Window gameplay and
   native teardown evidence.
2. Product Code And Release Qualification: R11/R12 quality, strict build and
   fresh artifact evidence.
3. Console Presentation Qualification: accepted in T18; Core text-frame
   gameplay and raw/cooked return now have native x64/x86 proof.
4. M3 Final Reconciliation And Closure: accepted in T19; the finite G/R ledger,
   developer artifacts and milestone audit are closed.

## Implementation Forecast

Estimates are planning ranges for tracked App/Core/test lines, excluding generated
artifacts and documentation. They guide task size; they are not acceptance proof.

| Candidate | Likely production surface | Likely proof surface | Estimated change | Depends on | Concrete outcome |
| --- | --- | --- | --- | --- | --- |
| Window-first | `app/composition.c`, `app/main.c`, `core/driver.c`, `core/ppu.c`, owned private driver/display helpers if needed | `test/app/composition_smoke.c`, `test/core/ppu_smoke.c`, `test/integration/visual_rom_smoke.c` plus a native Window receiver | 250--650 lines | Existing PPU/input path | ROM-driven input visibly changes actual Win32 pixels; focus/close/Esc lifecycle remains safe. |
| Release qualification | root/app/core CMake, only App/Core paths that fail ownership/style review | existing owner tests plus build/static scripts | 100--350 lines | Window proof | R11/R12 current source/build proof, strict x64/x86 build, fresh artifacts and hashes. |
| Console-last | `core/driver.c`, new owner-local display/input helpers only when substantive, `app/composition.c` for route handoff | `test/core`, `test/app`, `test/integration` Console/native receivers | 300--850 lines | Window and release qualifications | Core-generated text frames make the same fixture playable through raw Console; Esc restores cooked monitor. |
| Final closure | only defect receivers discovered by ledger reconciliation | full CTest, native observations, evidence/closure records | 0--300 lines | All preceding evidence | Every applicable G01--G14/R01--R12 member has direct evidence and no blocked backend. |

## Task-Level Boundaries

Window-first may change neither Console conversion nor raw-Console activation;
that keeps its failure graph limited to the existing Window presenter. Release
qualification may consolidate a product-owned mechanism when it finds a real
R11/R12 defect, but cannot smuggle Console behavior into a build task. Console
starts only after Window and release evidence are accepted, so its raw/cooked
handoff can reuse a qualified lifecycle rather than debugging both native paths
at once. Final closure does not invent features: it repairs only a ledger member
and otherwise records its evidence.

## Ledger Handoff

Window-first receives G07's Window half, G12's Window lifecycle half, G13's
Window observation and the native part of R04. Release qualification receives
R11/R12 and G14. Console-last receives G07/G08/G12/G13 Console members and their
input/monitor variants. Final closure reconciles all G01--G14/R01--R12, including
earlier M2 work now classified as M2 history. A candidate admission expands only
its named finite members into S work; no future numeric T is allocated here.
