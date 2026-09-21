# T533 S1 Fixed-Machine Baseline Ledger

This is the finite convergence ledger for T533.  It records the observed
`4c40d46c` runnable baseline, not the target layout.  Its complete universe is
the four implemented machine profiles, retained CPU variants, every current
construction/configuration route, and the configured test/build receivers.
PC110 is deliberately outside this universe: it has no implementation or
runnable target and remains M6 work.

## Retained Product Universe

| Product/profile | Current factory and composition facts | Current external roles | Current coverage | T533 receiver/disposition |
| --- | --- | --- | --- | --- |
| IBM 5160 Model 268 (XT) | `vm_machine_create_xt_from_assets()` in `src/core/machine/machine.c`; board declaration and ROM mapping are `src/core/profile/xt/xt_5160_268.c` and `rom/xt_5160_268_rom.c`. Fixed 8088/no-FPU and 360 KiB drive policy. | Required system BIOS; optional second Xebec ROM and video ROM; no CMOS seed; optional text generator. | `vm-xt-5160-268-profile-smoke`; 5160 YAML boot row. | S2 moves source/test owner spelling; S3 moves final construction/provider ownership to `profiles/xt`; S4 replaces YAML with fixed XT build plus BYOB manifest binding. |
| IBM 5170 Model 339 (AT) | Generic PC/AT branch of `vm_machine_create_from_assets()` selects `vm_profile_ibm_5170_root_resolve_memory()` and fixed 1.2 MiB drive/CGA constraints from `default_profile/pc_at_profile.c`. | Two system-ROM chips and 64-byte CMOS seed required; video option ROM prohibited for the fixed CGA board; optional text generator. | 5170 resolver/composition/CGA/FDC tests; 360 KiB and 1.2 MiB YAML boot rows. | S2 renames profile/test owner; S3 makes `profiles/at` own direct composition instead of a resolver-derived branch; S4 creates fixed AT build/root-manifest route. |
| DeskPro 386 Model 40 | `vm_machine_create_model40_from_assets()` plus `model40_composition.c`; profile declarations and ROM layout are in `src/core/profile/model40/`. It alone supports two removable slots and a Model-40 HDC path. | Two system-ROM chips, video ROM and 64-byte CMOS seed required; optional text generator. | Model-40 composition/CMOS/FDC/HDC tests; 1.2 MiB YAML boot row and product console integration. | S2 source/test move; S3 transfers all board wiring/provider choice from Machine to `profiles/model40`; S4 fixed Model-40 target and manifest. |
| default PC/AT | Generic PC/AT branch calls `vm_machine_default_at_resolve()` from `default_profile/pc_at_profile.c`; it is the only current runtime-configurable CPU/FPU/memory/floppy format path. | One logical BIOS or two chip ROMs and 64-byte CMOS seed required; video option ROM and text generator are optional. | Default profile/ROM/app tests; 16 default-PC/AT CPU-by-floppy boot rows; FDD/HDD/Windows integration scenarios. | S2 source/test move; S3 flattens direct fixed board plan without recursive parent/provenance mirrors; S4 replaces runtime profile/CPU/firmware selection with one selected build while preserving the CPU test corpus. |
| PC110 | No profile implementation, firmware contract, target or boot claim exists. | None admitted. | No runnable coverage. | Explicitly retained for M6; S2--S5 must not create a placeholder directory, executable or asset claim. |

### CPU Preservation

`src/core/core/cpu_interface.h` retains `DEFAULT`, 8086, 8088, 80186, 80286
and 80386.  The current App YAML parser exposes 8086/80186/80286/80386 and
the matching 8087/80287/80387 choices; XT fixes 8088 directly.  The sixteen
default-PC/AT boot rows cover the four App-exposed CPU values and four floppy
formats.  8088 remains covered by Core ledgers and the XT board; 80188 and 486
are not current Core model entries and cannot be claimed by T533.  S2 preserves
all Core CPU code/tests; S4 removes only runtime product CPU selection from the
fixed-executable path.

## Current Construction And Configuration Routes

