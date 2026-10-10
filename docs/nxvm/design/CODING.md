# Source Layout

This is the macro layout authority. Apply [Coding Rules](../../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current Tree

The product/shared-corpus layout is current. My5160 owns its `product` and
`profiles` roots; My5170 owns those same roots after accepted S2.
MyDeskPro386 owns its relocated composition after accepted S3;
Current records runnable acceptance. Only default remains below `app-nxvm`.
`firmware` owns
project-authored guest firmware source and its build tools. `pc110` remains a
future Profile only when its separate evidence task admits real source files.

```text
src/
  lib/                  shared C and platform services
  emulator/{machine,session,ui,product}/
                        ISA-neutral execution, control, UI and composition
  x86/{xasm32,debug,product}/
                        portable x86 tools and shared IBM PC interaction
  core/{chips,x86,board-base,board-xt,board-at,machine,product}/
                           NXVM-only machine stack and family product support
  app-my5160/           fixed IBM 5160 App
    product/            thin main and fixed XT binding
    profiles/           IBM 5160 composition and firmware slots
  app-my5170/           fixed IBM 5170 App
    product/            thin main and fixed AT binding
    profiles/           IBM 5170 values, constraints and preparation
  app-mydeskpro386/     fixed DeskPro 386 App
    product/            thin main and fixed Model40 binding
    profiles/           Model40 composition, D4 and firmware slots
  app-nxvm/             core product implementation
    product/            thin main and fixed composition binding
    firmware/           project-owned BIOS source and offline ROM construction
    profiles/           retained default PC/AT composition and firmware slots
```

Keep machine-specific profile declarations at the profiles root. Common
floppy geometry/channel, Option ROM validation and profile-contract validation
live in flat `core/board-base`, with their independent tests under `test/core`.
The shared execution/debug adapter lives in `core/machine`; App profiles
prepare copied construction values and transfer their real context lifetime.
Shared owns candidate publication and rollback, never an App layout or model ID.
Common PC/AT descriptor/materialization and immutable ROM mapping live in
core/board-base/pc_at_profile and pc_at_rom. Machine's pc_at_preparation owns the
one allocated AT candidate and ROM preparation; each App supplies its fixed
choices and validator. No mixed default/5170 compile guards remain.
The shared x86/IBM-PC Debug and hotkey implementation lives in
`src/x86/product`; neutral Machine/Session/UI composition, fixed monitor
command grammar and help/startup framing live in `src/emulator/product` and
`test/emulator/product`. `core/product`
contains the NXVM-family INI, Machine adapter and extensions used by the four
NXVM Apps; it is not an IBM PC Product dependency. Shared process entry/banner
formatting consumes App-provided identity, request loader and factory. The
former `app-nxvm/devices` implementation is removed. Shared board mechanisms
live in the flat `core/board-*` receivers; genuine D4 state stays in
`app-mydeskpro386/profiles`. Current records delivery acceptance, not this layout.
The
former singular `core/profile` root is retired; do not restore a compatibility
directory. Preserve existing machine identities and
variants rather than renaming them into a replacement Standard profile.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in `core/chips/cpu`, not copied into board directories.

ROM mapping declarations and asset roles remain with the core product profile.
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
adjacent NXVM.ini live only in `assets/<app>/`, without a profile subdirectory; My5160 uses
`assets/my5160`; My5170 uses `assets/my5170`; MyDeskPro386 uses
`assets/mydeskpro386`. The final NXVM cutover uses `assets/nxvm` directly. Relative
runtime-media paths resolve from that file. It has no firmware/CMOS/font asset
path keys. NXVM.ini is the sole
product runtime configuration route; repository-only tests do not load it.
The owner explicitly requires the embedded-ROM EXEs in `assets/<app>/`
to be committed with their product delivery. Raw vendor ROMs and generated
byte sources/objects remain outside tracked source.
Do not rename/move external assets merely to match target source directory names.

## Files And Names

Headers stay beside implementation; only `*_interface.h` is public across
components. Keep cohesive files flat until a real subsystem needs a directory.
Stable symbol prefixes need not change because a directory moved.
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

Repository-only shared tests remain `test/{lib,common,x86,ibmpc}`. The
four-App NXVM-family Product component owns `test/core/product`; individual
App-only tests live below their matching `test/app-*/` root.
Shared board and family tests live in `test/core/board-base`,
`test/core/board-at` and `test/core/board-xt`, with their actual source owners.
Profile tests follow their real `src/app-*/profiles/` owner. The independent
My5160, My5170 and MyDeskPro386 test roots preserve their original assertions;
default remains in `test/app-nxvm`. PC-family composition tests and their
single mixed-family fixture live in `test/core/machine/composition` and
`test/core/machine/support`; board-wiring tests live in
`test/core/board-base/composition`.
The former helper-only `device` and `byob` roots are retired. Do not create
empty future-profile directories. Retained
NXVM composition/firmware tests live in `test/app-nxvm/unit/profiles/` with their
product owner. CPU, chip, execution-Core and board-timing tests live under
`test/core`, preserving all CPU models. The remaining `test/x86` package
covers only portable Debug/Xasm32. Neutral Core tests build independently of
NXVM board composition.
`test/app-nxvm/integration/` stays separate;
at cutover it uses the real INI path and external assets. Unit tests use code-
owned values without external ROM/INI/YAML/media dependencies.

Preserve every implemented board's tests and boot scenarios. Rehome CPU, chip,
transaction, lifecycle and failure regressions with their owner. Factoring
fixtures or removing duplicate machinery must not reduce behavior coverage.

## Shared PC Family

The four fixed Apps consume the sole ibmpc Machine/Product implementation;
they do not supply libraries to one another. Portable Debug/Xasm32 tooling
remains in x86; NXVM PC mechanisms live in Core, and immutable model choices
and D4 remain in each App.
PC110 remains future work, not an empty source/test directory.

Each App has its own source, profile assertions, integration registration and
flat artifact root. PC-family docs/nxvm, tools/nxvm, version and MTSP stay
unified. The external integration suite retains one family observer and INI
fixture under test/app-nxvm/integration. These are test harnesses compiled
against the selected App binding, never production dependencies or
repository-only unit inputs. Concrete XT/AT/DeskPro cases are registered in
their own test/app-*/integration trees; default scenarios stay in NXVM's.
This preserves the original predicates without copying a boot observer or
moving external-asset scenarios into test/ibmpc's unit corpus.

[T543](../history/M5-T543-four-pc-apps.md) records the ownership cutover;
[T542](../history/M5-T542-shared-pc-machine-adapter.md) records the prerequisite
Machine/composition extraction. Current alone records acceptance.
