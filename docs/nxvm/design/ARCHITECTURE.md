# System Architecture

This is the concrete architecture authority; apply the
[Architecture Rules](../../rules/ARCHITECTURE.md). The
[current baseline](../states/CURRENT.md) distinguishes implemented behavior
from this approved target.

## Product Shape

NXVM retains an extensible multi-machine architecture. XT, AT, DeskPro 386,
default PC/AT and later PC110 each link one fixed machine composition, using the same
App, Common, Lib and x86 tooling. Build configuration selects that executable's
external BYOB build-input root; the selected Profile determines its fixed firmware
roles and relative asset names. The approved ROM-packaging target embeds the
selected ROM bytes at build time for all four machines; Current distinguishes
its implementation and acceptance status. A shared NXVM.ini configures supported memory,
media paths/access and presentation, not machine identity, firmware paths, CPU
population, controller topology, startup actions or firmware boot order. It
  replaced the retired YAML loader at the completed product cutover; no
  permanent compatibility loader remains.

All implemented machines and their required personalities remain supported;
new Standard-board selection is not a migration prerequisite. PC110 requires
its own 486-class contract, not a renamed 386 profile.
This implies no VDM, mantle, DOS implementation or DLL product. Existing
release behavior remains the baseline until an implemented, verified cutover.

## Modules, Ownership, And Assembly

The following map describes the implemented baseline. The queued successor
target is specified under **Queued Shared-Hardware And App Split** below;
it does not change runtime ownership before the corresponding cutover.

- `ibmpc/product` owns shared INI syntax, copied runtime request, path construction,
  Console/Debug/hotkey policy and atomic Common Session/UI/Machine composition.
  App supplies one frozen factory with copied INFO/speed operations; it owns
  fixed hardware/firmware projection. The PC Machine adapter's S6 receiver is
  `ibmpc/machine`; Current records its acceptance status.
- `app-nxvm/product` supplies immutable identity to the sole `ibmpc/product`
  process entry, banner and cleanup body. Its fixed composition binding selects
  one Profile's frozen values/assets and preparation for Product's shared factory.
  Product consumes Machine's public creation/INFO/speed API; Machine has no
  Product dependency or duplicate runtime input type.
- `ibmpc/machine` is that driver: asset/media lifetime, bounded execution,
  pacing and copied input/output/debug adaptation. It has no machine-name
  switch, independent lifecycle queue or guest-device state.
  App prepares copied construction values and transfers its genuine Profile
  context. The adapter owns publication/rollback and releases the context only
  after Core routes, providers and media cease borrowing it.
- `app-nxvm/profiles` owns each board's actual composition: device construction,
  wiring, clocks, memory constraints, firmware slots, fixed relative asset names
  and board-specific behavior.
  It constructs and destroys the selected machine through neutral device
  contracts; it does not depend on Common or Machine-adapter internals.
- `x86/chips` owns the extracted CPU, FPU, PIC, PIT, DMA, RTC, KBC, PPI,
  keyboard, mouse, FDC, HDC and video chip mechanisms. T539 closed their
  independent-chip source ownership. The neutral executor,
  guest timeline, memory/port routes and plan transaction live in `x86/core`.
  Its production target depends only on Types, CPU and FPU; NXVM links that
  sole implementation. Accepted S89 puts the complete media registry
  and display-provider slot in `ibmpc/board-common`, consumed through public
  contracts. The common PIT adapter installs four copied routes against a
  board-owned opaque chip; composition alone selects the Core implementation.
  The common PIC aggregation owns opaque controller endpoints and IRQ source
  leases, port/cascade routing, reset, deadline and acknowledgement. App
  producers borrow sources for that aggregation's lifetime; copied register
  diagnostics do not expose a chip or mutable layout. Current records its
  verification and acceptance status.
  `ibmpc/board-common` also owns the complete board construction, reset,
  clock/deadline reduction and teardown, plus opaque FDC/HDC/video adapters.
  `ibmpc/board-at` owns KBC and planar parity; `ibmpc/board-xt` owns PPI keyboard
  wiring. Common composition integrates their public contracts; neither
  family reads the common board layout. Model40 alone owns D4 memory,
  Port B and refresh state through one frozen board-profile binding.
  The former `app-nxvm/devices` implementation is removed. Chip state stays
  in `x86/chips`, guest time in Core, and profile/firmware/media choices in
  App composition. Current records T540 acceptance and the next App cutover.
- `common/machine` owns the shared execution/control protocol and paused-debug
  lease; `common/session` is the sole product-control reducer;
  `common/ui` binds Lib KVM and the Console broker.
- `x86/debug` owns Debug CLI continuations; `x86/xasm32` owns assembly and
  disassembly. Paused Debug operations go through Common Machine and the NXVM
  driver to Core, not a second machine path.
- `lib` owns platform/C-runtime services. Lib/Common/x86 retain their existing neutral boundaries. The separate
  ibmpc package contains only shared PC integration, never App-private definitions.