| Route | Current sole entry/owner | Observed incompatible duplication | Required disposition |
| --- | --- | --- | --- |
| User session selection | `src/app/catalog.c` parses YAML files into `vm_session_request`; `src/app/config.c` parses profile, CPU, FPU, firmware, media, display and memory into `vm_machine_config`. | One YAML document selects board identity and every protected firmware path, contrary to fixed-profile products. | S4 replaces the production catalog/YAML path with one NXVM.ini parser. Unit parser values stay code-owned; no YAML compatibility path remains. |
| File-backed assets | `vm_machine_create()` -> `vm_machine_file_assets_load()` in `src/core/machine/machine.c`; it copies files into `vm_machine_assets`. | File-size rules and profile-kind switches live in Machine instead of the board Profile. | S3 moves slot/mapping/size knowledge to Profiles behind a neutral asset service; S4 consumes exactly one CMake-bound external root and validated profile manifest. |
| Direct test construction | Repository-only tests call `vm_machine_create_from_assets()` with literal byte views. | This is intentionally distinct from user firmware loading, not a second production path. | Retain as the code-owned unit seam; S3 preserves it without external assets or INI/YAML dependencies. |
| Board construction | `vm_machine_create_from_assets()` branches to XT, Model-40, default PC/AT or 5170; `machine_private.h` stores simultaneous board-specific fields and firmware kind. | Machine owns four board choices, copied ROM buffers and firmware-provider selection. | S3 replaces branch/four-board storage with one build-selected Profile plan and selected-resource lifetime; retain one Machine/Common adapter. |
| Firmware provider binding | `src/core/machine/lifecycle.c` switches `firmware_kind` among Model-40, XT and external PC/AT providers. | Adapter makes board policy decision after construction. | S3 Profile provides the fixed neutral firmware binding during plan construction; lifecycle binds once. |
| Runtime media | `vm_machine_config` has ordered floppy/fixed-disk slots and `lib_storage_medium_mode`; FDD/HDD adapters live in `src/core/machine/media/`. | None: geometry/change behavior is genuinely device-semantic, while Lib Storage owns file modes. | S3 retains adapters and one Lib Storage route; S4 NXVM.ini supplies only ordered media paths/modes. |

## Current Build And Test Universe

| Surface | Baseline fact | T533 receiver |
| --- | --- | --- |
| Sources | `CMakeLists.txt` builds `core-machine`, `vm-profile`, `vm-media`, `vm-machine` and `vm-app`; profile sources are singular `src/core/profile`, generic execution is `src/core/core`. | S2 makes one atomic source/test/CMake rename to `core/devices` and `core/profiles`, removing former owner spellings. |
| Product executable | The CMake baseline uses a single current `vm-0-5-0532` executable and hard-codes `assets/sessions/nxvm_0_5_0532_<arch>.exe` for product integration. | S4 parameterizes one selected-profile executable per build, validates `NXVM_PROFILE_ASSETS_ROOT`, and removes the current runtime selector rather than adding a parallel executable path. |
| Repository-only tests | `test/app`, `test/core/core`, `test/core/machine` and `test/core/profile` mirror existing owners.  CPU/controller tests and direct-asset construction are repository-only. | Singular Profile and generic-Core names must move with their source; literal assets must not become BYOB inputs. | S2 relocates only matching owners; S3/S4 update construction expectations without reducing the CPU/controller corpus. |
| External integration | `test/integration/support/session_yaml.c` opens YAML; CMake registers 20 historical boot rows plus default FDD/HDD/Windows/product cases using `assets/sessions`. | Integration is still a separate YAML loader, and existing external assets remain mandatory. | S4 migrates every retained scenario through NXVM.ini and the same external asset root/overlay route; no media copying, embedded firmware or narrowed matrix. |

## Frozen Contracts And Completion Predicate

1. Each future runnable product has exactly one selected Profile and one
   `NXVM_PROFILE_ASSETS_ROOT`; the Profile manifest is the sole firmware/CMOS/
   font role, relative-path, size and SHA-256 authority.  CMake validates the
   selected manifest and emits an ignored local root binding.  Vendor bytes are
   neither committed nor embedded.
2. NXVM.ini is shared across selected builds.  It accepts only runtime memory,
   ordered removable/fixed media with Direct/Readonly/Overlay modes, display
   and existing `console_control` behavior.  It selects neither machine, CPU,
   topology, firmware, boot order nor automatic start.
3. S2 may change paths only; S3 consumes the complete construction/provider
   rows above; S4 consumes the complete YAML/build/integration rows; S5 proves
   there is one construction, reset, asset, media and presentation route per
   selected product.  Any row not consumed is an explicit T533 stop condition.

The S1 batch is complete when this fixed table is reconciled with the tracked
source/CMake/test paths listed above.  It is a migration ledger, not evidence
that the future fixed-build/INI implementation exists.

## Verification

- The ledger was cross-checked against the tracked App, Core, CMake and test
  routes named in each row; it does not inspect, copy or claim ownership of
  external BYOB payloads.
- An isolated x64 CMake configure/build completed 832 targets, followed by the
  complete repository-only unit suite: 336 of 336 tests passed in 226.85
  seconds on 2026-09-21.  Existing `src/x86/debug/command.c` format-truncation
  warnings remain covered by their established targeted rule.
- `git diff --check` and the documentation governance gate pass.  A stale
  pre-existing build tree that lacked current test executables is excluded from
  this evidence; it was not used for the acceptance result.
