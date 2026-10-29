# Source Layout

This is the macro layout authority. Apply [Coding Rules](../../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current Tree

The product/shared-corpus layout is current. My5160, My5170 and default NXVM
are flat Apps; only default's `firmware` owns project-authored guest firmware
source and its build tools. MyDeskPro386 is also flat, with cohesive D4,
construction, firmware and product files. MyPC110 owns its 486SX construction and stopped-only DSTN/TFT firmware
selection. Its two host-architecture products share one Machine lifetime;
Current records the retained qualification limits.

```text
src/
  lib/                  shared C and platform services
  emulator/{machine,session,ui,product}/
                        ISA-neutral execution, control, UI and composition
  x86-debug/{xasm32,product}/
                        portable assembler/disassembler and Debug CLI
  x86-ibmpc/product/     shared PC interaction and process entry
  x86-nxvm/{chips,board-base,board-xt,board-at,machine,product}/
                        NXVM peripherals, PC boards, adapter and family product
  x86-core/{cpu,fpu,machine}/
                        shared CPU/FPU and neutral execution mechanisms
  app-my5160/           fixed IBM 5160 App: main, product binding,
                         machine plan/configuration/construction and firmware map
  app-my5170/           fixed IBM 5170 App: main, product binding,
                         machine plan/configuration/construction and firmware map
  app-mydeskpro386/     fixed DeskPro 386 App: D4 hardware/wiring, machine
                         plan/configuration/construction, firmware map and entry
  app-nxvm/             fixed default AT App: main, product binding,
                         machine plan/configuration/construction and firmware map
    firmware/           project-owned BIOS source and offline ROM construction
  app-mypc110/          DSTN and TFT PC110 qualification App: board construction,
                         firmware bindings and stopped-state display selection
```

Keep machine-specific declarations beside the fixed App's construction source.
Common floppy geometry/channel, Option ROM validation and profile-contract
validation live in flat `x86-nxvm/board-base`, with their independent tests under
`test/x86-nxvm`.
The shared execution/debug adapter lives in `x86-nxvm/machine`; App profiles
prepare copied construction values and transfer their real context lifetime.
Shared owns candidate publication and rollback, never an App layout or model ID.
Common PC/AT descriptor/materialization and immutable ROM mapping live in
x86-nxvm/board-base/pc_at_profile and pc_at_rom. Machine's pc_at_preparation owns the
one allocated AT candidate and ROM preparation; each App supplies its fixed
choices and validator. No mixed default/5170 compile guards remain.
Shared PC Debug routing and hotkeys live in `src/x86-ibmpc/product`; reusable
Debug CLI and assembly tools live in `src/x86-debug`. Neutral Machine/Session/UI composition, fixed monitor
command grammar and help/startup framing live in `src/emulator/product` and
`test/emulator/product`. `x86-nxvm/product`
contains the NXVM-family INI, Machine adapter and extensions used by the four
NXVM Apps; it is not an IBM PC Product dependency. Shared process entry consumes
App-provided identity/banner and already composed machine/UI values; X86 NXVM Product owns
the INI loader and factory. The
former `app-nxvm/devices` implementation is removed. Shared board mechanisms
live in the flat `x86-nxvm/board-*` receivers. MyDeskPro386 keeps D4 memory decoding
in `d4_memory.*`, board signals in `d4_platform.*`, and fixed construction in
the adjacent `machine_*` files. Current records delivery
acceptance, not this layout.
The
former singular `core/profile` root is retired; do not restore a compatibility
directory. Preserve existing machine identities and
variants rather than renaming them into a replacement Standard profile.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in `x86-core/cpu`, not copied into board directories.

PC-wide timing declarations and device trace identities are public Board-Base
contracts in `x86-nxvm/board-base/{timing,trace}_interface.h`, not neutral execution
types. Their implementation remains with the cohesive plan/deadline owners.
Matching qualification/trace tests live under `test/x86-nxvm/board-base`.
Neutral executor fixtures supply explicit CPU and
RAM inputs; ordinary test allocation sizes do not import PC resize policy.
The neutral executor and its matching fixtures/tests live in
`src/x86-core/machine` and `test/x86-core/machine`; package-level metadata stays
at the package root. Board qualification programs that join NXVM manual
ledgers remain under the outer Board qualification owner, with registration
in `cmake/nxvm/NxvmTimingTests.cmake`, not a dependency of the standalone
component tests. Repository cross-App checks are registered separately by
`cmake/nxvm/NxvmRepositoryTests.cmake`. Device advancement stays cohesive in Board-Base's `board_advance.c`; the neutral
attachment declares only before/after-internal advancement and CPU interrupt
signals, not PIT pairs or media/RTC/peripheral slots. Matching Board-order and
neutral internal-readiness assertions stay with those respective test owners.
PC A20 policy and Boolean pin adaptation stay in Board-Base. Execution config
contains only a copied address window/reset-clear-mask; neutral memory's raw
overlay and cache/inspection paths do not import a Board policy enum.

ROM mapping declarations and asset roles remain with each fixed App profile.
Project-owned BIOS source/build lives in `src/app-nxvm/firmware`, not in the
runtime driver or devices. Its build produces a candidate ROM under `build/`
and embeds its bytes into the selected EXE. Other machines embed their BYOB
ROMs through the same build step. Protected originals remain in the user-supplied,
owner-managed `nxvm-assets/profiles-nxvm/<machine>/` tree; original manuals remain
in `nxvm-assets/manuals/`. CMake receives the untracked absolute
`NXVM_PROFILE_ASSETS_ROOT` for a selected build. Generated ROM byte sources and
objects remain ignored under `build/`, not tracked source. The runtime ROM
contract carries immutable bytes, not external file paths. This is the approved
target; Current records cutover verification and acceptance status.
Documentation changes do not move assets. Each versioned local product EXE and its
adjacent `<app>.ini` live only in `assets/<app>/`, without a profile subdirectory; My5160 uses
`assets/my5160`; My5170 uses `assets/my5170`; MyDeskPro386 uses
`assets/mydeskpro386`. The final NXVM cutover uses `assets/nxvm` directly. Relative
runtime-media paths resolve from that file. It has no firmware/CMOS/font asset
path keys. The App-selected INI is the sole
product runtime configuration route; repository-only tests do not load it.
The owner explicitly requires the embedded-ROM EXEs in `assets/<app>/`
to be committed with their product delivery. Raw vendor ROMs and generated
byte sources/objects remain outside tracked source.
Do not rename/move external assets merely to match target source directory names.

## Files And Names

Headers stay beside implementation; only `*_interface.h` is public across
components. Keep cohesive files flat until a real subsystem needs a directory.
Chip hardware prefixes remain stable. Package-owned board, Machine and Product
interfaces use their canonical X86 NXVM owner prefixes; retired Core/VM aliases
are not a compatibility surface.
`src/lib/types` owns shared C vocabulary; legacy root aliases need a separate
caller migration, not another facade.

One build-selected profile entry supplies the existing factory. Prefer direct
construction and small immutable descriptions over string dispatch, recursive
inheritance or duplicated build source lists. Define the concrete interface
from actual construction requirements during implementation.
The shared Product factory consumes Machine's neutral input directly. App's
selected binding supplies fixed values, compiled assets and actual Profile
preparation; it has no second runtime-media projection or factory body.

## Source Organization

Repository-only shared tests follow `test/{lib,emulator,x86-debug,x86-ibmpc,x86-nxvm,x86-core}`. The
four-App NXVM-family Product component owns `test/x86-nxvm/product`; individual
App-only tests live below their matching `test/app-*/` root.
Shared board and family tests live in `test/x86-nxvm/board-base`,
`test/x86-nxvm/board-at` and `test/x86-nxvm/board-xt`, with their actual source owners.
Profile tests follow their real source owner. The independent
My5160, My5170 and MyDeskPro386 test roots preserve their original assertions;
default remains in `test/app-nxvm`. PC-family composition tests and their
single mixed-family fixture live in `test/x86-nxvm/machine/composition` and
`test/x86-nxvm/machine/support`; board-wiring tests live in
`test/x86-nxvm/board-base/composition`.
The former helper-only `device` and `byob` roots are retired. Do not create
empty future-profile directories. Retained
NXVM composition/configuration tests live directly in `test/app-nxvm/unit/`;
firmware tests remain in `test/app-nxvm/unit/firmware/`. All other NXVM unit tests are direct children of `test/app-nxvm/unit/`. DeskPro unit tests
are flat under `test/app-mydeskpro386/unit`. CPU, chip, execution-Core and board-timing tests
live under their owners: peripheral chip tests in `test/x86-nxvm/chips`, CPU/FPU
and neutral execution tests in `test/x86-core`, and remaining board tests in
`test/x86-nxvm`, preserving all CPU models. `test/x86-debug` covers
portable Debug/Xasm32; `test/x86-ibmpc` owns PC interaction tests and
`test/x86-nxvm` owns peripheral, board, Machine adapter and family Product
contracts, including information/speed. Neutral X86 Core tests build independently of
NXVM board composition.
`test/app-nxvm/integration/` stays separate;
at cutover it uses the real INI path and external assets. Unit tests use code-
owned values without external ROM/INI/YAML/media dependencies.

Preserve every implemented board's tests and boot scenarios. Rehome CPU, chip,
transaction, lifecycle and failure regressions with their owner. Factoring
fixtures or removing duplicate machinery must not reduce behavior coverage.

## Shared PC Family

The four fixed Apps consume the sole X86 NXVM Machine/Product implementation;
they do not supply libraries to one another. Portable Debug/Xasm32 tooling
remains in x86-debug; NXVM PC mechanisms live in x86-nxvm, and immutable model choices
and D4 remain in each App.
`app-mypc110` owns fixed 486SX construction, the two external firmware
bindings, Product entry/binding and PC110 decode choices. Its selectable TFT and
derived-DSTN variants use the normal Product/Machine route and deploy one
executable per host architecture under `assets/mypc110`; Current records remaining
guest, display and release qualification.

Each App has its own source, profile assertions, integration registration and
flat artifact root. PC-family docs/nxvm, tools/nxvm, version and MTSP stay
unified. The external integration suite retains one family observer and INI
fixture under test/x86-nxvm/setup. These are test harnesses compiled
against the selected App binding, never production dependencies or
repository-only unit inputs. Concrete XT/AT/DeskPro cases are registered in
their own test/app-*/integration trees; default scenarios stay in NXVM's.
This preserves the original predicates without copying a boot observer or
moving external-asset scenarios into a shared unit corpus.

[T543](../history/M5-T543-four-pc-apps.md) records the ownership cutover;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) records the prerequisite
Machine/composition extraction. Current alone records acceptance.