- `app-nxvm/firmware` owns project-authored guest BIOS source and offline ROM
  construction. Its build tool may consume x86 assembly and Lib file services,
  but the construction tool is not linked into the machine executable. Its
  generated guest ROM and BYOB vendor ROMs use one build-time embedding and
  immutable Core mapping route; no host BDA/IVT/reset
  service or software-interrupt interception is restored.

### Core And Board Lifetime

Board APIs consume an opaque board handle; execution/debug APIs consume the
opaque Core handle. Construction publishes both only after the frozen plan
succeeds. Core's one attachment binding owns board teardown; the driver merely
borrows the board and clears that handle on destruction. No private association,
lookup getter, side registry or second lifetime owner remains.

The board owns peripheral/family allocations and IRQ leases. Its optional
profile binding is frozen during construction and finalized before borrowed
PITs. Failed candidates follow the same teardown path. The
[S93 receiver](../etc/architecture/t540-s93-whole-board-receiver.md) defines this
boundary; its [work evidence](../etc/evidence/t540-s93-whole-board-receiver-work.md)
and Current distinguish implementation from accepted delivery.

### Fixed Composition Without A New Framework

Build selection supplies one profile composition entry to the adapter:

```text
build-selected profile + compiled immutable firmware + App INI options
                              |
                              v
       Profile resolves its fixed assets, then constructs one frozen Core plan
                              |
                              v
           one Core + Board instance -> Machine adapter -> Common Machine
```

Profile owns hardware constraints and firmware asset resolution; shared Product
owns INI syntax and runtime-media path resolution; Core owns
generic structural/state invariants. These are distinct checks, not copies of
one board rule in all three layers. Share constants or construction helpers
only where semantics match. Fixed targets need no profile inheritance
engine, plugin registry, per-field provenance mirror or runtime machine registry.
Provenance belongs in evidence rather than repeated runtime strings.

Construction prepares assets and a plan, validates, publishes one live machine,
and rolls back through one resource owner on failure. Reset reuses the frozen
plan and Core reset path. The driver retains only the selected board's resources,
not simultaneous XT, PC/AT and Compaq ROM/state records. Profile owns board
construction; Machine retains execution, host-resource lifetime and Common
adaptation only.
Injected asset/media services use neutral contracts, avoiding a Profile-to-
Machine dependency cycle. Adding a board needs a composition and build entry,
not another Common queue, App parser or generic-device machine-name branch.

The build root is a local CMake input, not a tracked absolute path or runtime
configuration selector. A build validates each selected BYOB ROM's slot, size
and hash, generates ignored byte objects and links them into that profile's
EXE. Project-owned BIOS source is assembled before this same embedding step.
Missing or mismatched build inputs fail the build; there is no runtime file
fallback or cross-profile ROM selection. Embedding is packaging, not host-side
BIOS emulation: guest instructions still execute through the CPU and devices.
Vendor ROM originals remain external and generated byte sources remain ignored.
The owner explicitly authorizes committing the resulting embedded-ROM EXEs in
the normal product artifact directory; see the source policy.

### CPU And Machine Preservation

CPU identity, feature/timing tables and instruction dispatch remain chip-owned
at `x86/chips/cpu` and selectable by Core callers and repository-only CPU tests. Preserve all
existing models and tests even when no shipped machine uses them. New 188/486
coverage needs sources and implementation; an enum alias cannot turn 386 into
486. Fixed products choose their documented CPU once. Do not scatter build
macros through handlers or remove later CPUs' 16-bit, real-mode or VM86 semantics.

Retain every implemented machine's device personalities, including default
and Compaq-specific behavior. Structural cleanup may remove duplicate or dead
mechanisms with caller proof, but not live machine capabilities. One VADP owner
retains needed VGA/EGA mechanisms;
PC110 extensions must not create second VRAM/frame truth. One HDC owner retains
only selected storage personalities, not an assumption that all disks are ATA.

Board-local mutable registers have one owner, lifetime and event registration
within the same Core instance. Their implementation may live with the profile,
but it uses bounded Core contracts, not private CPU/RAM pointers or a board
scheduler. Immutable configuration is distinct from guest-programmed state.

## Product And Host Boundary

Common retains one Session control queue and one Machine execution boundary;
App adds neither a parallel queue nor a second reducer. The NXVM driver runs
the existing bounded Core path; no per-profile executor, presenter, debugger
or file backend is introduced.

Core alone advances guest time. Board clocks/wiring feed its existing plan.
The driver may limit already-produced progress against host monotonic time;
host elapsed time never generates or skips guest ticks. Exact source values
or formulas are L3, model/proportional estimates L2, order-only behavior L1.
Fixed-machine packaging is not a timing upgrade.

Lib Storage remains the sole file/lock/direct/readonly/overlay implementation.
The machine adapter retains genuine media semantics such as geometry and
change generation; do not force FDD/HDD into identical behavior merely to
reduce files. In the approved packaging target, ROM bytes are immutable linked
inputs selected at build time; seed configuration
initializes Core-owned writable CMOS rather than a second BIOS/register mirror.

