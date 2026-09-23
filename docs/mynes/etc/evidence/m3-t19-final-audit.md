# M3 T19 Final Audit

## Basic-Use Outcome

MyNes has a versioned x64 and x86 local developer executable under
`build/output/`, with the required editable `mynes.ini` beside each executable.
The configuration selects a local mapper-0 ROM and `window` or `console`.
The process starts in the cooked monitor, `run` starts the attached machine,
and `quit` performs the ordinary shutdown path. Window and Console gameplay
use the same machine, controller mapping and lifecycle; their presentation
representation is selected only at startup.

This is a basic-use qualification, not a compatibility claim. The supported
profile is NTSC-oriented mapper-0/NROM without APU audio, PAL, expansion devices,
save states, rewind or commercial-ROM certification.

## Current Direct Evidence

| Required result | Direct current proof |
| --- | --- |
| Startup configuration and deployment | `test/app/config_smoke.c` parses `rom` and both video values; the product CMake copies the versioned executable and its required `mynes.ini` to `build/output/`. |
| Cartridge and CPU/PPU path | T11's original NROM fixture drives production CPU/MMIO, mapper-0 cartridge and PPU; T12's timing receivers extend the production slot, DMA, NMI and frame path. |
| Controller and command lifecycle | T13's controller/command receivers cover serial input, reset/drain and cooked command transitions. |
| Native Window | `mynes.integration.native-window-smoke` reads a real `LibKvmWindow` pixel, changes it through native K=A input, checks F5/F12 and closes/tears down safely on x64/x86. |
| Native Console | `mynes.integration.native-console-smoke` reads the selected `CONOUT$` 80x25 buffer, injects raw K=A/F5/F12 through `WriteConsoleInputW`, observes changed Core text cells, returns through the cooked monitor and tears down safely on x64/x86. |
| Build and regression | Fresh x64 and x86 JUnit suites each contain 103 tests with zero failures and zero disabled tests; strict product warning policy remains enabled by the root/App/Core CMake targets. |
| Delivery artifacts | `mynes_0_1_0011_x64.exe`: `704E52E31B867BAC8053DF1CA4C6FC117A402E15345BBB07E20158C7967F9010`, PE x86-64. `mynes_0_1_0011_x86.exe`: `EA0D634813859A10DC194A780EE3B5AACADEF1835F87D7A1549EFC7E4F975B27`, PE i386. |
| Reuse boundary | `git diff 9d3142d..HEAD -- src/lib src/common test/lib test/common` is empty. The adopted four roots remain unchanged after their approved M1 refresh. |

## Remediation Ledger Reconciliation

| Ledger members | Final proof owner |
| --- | --- |
| R01--R06, R08--R10 | T11 vertical implementation and its Core/App/integration receivers; T13 completes input-state and command lifecycle variants. |
| R07 | T12 production slot-clock, DMA and PPU/NMI timing receivers. |
| R04 presentation teardown | T16 native Window and T18 native Console receivers. |
| R11 | T17 strict App/Core review and T18's owner-local Console receiver; source layout remains flat `src/core`/`test/core`. |
| R12 | T17 strict release qualification, refreshed by T19's generated versioned output artifacts and full dual-architecture JUnit suites. |

The historical G01--G14 implementation records are retained in T11--T13. T16
and T18 supply the missing native presenter observations; T17 and T19 supply
the current build/artifact proof. No ledger member depends on an unreviewed
Lib/Common modification.
