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
src/app-mydeskpro386/           flat (S9 owner-approved revision)
src/app-nxvm/                   flat, except firmware/
```

MyNES Core retains deterministic CPU/bus/PPU/APU/controller/cartridge execution
and snapshot encoding. Machine owns the Emulator driver, input/output adaptation,
host pacing/audio delivery, file-backed media/battery operations and executor-side
debug protocol dispatch. Product owns configuration, commands and composition.
Dependency direction is Product -> Machine -> Core; Machine also consumes public
Emulator/Lib contracts. Core must not depend on Machine or Emulator.

At admission, `core/driver.c`, `core/media.c` and `core/debug.c` were the principal
Machine candidates. Their direct access to `core/machine.h`, PPU, APU and cartridge
state required a real boundary repair, delivered in S2. Inventory every access and reuse existing
Core operations; add only consumer-required bounded operations or copied values.
Do not expose mutable Core layout, add a generic getter layer, or move deterministic
snapshot serialization out of Core merely because Machine transports its bytes.

My5160/My5170/default have thin bindings and fixed plans, so flatten their
product/profiles/rom source paths. Use responsibility filenames such as `main.c`,
`product_binding.c`, `machine_plan.c`, `machine_config.c`, and `firmware_map.c`,
with corresponding headers where used. Final names follow actual contents;
remove generic or redundant names without introducing compatibility headers.
Default's project-authored firmware and build source stay in `firmware/`.

The owner's S9 amendment supersedes the S6 DeskPro directory split: put all
DeskPro production C/H files directly under `src/app-mydeskpro386`, without
`chips`, `machine` or `product` subdirectories. Use responsibility-based filenames
for D4 register/decoder logic, Core attachment and wiring, fixed plan/configuration,
firmware mapping, product binding and entry. Preserve clear ownership within this
flat App; do not manufacture standalone chip APIs merely to preserve the retired
directory boundary. Core routes still have one attachment/rollback owner, and D4
registers one state owner. Generic CPU/PIT/video implementations stay in Core.

## Test, Build And Naming Contract

Keep unit/integration/diagnostic/support route categories. Within unit tests,
mirror real source owners: MyNES `{core,machine,product}`, flat DeskPro,
flat My5160/My5170 and default with `firmware/` retained. DeskPro unit sources
move directly under `test/app-mydeskpro386/unit`; retain integration, diagnostic
and genuine support categories, not obsolete component nesting.
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
| S7 | DeskPro386 normal-boot qualification | Compare the pre-layout Model 40 boot route with current behavior, repair the proven Core boot-path defect, and prove an unattended x64/x86 boot reaches the installer checkpoint. Do not alter firmware, media, INI inputs or production input policy. |
| S8 | Comprehensive quality audit | Inspect governance, current documentation, five Apps/Core code and test/build ownership; reconcile actual proof and report confirmed defects with bounded S9 repairs. No implementation changes in the audit. |
| S9 | Audit-driven repairs, DeskPro flattening and final qualification | Flatten DeskPro source and matching units, repair includes/names/build/gates and current docs; fix confirmed S8 findings under this revised ownership; run complete repository suites per width, verify ten current EXEs, push target-separated commits and leave a clean worktree. Shared-six and shared-rule changes require their separate admission. |
| S10 | Owner-added MyNES RDP input investigation and repair | Trace native normalization through Emulator to the NES controller, reproduce the reported lost input and repair its proven owner; preserve one input path and hotkey semantics, qualify both widths and publish affected artifacts. Required Shared changes stop for explicit review. |
| S11 | Owner-added default text-raster correction and DeskPro verification | Configure default guest BIOS for full 8x16 80x25 text with coherent raster/cursor, verify 640x400 through the native client route; retain DeskPro 14/350 if correct. Owner subsequently admits the exposed Core TIME failure: select retirement timing before asynchronous interrupt entry overwrites decoder state, regress the interrupted boundary, qualify both widths and the affected four-PC release pairs. No Shared or unrelated work changes. |

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

## Accepted S4 Record

My5170 S4 flattens its fixed AT App and matching tests: `main`,
`product_binding`, `machine_config`, `machine_plan` and their interfaces are at
the App root; unit sources are direct children of `unit/`, and the product
integration source is direct under `integration/`. The composition library is
now `my5170-machine`. Both-width full current graphs build, Core manifests and
corpus pass, and all ten My5170 unit/integration routes pass, including the
actual floppy installer boot. The x64 artifact is republished; the x86 rebuild
is deterministic and byte-identical.

## Accepted S5 Record

NXVM S5 flattens the fixed default AT App while deliberately retaining its
project-authored `firmware/` component. The App root now owns `main`,
`product_binding`, `machine_plan`, `machine_config` and their direct
interfaces; matching plan/config/apply/ROM-materialization receivers are direct
children of `test/app-nxvm/unit`, while `unit/firmware` remains the firmware
test owner. The composition library is now `nxvm-machine`. Product build
discovery recognizes this root entry shape, and the two NXVM-specific boundary
verifiers search the flat layout rather than retired component directories.

The full current default graph builds on x64 and x86. The registered NXVM and
integration selection passes 58/58 per width, including firmware, composition,
DOS, Windows and media routes; Core manifest/corpus gates and direct NXVM
layout-boundary checks also pass on both widths. No Shared-six or Core source
was modified. S6 subsequently delivered DeskPro386's original
`{chips,machine,product}` split. The S9 owner amendment now replaces that layout
with the flat App while preserving its accepted behavior.

## S9 Delivery

The [delivery report](../etc/evidence/t556-s9-quality-repair-delivery.md) records
the owner-amended flat DeskPro layout, A1--A5 repairs, MyNES receiving commits,
dual-width 592-unit and complete admitted integration results, actual diff and
ten release artifacts. Public six remain unchanged. A6 retains its standalone
Td receiver. T556 remains open for owner review.

## Completion And Stop Conditions

NXVM disposition: the owner accepted S11 delivery and requested T557. The
NXVM work is closed; S10's unresolved MyNES input repair remains in its own
active packet. Diagnostic publication does not count that repair as complete.
The original criteria below and prior evidence remain historical context.

The owner added S10 after S9 delivery and explicitly identified MyNES as the
affected product. Its full active packet is in
[MyNES Current](../../mynes/states/CURRENT.md). This bounded input-correction
amendment does not authorize PC behavior changes or Shared implementation edits,
nor does it assert owner acceptance of the earlier S9 delivery.

Every current production/test file has a destination or justified retention;
all five target trees, directional dependencies and relevant names match this
design. Existing user-visible behavior and snapshot/INI/media formats are retained.
Run affected x64/x86 tests per batch, then final full repository units and registered
integrations once per width. Rebuild/deploy the ten current App EXEs under artifact
rules, preserving owner inputs. Review actual changes before closing each S/T.

Stop for a required shared-six implementation change, guest behavior change,
incompatible persisted format, new runtime manager or unexplained loss of tests.
S1 was design/inventory only; the accepted records and S9 delivery evidence,
not admission alone, establish migration and qualification progress.
