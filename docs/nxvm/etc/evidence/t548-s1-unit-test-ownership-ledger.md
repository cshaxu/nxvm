# T548 S1 Unit-Test Ownership Ledger

## Frozen Baseline

- Task-admission commit: `075159001`.
- Production/test baseline: the committed tree at admission; no test source,
  registration, fixture or production file has been moved, merged or removed
  by S1.
- Enumeration command for C unit-entry candidates:

  ```powershell
  rg --files test -g '*.c' |
    Where-Object { $_ -notmatch '\\fixtures\\' -and $_ -notmatch '\\integration\\' }
  ```

- Enumeration command for registration/support source:

  ```powershell
  rg --files test -g '*.cmake' -g 'CMakeLists.txt'
  ```

`*_smoke.c` naming alone is not evidence that a file is a registered unit.
Registration is determined by the applicable CMake entry point and the final
CTest graph. Integration, native-desktop and external-media routes remain
outside this S1 unit inventory unless a later S records them as consumers.

## Entry Universe

| Candidate owner root | C entries excluding fixtures/integration | Initial disposition |
| --- | ---: | --- |
| `test/lib` | 37 | Canonical Lib corpus; inspect only for duplicate assertions or misplaced fixtures. |
| `test/emulator` | 17 | Canonical Emulator corpus; retain composition and monitor evidence pending case mapping. |
| `test/product` | 9 | Canonical shared PC Product corpus; retain Debug/Xasm32/Surface evidence pending consumer comparison. |
| `test/core` | 350 | Canonical NXVM Core/chip/board/machine corpus; split only where a test proves a selected App profile rather than Core behavior. |
| `test/app-nxvm` | 55 | Default-PC App/profile and genuine App-composition candidates; several machine tests require owner review. |
| `test/app-my5160` | 1 | 5160 profile increment. |
| `test/app-my5170` | 3 | 5170 profile/firmware/composition increments. |
| `test/app-mydeskpro386` | 25 | DeskPro Model 40/D4/profile increments. |
| `test/app-mynes` | 44 | MyNES App-owned CPU/PPU/APU/mapper/product increments. |
| **Total** | **541** | Every entry remains retained until a later S records its exact receiver. |

There are 34 tracked CMake/CMakeLists test-registration or test-support files
outside fixtures. The full repository also has 45 C files below `integration`;
they are explicitly excluded from the unit-entry count, not deleted or
relabeled.

## Registration Owners

| Owner | Registration authority | What it may register |
| --- | --- | --- |
| Lib | `test/lib/CMakeLists.txt` | Lib units and Lib static-contract checks. |
| Emulator | `test/emulator/CMakeLists.txt` | Emulator units and its finite corpus/manifest gates. |
| Product | `test/product/CMakeLists.txt` | Shared PC Product units and Product gates. |
| Core | `test/core/CMakeLists.txt`, `x86_tests.cmake`, `board_tests.cmake` | Chips, x86, boards, machine and Core Product units. |
| NXVM Apps | `cmake/nxvm/NxvmProduct.cmake` plus App integration registrations | Selected profile/App and external consumer increments only. |
| MyNES | `test/app-mynes/unit/{core,product}/CMakeLists.txt` | MyNES Core and App Product units. |

`test/register.cmake` is shared registration mechanics only; it does not own
test behavior. `test/verify_test_boundaries.cmake` proves finite include/link
direction, not semantic assertion ownership.

## Confirmed And Candidate Findings

| ID | Evidence | Status and required receiver decision |
| --- | --- | --- |
| L1 | `test/app-nxvm/unit/product/nxvm_ini_smoke.c` tested `vm_app_configure_machine(..., NULL, ...)` clearing output. `test/core/product/factory_smoke.c` already covers the same Core factory null-request/output-clearing contract. | **Resolved by S2.** The App file and both `vm-app-ini-smoke` registrations are removed. Reconfigured x64/x86 CTest graphs retain only `core.factory`, which passes on both widths. |
| L2 | 79 test C files include a `core/*_private.h`; 53 of those are PC-App unit entries. | **Review trigger, not a violation.** Selected-profile and board-wiring proof can legitimately need a private fixture. S2-S4 must map every App assertion to a genuine selected-profile increment or a Core receiver. |
| L3 | `test/app-nxvm/unit/board/{cpu_bus_boundary_negative,fdc_boundary_negative}.cmake` are product-registered static gates that copy Core source/test inputs. | **Candidate misownership.** S2 must decide whether each is a Core boundary gate with an App-specific increment, or a true App assembly gate. No copy/alias is permitted. |
| L4 | 31 test source paths retain historical task markers such as `_s20`, `_s28` or `_t242`. | **Naming debt.** S5 may rename only after each test's behavior/receiver is documented; task history belongs in evidence, not target/file/output identifiers. |
| L5 | Default App machine tests contain generic cancellation, runner, media, display, FDC/HDC and timing mechanisms beside selected-PC topology checks. | **Candidate mixed ownership.** S2/S3 will identify Core-only assertions versus default-PC construction/wiring increments before any split. |
| L6 | 5170 and Model 40 tests use the same Core private construction surfaces while asserting ROM, CMOS, D4, CECG, drive geometry, refresh and model-specific routing. | **Likely legitimate profile increments.** Retain unless a specific assertion is duplicated in the same execution context by Core. |
| L7 | Shared Emulator/Product and App MyNES/PC monitor tests coexist. | **Candidate consumer overlap.** S5 must preserve shared grammar/format tests in Emulator/Product and retain only App delegation, preflight, extension, Debug and snapshot increments above them. |

## S1 Disposition

S1 closed after freezing the reproducible entry/registration universe and its
first bounded semantic candidate groups. It does not claim that source-file
names or links prove runtime coverage. Each later S must expand its affected
candidate group to a behavior-level mapping: present registration/label,
production function, input/failure predicate, canonical owner, any required
higher-layer increment, and explicit retain/move/split/remove/defer decision.
No group other than L1 is authorized for relocation by this record.
