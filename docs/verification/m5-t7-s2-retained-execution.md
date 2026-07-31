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
`4155367c28635b44973e06257ccf88cc1366b55285e418e0ad77a16dcd963342`. It
emits `0.4.015d.m5t7`; scripted `help`, `info`, and `exit` commands returned
zero. This is closure evidence, not a release artifact.
