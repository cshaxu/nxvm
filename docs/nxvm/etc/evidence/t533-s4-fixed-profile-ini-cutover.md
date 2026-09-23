# T533 S4 Fixed-Profile INI Cutover
S4 removes the production session catalog and its YAML product variants.  A
build now selects exactly one frozen Profile, and the runtime reads exactly one
adjacent `NXVM.ini` for host-facing settings only.  The Core CPU corpus remains
available to repository-only tests; it is no longer a runtime product selector.

## Fixed Product Set

| Build-selected Profile | Frozen machine facts | Runtime INI |
| --- | --- | --- |
| `ibm-5160-model-268-360k` | IBM 5160 XT, 8088, 360 KiB FDD | `ibm-5160-model-268-360k.ini` |
| `ibm-5170-model-339-1200k` | IBM 5170 AT, 80286, 1.2 MiB FDD | `ibm-5170-model-339-1200k.ini` |
| `compaq-deskpro-386-model-40-1200k` | DeskPro 386 Model 40, 80386, 1.2 MiB FDD | `compaq-deskpro-386-model-40-1200k.ini` |
| `default-pc-at-80386-1440k-hdd` | default PC/AT, 80386, 1.44 MiB FDD and HDC | `default-pc-at-80386-1440k-hdd.ini` |

`NXVM_PRODUCT_PROFILE` accepts only these four values.  The generated local
binding selects Profile, CPU/FPU, firmware roles and external BYOB paths.  INI
accepts only memory, presentation and ordered floppy/fixed-disk paths with
their access modes.  It cannot select board, CPU, firmware, boot order or an
automatic start action.

The adjacent INI is copied unchanged.  Its relative media paths are relative
to that INI, so rewriting `../../../nxvm-assets` for a build/output directory
would incorrectly point at `nxvm/nxvm-assets`; that obsolete rewrite was
removed.  This leaves one path-resolution rule for source templates, build
trees and deployed artifacts.

## Similar-Issue Sweep

- `assets/sessions/*.yaml`: no production session document remains.
- `src/app/catalog.c`: removed; `src/app/ini.c` is the only product parser.
- Integration support: `session_ini.c` uses the same App INI parser and opens
  external media through the normal overlay route; it does not copy media.
- `verify_t533_integration_ini_boundary.cmake` rejects YAML product sessions,
  firmware/profile keys in INI and unit tests that load external assets.
- `verify_t344_unit_test_registration.cmake` now proves the selected build has
  exactly one INI boot row, rather than preserving the retired twenty-row YAML
  matrix assumption.

## Verification on 2026-09-21

- `cmake -DPROJECT_SOURCE_DIR:PATH=. -P cmake/verify_t533_integration_ini_boundary.cmake`:
  pass.
- `verify-t344-unit-registration`: pass, one selected INI boot row.
- `ctest --test-dir build/t533-s4-make-x64 -L unit -j 4 --output-on-failure`:
  336/336 pass.
- Real fixed-product boot probes:
  - XT/360 KiB: `installer-running`, 55.08 seconds.
  - 5170/1.2 MiB: `installer-running`, 89.90 seconds.
  - default PC/AT/1.44 MiB plus HDC: `dos-prompt`, 3.39 seconds.
  - Model 40/1.2 MiB: console and CMOS rows pass; the real boot row times out
    at 180.08 seconds while FDC command `E6` is still active.  This is retained
    as an explicit boot-chain failure; it is not a passing result or a retired
    product.
