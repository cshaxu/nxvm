# T556 S1 — Five-App Layout Inventory

## Scope And Baseline

This inventory records the source, test and build ownership before any move.
It was taken from the T555 closure baseline plus the admitted T556 planning
files. It changes no production or shared-six file. The target order is
MyNES, My5160, My5170, NXVM, then DeskPro386; the last entry deliberately
reserves the only behavior-splitting move for after the mechanical layouts.

## Ownership Decisions

### MyNES

| Current owner/files | Destination | Reason |
| --- | --- | --- |
| `core/{opcode,bus,cpu,ppu,apu,controller,cartridge,machine,snapshot}.*` | `src/app-mynes/core/` | Deterministic guest execution, devices and persisted snapshot encoding. |
| `core/{driver,media,debug}.*` and their interfaces | `src/app-mynes/machine/` | The sole adapter from Emulator Machine requests to Core execution, input, copied presentation/audio, host media and paused-debug operations. |
| `product/*` | `src/app-mynes/product/` | Product configuration, monitor commands and product composition. |

The apparent move is not safe as a rename alone. `driver.c` currently reaches
PPU, APU and Controller fields inside `core_machine`; `media.c` reaches
Cartridge fields and mutable battery-dirty state; `debug.c` already mostly
uses Core public operations. S2 therefore establishes one bounded Core machine
service boundary for the exact Machine needs: copied frame/audio observation,
controller input, battery import/export/acknowledgement and existing debug
operations. It must not export `core_machine` layout, component pointers or a
generic getter family. Core remains independent of Emulator; Machine depends
on Core plus public Emulator/Lib contracts.

`test/app-mynes/unit/core/` stays with deterministic instruction/device,
cartridge and snapshot assertions. Driver, media, pacing, presentation/audio
adapter and debug-router tests move to `test/app-mynes/unit/machine/`.
`unit/product/`, integration, diagnostic and support retain their existing
route categories; only source-owner subdirectories change. `cmake/mynes/
MyNesProduct.cmake` gains one Machine traversal and updates its aggregate
target list.

### My5160

| Current source | Flat destination |
| --- | --- |
| `product/main.c` | `main.c` |
| `product/machine_binding.c`, `binding_interface.h` | `product_binding.c`, `product_binding_interface.h` |
| `profiles/machine_plan.c`, `construction_interface.h` | `machine_plan.c`, `machine_construction_interface.h` |
| `profiles/xt_5160_268.c,h` | `machine_config.c,h` |
| `profiles/rom/xt_5160_268_rom.c` | `firmware_map.c` |

The fixed model identity remains in public symbols and asset keys; only the
misleading directory ownership disappears. `test/app-my5160/unit/profiles/`
becomes flat `unit/`; integration keeps scenario grouping, not a fake source
component. The App CMake source list and product-entry/binding paths change
once, with no Core or shared component edit.

### My5170

| Current source | Flat destination |
| --- | --- |
| `product/main.c` | `main.c` |
| `product/machine_binding.c`, `binding_interface.h` | `product_binding.c`, `product_binding_interface.h` |
| `profiles/machine_plan.c`, `construction_interface.h` | `machine_plan.c`, `machine_construction_interface.h` |
| `profiles/pc_at_profile.c`, `profile_interface.h` | `machine_config.c`, `machine_config_interface.h` |

Firmware remains external owner-provided input selected by the fixed App
CMake request, not a source subcomponent. Profile-owned unit tests flatten to
`test/app-my5170/unit/`; support and integration remain categorized by test
role. No behavior or firmware selection changes.

### NXVM Default AT

| Current source | Flat destination |
| --- | --- |
| `product/main.c` | `main.c` |
| `product/machine_binding.c`, `binding_interface.h` | `product_binding.c`, `product_binding_interface.h` |
| `profiles/machine_plan.c`, `construction_interface.h` | `machine_plan.c`, `machine_construction_interface.h` |
| `profiles/pc_at_profile.c`, `profile_interface.h` | `machine_config.c`, `machine_config_interface.h` |
| `firmware/*` | unchanged `firmware/` |

Only `firmware/` remains a source component because it contains the
project-authored firmware builder and source assets. The remaining nesting is
purely historical. Its tests become direct `test/app-nxvm/unit/` entries
except `unit/firmware/`, which remains paired with that component. Existing
integration/diagnostic scenario directories remain test categories rather
than product-source components.

### DeskPro386

| Current source | Target owner | Required division |
| --- | --- | --- |
| `product/{main,machine_binding,binding_interface}` | `product/` | Process entry and family Product binding only. |
| `profiles/d4_memory.*`, D4 register/control-only portions | `chips/` | Model-specific state/register semantics with no board composition dependency. |
| `profiles/{machine_plan,model40,composition,construction_interface,composition_interface,observation_interface,rom/model40_rom}` and D4 wiring portions | `machine/` | Model assembly, port/memory registration, PIT/KBC/refresh/HOLD wiring, construction and firmware map. |

`d4_platform.c` cannot move wholesale: its state and callback routes mix D4
semantics with Board integration. S6 first extracts only a real D4 chip state
owner, then lets Machine own existing Core board wiring. There will be no
Machine-to-Chips reverse include and no generic D4 framework. D4 register
tests move to `unit/chips/`; refresh, port route, plan, Model40, FDC/HDC/DMA,
composition and ROM tests move to `unit/machine/`; entry/binding tests use
`unit/product/`. Test-only support remains under `test/app-mydeskpro386/
support/` because it is shared by those three test owners, not production.

## Build, Gate And Documentation Callers

The migrations must update these direct registrations instead of retaining
compatibility directories:

| Caller | Current responsibility |
| --- | --- |
| `cmake/mynes/MyNesProduct.cmake` | MyNES Core/Product traversals and unit aggregate targets. |
| `src/app-my5160/CMakeLists.txt` | XT source list and Product binding/entry paths. |
| `src/app-my5170/CMakeLists.txt` | AT source list and Product binding/entry paths. |
| `src/app-mydeskpro386/CMakeLists.txt`, `cmake/nxvm/NxvmProduct.cmake` | Model40 source list, Product binding/entry paths and static ownership gates. |
| `src/app-nxvm/CMakeLists.txt` | Default-AT source list, Product binding/entry paths and firmware retention. |
| `cmake/nxvm/verify_*.cmake` | Explicit source paths for board/locality/D4/owner checks; rename them with the actual owner paths, not broad globs. |
| Active layout docs | Current source/test component descriptions only; historical evidence remains unchanged. |

## Verification And Safety Contract

Each S uses `git mv` for mechanical relocations, replaces every include/CMake/
gate path in the same batch, refreshes the affected manifests and executes the
affected x64/x86 unit routes. A completed App source batch rebuilds its current
artifact pair. S7 performs one reused dependency-graph build of all ten App
EXEs and full repository unit/setup/integration qualification per width.

No S modifies `src/lib`, `src/emulator`, `src/x86`, `src/ibmpc` or their tests.
Owner INIs, media and snapshots remain untouched. A required change there
stops the batch for an explicit task decision.
