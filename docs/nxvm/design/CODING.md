# Source Layout

This is the macro layout authority. Apply [Coding Rules](../../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current Tree

The product/shared-corpus layout is current. `machine`, `product`
and `profiles` are NXVM runtime roots below `app-nxvm`; `firmware` owns
project-authored guest firmware source and its build tools. `pc110` remains a
future Profile only when its separate evidence task admits real source files.

```text
src/
  lib/                  shared C and platform services
  common/{session,machine,ui}/
  x86/{chips,core,ibmpc-common,ibmpc-at,ibmpc-xt,xasm32,debug,product}/
  app-nxvm/             NXVM product implementation
    product/            main and fixed config/factory projection
    machine/            NXVM driver, asset/media and execution adapter
    firmware/           project-owned BIOS source and offline ROM construction
    profiles/
      xt/               IBM 5160 board composition and firmware slots
      at/               IBM 5170 board composition and firmware slots
      model40/          retained DeskPro 386 composition and firmware slots
      default_profile/  retained default PC/AT composition and firmware slots
```

Keep shared profile declarations and proven helpers at the profiles root. The
shared Product INI, command/hotkey and Common composition files and their
reusable assertions live in x86/product and test/x86/product. Shared process
entry/banner formatting consumes App-owned immutable identity and factory values. The
former `app-nxvm/devices` implementation is removed. Shared board mechanisms
live in the flat `x86/ibmpc-*` receivers; genuine D4 state stays in
`app-nxvm/profiles/model40`. Current records delivery acceptance, not this layout.
The
former singular `core/profile` root is retired; do not restore a compatibility
directory. Preserve existing machine identities and
variants rather than renaming them into a replacement Standard profile.
Do not add a framework or empty future directories. CPU-family implementations
and selection tables stay in `x86/chips/cpu`, not copied into board directories.

ROM mapping declarations and asset roles remain with the NXVM product profile.
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
adjacent NXVM.ini live only in `assets/nxvm/<profile>/`; relative
runtime-media paths resolve from that file. It has no firmware/CMOS/font asset
path keys. NXVM.ini is the sole
product runtime configuration route; repository-only tests do not load it.
The owner explicitly requires the embedded-ROM EXEs in `assets/nxvm/<profile>/`
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

## Source Organization

Repository-only shared tests remain `test/{lib,common,x86}`. NXVM-only tests
live below `test/app-nxvm/`, mirroring `app-nxvm` beneath `unit/`.
Shared board and family tests live in `test/x86/ibmpc-common`,
`test/x86/ibmpc-at` and `test/x86/ibmpc-xt`, with their actual source owners.
Profile tests mirror their real `src/app-nxvm/profiles/` owner when
implemented. The current product roots are `xt`, `default_profile`, `model40`,
`device`, and `byob`; do not create empty future-profile directories. Retained
NXVM composition/firmware tests live in `test/app-nxvm/unit/core/` with their
product owner. CPU mechanism and retained board-timing recipe tests live under
`test/x86`, preserving all CPU models. Neutral Core tests live
in `test/x86/core` and build independently of NXVM board composition.
`test/app-nxvm/integration/` stays separate;
at cutover it uses the real INI path and external assets. Unit tests use code-
owned values without external ROM/INI/YAML/media dependencies.

Preserve every implemented board's tests and boot scenarios. Rehome CPU, chip,
transaction, lifecycle and failure regressions with their owner. Factoring
fixtures or removing duplicate machinery must not reduce behavior coverage.

## Queued Successor Layout

The queued [App split](../states/QUEUE.md) targets:

```text
src/
  lib/
  common/
  x86/{chips,core,ibmpc-common,ibmpc-at,ibmpc-xt,xasm32,debug,product}/
  app-mypcxt/
  app-mypcat/
  app-mypcdeskpro386/
  app-nxvm/             default 386 only after cutover
```

`app-mypc110` is future work, not an empty directory to create now. The neutral
x86 machine executor lives in `x86/core`; real chip responsibilities live in
`x86/chips`; common PC board mechanisms live in the flat `x86/ibmpc-*`
components; product-specific assembly
remains App-owned. Existing CPU implementation style and coherent file
boundaries are preserved, not rewritten for renaming.
Matching shared tests live in
`test/x86/{chips,core,ibmpc-common,ibmpc-at,ibmpc-xt}`; each App owns its
`test/app-<product>` unit/integration tree, documentation, tools and build entry.
No source, test, firmware, INI or executable is relocated by this proposal-only
governance. Deployment identities/paths are separately governed at App cutover.

T541 first extracts Product while the existing App/profile directories stay
in place. The separate queued split creates the four App roots afterward.
Neither stage modifies Lib or Common under the current owner restriction.

The owner-required x86/product receiver owns the identical PC Console/API,
INI/startup/UX implementation once, with matching test/x86/product coverage.
Its final flat file set and minimum typed Machine binding are determined by
T541 S1's actual-source inventory; existing board/media/execution adapter files
stay App-owned. Each App retains real
product identity, fixed board/firmware composition and build binding; no App
is the source library of another App or the shared Product implementation.
