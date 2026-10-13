# Five-App Component Layout

## Goal And Admission

Owner admitted M5 T556 on 2026-10-12 after T555. Restructure the five Apps
around their actual responsibilities, preserving machine behavior, saved data,
configuration and the existing shared runtime. This task is hosted by NXVM;
MyNES implementation and documentation use separate MyNES-target commits with
the same T556 identifier. Shared root build references, if necessary, use
separate Shared commits. Shared six source/test trees are outside this task.

## Target Owners

```text
src/app-mynes/{core,machine,product}
src/app-my5160/                 flat
src/app-my5170/                 flat
src/app-mydeskpro386/{chips,machine,product}
src/app-nxvm/                   flat, except firmware/
```

MyNES Core retains deterministic CPU/bus/PPU/APU/controller/cartridge execution
and snapshot encoding. Machine owns the Emulator driver, input/output adaptation,
host pacing/audio delivery, file-backed media/battery operations and executor-side
debug protocol dispatch. Product owns configuration, commands and composition.
Dependency direction is Product -> Machine -> Core; Machine also consumes public
Emulator/Lib contracts. Core must not depend on Machine or Emulator.

The present `core/driver.c`, `core/media.c` and `core/debug.c` are the principal
Machine candidates. Their direct access to `core/machine.h`, PPU, APU and cartridge
state requires a real boundary repair. Inventory every access and reuse existing
Core operations; add only consumer-required bounded operations or copied values.
Do not expose mutable Core layout, add a generic getter layer, or move deterministic
snapshot serialization out of Core merely because Machine transports its bytes.

My5160/My5170/default have thin bindings and fixed plans, so flatten their
product/profiles/rom source paths. Use responsibility filenames such as `main.c`,
`product_binding.c`, `machine_plan.c`, `machine_config.c`, and `firmware_map.c`,
with corresponding headers where used. Final names follow actual contents;
remove generic or redundant names without introducing compatibility headers.
Default's project-authored firmware and build source stay in `firmware/`.

DeskPro Chips owns model-specific register/state logic, including D4 memory and
control mechanisms. Machine owns port/memory registration, PIT/KBC/refresh/HOLD
wiring, construction/reset/teardown, the fixed plan and immutable firmware mapping.
Product retains the thin process entry and family Product binding. Existing
`d4_platform.c` and `d4_memory.c` mix these roles and must be divided by behavior,
not moved wholesale into Chips. Chips must not depend on Machine/board composition;
Machine integrates Chips with the existing Core public board contracts. Generic
CPU/PIT/video implementations remain in Core, with no App-local copies.

## Test, Build And Naming Contract

Keep unit/integration/diagnostic/support route categories. Within unit tests,
mirror real source owners: MyNES `{core,machine,product}`, DeskPro
`{chips,machine,product}`, flat My5160/My5170 and default with `firmware/` retained.
Move assertions according to behavior, including driver tests currently under
MyNES Core. Preserve independent assertions and fixture ownership.

Update includes, guards, public symbols belonging to moved owners, CMake targets,
test names/labels, dependency gates, tools and current architecture/layout docs.
Do not rename unchanged shared/Core symbols or rewrite historical evidence.
Retire old paths and duplicate source registrations in the same App batch.
Keep original Ninja/ccache trees and compile shared dependencies once per width.

## Ordered S Plan

| S | Scope | Exit proof |
| --- | --- | --- |
| S1 | Design and complete migration inventory | Current files, callers, private accesses, targets and tests mapped to destination owners; boundary decisions and expected diff recorded before implementation. |
| S2 | MyNES Machine extraction | Core has no adapter dependency; Machine uses approved Core public operations; snapshots, battery, debug, audio and presentation retain behavior; dual-width App tests and pair updated. |
| S3 | My5160 flattening | No product/profiles/rom source subdirectories; clear filenames, repaired references and matching tests; dual-width qualification and pair updated. |
| S4 | My5170 flattening | Same flat-source and reference proof, with AT-specific behavior retained; dual-width qualification and pair updated. |
| S5 | NXVM flattening except firmware | Firmware remains cohesive; other source files flat with clear roles; tests/build/docs updated; dual-width qualification and pair updated. |
| S6 | DeskPro Chips/Machine/Product split | Register logic and board integration have separate owners; no reverse dependency; D4/KBC/PIT/HOLD/ROM tests preserved; dual-width qualification and pair updated. |
| S7 | Whole-task audit and delivery | No retired path/symbol/reference remains in active code; all repository suites pass per width; ten current EXEs verified; target-separated commits pushed and worktree clean. |

This is an initial ordered plan. Any necessary refinement must preserve the whole
five-App scope and linear numeric S identifiers. Each implementation S reports its
file map and estimated additions/deletions, then actual diff and verification.

## Accepted S2 Record

MyNES S2 is accepted in `693030c2e`. The adapter/media/debug files and their
tests now live under `src/app-mynes/machine` and
`test/app-mynes/unit/machine`; Core retains deterministic MMC3 behavior tests.
Core exposes only the copied/direct operations actually consumed by Machine for
audio, completed video samples, input buttons, battery persistence, debug and
bounded ROM-image normalization. Machine no longer includes Core's private
cartridge or machine layout. MyNES x64/x86 CTest passes 57/57 on each width,
including 45 unit and 12 integration routes; the current 0044 pair was rebuilt.

## Accepted S3 Record

My5160 S3 flattens its fixed XT App in the accepted inventory shape: `main`,
`product_binding`, `machine_plan`, `machine_config`, `firmware_map`, and their
interfaces now live directly in `src/app-my5160`; its two unit sources now live
directly in `test/app-my5160/unit`. The target is renamed `my5160-machine`.
The common profile-build directory derivation now correctly recognizes both an
App-root entry and a retained `product/` entry, so embedded firmware remains
owned by the selected App during incremental flattening. No Shared-six or Core
implementation changed. Core/test-core manifests now record the moved trees.
My5160 x64/x86 units, manifest/corpus/ownership gates, session-status route and
actual floppy installer boot each pass; the x64/x86 artifact pair was rebuilt.

## Completion And Stop Conditions

Every current production/test file has a destination or justified retention;
all five target trees, directional dependencies and relevant names match this
design. Existing user-visible behavior and snapshot/INI/media formats are retained.
Run affected x64/x86 tests per batch, then final full repository units and registered
integrations once per width. Rebuild/deploy the ten current App EXEs under artifact
rules, preserving owner inputs. Review actual changes before closing each S/T.

Stop for a required shared-six implementation change, guest behavior change,
incompatible persisted format, new runtime manager or unexplained loss of tests.
S1 is design/inventory only; this admission does not claim a completed migration.