Guest writes flow through sole device state into copied snapshots, Common UI
and Lib KVM. Native handles, fonts and presentation are not guest-video owners.
All executables retain the same lifecycle and UX. The shared INI is one format
and parser, not a promise that every memory size or disk fits every board.
Omitted values use selected-profile defaults; explicit unsupported values fail
clearly rather than selecting another board or silently changing hardware.

Each selected product deploys once to the versioned
`assets/nxvm/<profile>/` directory, alongside its owner-maintained `NXVM.ini`.
That is the only current executable location; `build/` remains compiler state
apart from historical evidence. The tracked executable/INI pair is adjacent
and used by deployed-product integration. EXE deployment never rewrites or
relocates the owner INI: its relative media paths belong to that directory.

## Runtime Admission Boundary

The [source policy](../etc/operations/policy/source-policy.md) owns asset and
redistribution handling. Profile-local ROM code means slot/mapping declarations;
project-owned BIOS source belongs to firmware. Raw vendor ROM files are not
committed; the owner-approved embedded EXEs are product artifacts. Boot order and POST remain guest
firmware behavior; no synthetic F1, host service or silent fallback substitutes
for missing hardware. The self-built default BIOS is an explicit firmware
choice, never a fallback for a missing IBM or Compaq ROM.

[Roadmap](ROADMAP.md) and [Current](../states/CURRENT.md) own sequencing and
implemented status. The [T533 consolidation record](../history/M5-T533-fixed-machine-products.md)
maps this design to observed code and bounded migration evidence.

## Queued Shared-Hardware And App Split

The owner-approved planning direction has five ordered stages. The first is
[closed T539](../history/M5-T539-independent-shared-chips.md), followed by
the closed T540 board receiver, [delivered Product extraction](../history/m5-shared-pc-product.md)
and [queued App split](../proposals/m5-independent-pc-apps.md):

1. `x86/chips` is the target owner of independent chips, including CPU, PIC, PIT and DMA;
   each retains its state and internal timing. It does not own a PC profile,
   host executor, firmware workaround or peer chip's internals. Composition
   connects public memory/I/O cycles, signals and interrupt acknowledgement.
2. `x86/core` owns the neutral x86 machine executor and plan transaction;
   `ibmpc/board-common`, `ibmpc/board-at` and `ibmpc/board-xt` own the four machines'
   proven common/family-specific PC board routing and
   wiring. Both use independent chip contracts; product-specific topology and
   asset selection stay in the App. No mirrored device state, product policy or
   second Common lifecycle loop is introduced.
3. `ibmpc/product` receives the four PC products' identical Console command/API,
   INI/startup/composition and UX implementation. Common/Lib keep their
   existing reducer, queue and native presentation owners. The complete
   Product-to-Machine boundary uses the existing Common driver plus a frozen
   typed App factory and copied INFO/speed operations, never an App-private
   include. The shared board/media/execution adapter now lives in
   `ibmpc/machine`; App retains frozen identity,
   hardware construction and firmware definitions, with no per-machine console copy or new
   generic App framework is introduced.
   T541 keeps the four current builds and deployment paths intact. Lib/Common
   and existing x86 components are not implementation-change targets.
4. Reopened [T542](../history/M5-T542-shared-pc-machine-adapter.md) delivers
   the remaining shared PC Machine adapter into ibmpc/machine and proven
   construction helpers into ibmpc/board-common. Corrective S11 separates
   this PC package from x86 without changing algorithms. S2-S6 delivered those receivers;
   S7 replaced the App's all-profile plan unions and runtime construction
   dispatch with build-selected constructors. App retains fixed profile/firmware rules and genuine
   D4; no Lib/Common change, extra queue or profile registry is introduced.
   The [S1 receiver ledger](../history/M5-T542-shared-pc-machine-adapter.md)
   owns the task-specific migration contract and evidence.
   Its corrective composition batch finishes the remaining factory, asset and
   shared AT assembly mechanisms before App splitting. The AT mechanism has
   three consumers (5170, default and DeskPro), not three identical machine
   definitions; each model retains its own hardware values and genuine behavior.
5. The separate queued task makes four independent Apps compose these capabilities: `app-mypcxt`
   (5160), `app-mypcat` (5170), `app-mypcdeskpro386` (Model 40), and
   `app-nxvm` (default 386). Later PC110 belongs to `app-mypc110` after its
   separate hardware qualification. No App depends on another App.

The x86 package owns neutral chips/Core/tools; ibmpc owns reusable PC
integration. Lib/Common stay neutral and no x86 component depends on ibmpc. Chip and shared-board tests follow
their shared owner; product firmware/boot/INI tests follow their App. Preserve
existing CPU families, personalities and tests. Structural moves are not V30,
Raiden II, 486 or PC110 implementation and do not upgrade timing evidence.
New product scope names and deployment rules require separate Td governance
before the final App cutover; current names/locations remain valid.
