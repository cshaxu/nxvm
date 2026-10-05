# Source Layout

This is the macro layout authority. Apply [Coding Rules](../../rules/CODING.md);
dependencies belong to [System Architecture](ARCHITECTURE.md).

## Current Tree

The product/shared-corpus layout is current. My5160 owns its `product` and
`profiles` roots; My5170 owns those same roots after accepted S2.
MyDeskPro386 owns its relocated composition during S3 verification;
Current records runnable acceptance. Only default remains below `app-nxvm`.
`firmware` owns
project-authored guest firmware source and its build tools. `pc110` remains a
future Profile only when its separate evidence task admits real source files.

```text
src/
  lib/                  shared C and platform services
  common/{session,machine,ui}/
  x86/{chips,core,xasm32,debug}/
  ibmpc/{board-common,board-xt,board-at,machine,product}/
  app-my5160/           fixed IBM 5160 App
    product/            thin main and fixed XT binding
    profiles/           IBM 5160 composition and firmware slots
  app-my5170/           fixed IBM 5170 App
    product/            thin main and fixed AT binding
    profiles/           IBM 5170 values, constraints and preparation
  app-mydeskpro386/     fixed DeskPro 386 App
    product/            thin main and fixed Model40 binding
    profiles/           Model40 composition, D4 and firmware slots
  app-nxvm/             NXVM product implementation
    product/            thin main and fixed composition binding
    firmware/           project-owned BIOS source and offline ROM construction
    profiles/
      default_profile/  retained default PC/AT composition and firmware slots
```

Keep machine-specific profile declarations at the profiles root. Common
floppy geometry/channel, Option ROM validation and profile-contract validation
live in flat `ibmpc/board-common`, with their independent tests under `test/ibmpc`.
The shared execution/debug adapter lives in `ibmpc/machine`; App profiles
prepare copied construction values and transfer their real context lifetime.
Shared owns candidate publication and rollback, never an App layout or model ID.
Common PC/AT descriptor/materialization and immutable ROM mapping live in
board-common/pc_at_profile and pc_at_rom. Machine's pc_at_preparation owns the
one allocated AT candidate and ROM preparation; each App supplies its fixed
choices and validator. No mixed default/5170 compile guards remain.
The shared Product INI, command/hotkey and Common composition files and their
reusable assertions live in ibmpc/product and test/ibmpc/product. Shared process
entry/banner formatting consumes shared PC identity/version and App-owned factory values. The
former `app-nxvm/devices` implementation is removed. Shared board mechanisms
live in the flat `ibmpc/board-*` receivers; genuine D4 state stays in
`app-mydeskpro386/profiles`. Current records delivery acceptance, not this layout.
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

Repository-only shared tests remain `test/{lib,common,x86,ibmpc}`. NXVM-only tests
live below `test/app-nxvm/`, mirroring `app-nxvm` beneath `unit/`.
Shared board and family tests live in `test/ibmpc/board-common`,
`test/ibmpc/board-at` and `test/ibmpc/board-xt`, with their actual source owners.
Profile tests follow their real `src/app-*/profiles/` owner. The independent
My5160, My5170 and MyDeskPro386 test roots preserve their original assertions;
default remains in `test/app-nxvm`. Final legacy unit-directory alignment and
the single mixed-family test fixture are S4 receivers;
the former helper-only `device` and `byob` roots are retired. Do not create
empty future-profile directories. Retained
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

The admitted [T543 App split](../proposals/m5-independent-pc-apps.md) targets:

```text
src/
  lib/
  common/
  x86/{chips,core,xasm32,debug}/
  ibmpc/{board-common,board-xt,board-at,machine,product}/
  app-my5160/
  app-my5170/
  app-mydeskpro386/
  app-nxvm/             original default 386 hardware, named NXVM after cutover
```

`app-mypc110` is future work, not an empty directory to create now. The neutral
x86 machine executor lives in `x86/core`; real chip responsibilities live in
`x86/chips`; common PC board mechanisms live in the flat `ibmpc/board-*`
components; product-specific assembly
remains App-owned. Existing CPU implementation style and coherent file
boundaries are preserved, not rewritten for renaming.
Matching shared tests live in
`test/x86/{chips,core}` and `test/ibmpc/{board-common,board-at,board-xt}`; each App owns its
`test/app-<product>` unit/integration tree, assets/<product> and build entry.
The four PC Apps continue to share docs/nxvm, tools/nxvm, version declarations
and one MTSP sequence; do not copy these into four independently maintained
hierarchies.
No source, test, firmware, INI or executable is relocated by this proposal-only
governance. Deployment identities/paths are separately governed at App cutover.

T541 first extracts Product while the existing App/profile directories stay
in place. The separate queued split creates the four App roots afterward.
Neither stage modifies Lib or Common under the current owner restriction.

Before the App split, accepted T542 delivers the remaining shared adapter in
`ibmpc/machine` (with its real media subresponsibility) and common
construction helpers in flat `ibmpc/board-common`. Matching independent tests
follow those owners. Current and the T542 ledger distinguish working source
from accepted delivery.
S3 relocates media implementation/tests to ibmpc/machine/media and
test/ibmpc/machine/media; App keeps only opaque media handles and its
composition assertions, not the shared media layouts.
S4 places pure mapper/frame conversion in flat ibmpc/machine. The
shared display orchestration directly converts the video snapshot;
the old guest-frame view lives only in test support, not production source.
The prepared Profile context retains Model40 D4 and terminal observations;
generic Machine holds neither. App probes consume copied Profile values, with
their stateless views under test/app-nxvm/support.
No App can retain another App's shared Machine implementation as its library.

The owner-required ibmpc/product receiver owns the identical PC Console/API,
INI/startup/UX implementation once, with matching test/ibmpc/product coverage.
Its final flat file set and minimum typed Machine binding are determined by
T541 S1's actual-source inventory and T542's construction ledger. Each App retains real
machine identity, fixed board/firmware composition and build binding; PC version
declarations are shared in ibmpc/product/version_interface.h. No App
is the source library of another App or the shared Product implementation.
