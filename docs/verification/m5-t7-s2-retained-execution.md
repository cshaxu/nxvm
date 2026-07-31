# M5 T7 S2 Retained Execution Verification

The retained bridge was exercised through the final product Console with the M1
local fixtures and instruction recording enabled. Each run used a twelve-second
external wall-clock budget; timeout is the intended bounded stop and no guest
media was modified.

| Boot target | Local trace result | Final observed execution |
| --- | --- | --- |
| FDD | ignored `build/m5-t7-fdd.trace`, 691,960,482 bytes | active DOS/BIOS instruction stream at timeout |
| HDD | ignored `build/m5-t7-hdd.trace`, 250,218,420 bytes | active DOS instruction stream at timeout |

Both fixture identities were previously revalidated in
`m5-t5-s2-pc-at-block-composition.md`. The external timeout leaves no NXVM
process running. `nxvm-m5-t7.exe` receives the product Console commands, creates
the product-owned PC/AT session, then calls the retained bridge through
`products/nxvm/pc_at`; the entry no longer calls the baseline Console.

The ignored local task artifact is `build/output/nxvm-m5_t7.exe`, SHA-256
`fa22dec38c58ca3b8fe914c48cdf3c96c06af5b07c713d1c6f218c23b8392d4d`. It
emits `0.4.015d.m5t7`. A scripted window-mode FDD run reported `running`,
accepted `stop`, returned to `ready`, exited zero, and left no process running.
The focused PC/AT smoke also covered stopped-state display selection, reset,
memory configuration, and an observed non-running state for both fixture
targets. Product media/session smokes also exercised frozen created FDD and HDD
providers. A created FDD was reset then removed through `remove <path>` into
the ignored build directory, producing 1,474,560 bytes. The complete focused
GCC smoke suite passed, including CPU probes, core machine/device/presentation,
firmware, registries, product adapters, sessions, and both local fixture
profile paths. `nxvm-product-debug-console-smoke` launches the artifact through
Win32 pipes, waits until its debugger prompt is active, submits `q`, then
proves the returned product Console reports `debugger-paused` and exits zero.
This is closure evidence, not a release artifact.

## M5 Exit Audit

CPU capability claims and the normalized retained-baseline `#UD` probe are
covered by `m5-t1-s1-cpu-probes.md`; optional Bochx remains excluded from
default builds and products. The canonical profile/provider registry, Firmware
V1 package, PC/AT reset image, media policy, Console state adapter,
presentation adapter, and debugger adapter have focused evidence in the M5 T2
through T6 records.

The final artifact's product Console supports configured RAM/display/media/
boot/recording, created media, optional media export, lifecycle actions, and
the retained debugger. The complete focused GCC smoke matrix passed, including
core, CPU, device, firmware, registry, media, Console, presentation/debugger,
PC/AT, session, and fixture-profile tests. `git diff --check` and `git diff
--exit-code -- src/nxvm-baseline` passed. Core and product dependency scans
found no direct `nxvm-baseline` include in product code and no forbidden
product/firmware/platform/DOS include in core. No guest media, Microsoft
binary, generated trace, or local artifact is tracked; default targets do not
include Bochx research.

This record demonstrated product composition and bounded regression behavior,
not completion of the M5 migration goal. The user-facing execution loop still
entered `src/nxvm-baseline/` through the temporary adapter, so M5 was reopened
before M6. The historical snapshot remains a checkpoint only.
