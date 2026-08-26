# T479 S8 Fixed-Disk Controller Standardization Closure Audit

`M5:T479:S8:OWNER:OK`

## Scope Reconciliation

| Personality or boundary | Accepted result | Closure disposition |
| --- | --- | --- |
| `default-at` ATA PIO | One explicit ATA task-file personality, sole Core HDC state and VM logical-media owner. | Retained and passed `vm-hdc-port-smoke` and `vm-hdc-hdd-boot-smoke`. |
| DeskPro 386 Model 40 | One Compaq WD personality, IRQ14 and source-specific shared `3F7h` read behavior. | Retained and passed both Compaq HDC smokes plus Model-40 HDC smoke. |
| IBM 5170 Model 339 | One source-backed IBM WD1003/ST-506 Type-3 configuration, `3F6h` high-head function and source-qualified step selector. | Retained and passed `core-machine-hdc-smoke` and Model-339 composition smoke. |
| IBM 5160-268/Xebec | IBM primary ledger proves an 8-bit `320h`--`323h`, DMA3/IRQ5 controller. | No immutable XT profile/ROM receiver exists. The exact contract transfers to the ordered 5160 profile admission; no ATA shim, profile or false executable support was added. |
| WD1007A-WAH ESDI | Western Digital primary ledger proves a distinct AT-compatible controller and its host contract. | No current immutable machine profile selects it. The named controller/profile/drive/ROM receiver transfers; no generic ESDI or ATA alias was added. |
| Physical media/service timing | Source describes MFM/ESDI mechanics, ECC, cache, long/format/recovery and device signals beyond logical media. | Explicitly unsupported; Core does not fabricate a deadline or VM media representation. |

`rg -n "CORE_MACHINE_HDC_PROTOCOL_|core_machine_hdc_config|core_machine_hdc_topology" src tests CMakeLists.txt --glob "*.c" --glob "*.h" --glob "CMakeLists.txt"` finds only the three admitted Core personality constants and their Core/profile/test receivers.  No Xebec or ESDI placeholder/ATA alias exists in product code.

## Verification

At source commit `2bc20c52b92145253c0d7aa597aafa0d2645e9de`:

| Check | Result |
| --- | --- |
| Focused `ctest --test-dir build/mingw-gcc-x64 -R "core-machine-hdc-smoke|core-machine-compaq-hdc-s5-smoke|core-machine-compaq-hdc-machine-s5-smoke|vm-ibm-5170-model-339-composition-smoke|vm-hdc-port-smoke|vm-hdc-hdd-boot-smoke|vm-model40-hdc-s26-smoke" --output-on-failure` | 7/7 passed. `M5:T479:S8:REGRESSIONS:OK` |
| `cmake --build --preset current-gates-gcc` | Passed all 78 aggregate verification steps, including the 294-target current-gate registration and documentation governance. `M5:T479:S8:GATE:OK` |
| `Get-FileHash build/output/nxvm_0_5_0479.exe -Algorithm SHA256` | `1422E42CF5AAAC3ED86BB83E298B4759C82C9E362C1E0044436521245BEFED73`, equal to CURRENT. `M5:T479:S8:RELEASE:OK` |

The developer artifact remains the stripped Release produced and verified by S5;
S6--S8 made evidence/governance changes only and did not alter its source or
binary.  Across T479 implementation commits through the S7 baseline, the
bounded change is 445 insertions and 55 deletions, including 86 Core HDC lines,
profile/firmware/test proof and the required ledgers.  It adds no controller
framework or second state owner.

## Exact Transfers

- The IBM 5160-268 Xebec personality awaits its already ordered immutable XT
  profile/BYOB-ROM admission; it must consume the S6 primary ledger.
- WD1007A-WAH awaits a later named machine/controller/drive/ROM profile
  admission; it must consume the S7 primary ledger.
- Raw MFM/ESDI, ECC, long-sector, format/recovery, cache, mechanical signal and
  source-qualified full service-deadline work remain physical/controller
  receivers, not logical-media approximations.

T479 therefore closes the false universal-ATA abstraction while retaining only
implemented, source-backed personalities.  It does not claim that every
historical controller can be started before its machine profile is admitted.
