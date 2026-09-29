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

- `app-nxvm/product` owns INI syntax, runtime-media paths, product CLI and the one composition root.
  It assembles Common Session/UI/Machine and the NXVM driver.
- `app-nxvm/machine` is that driver: asset/media lifetime, bounded execution,
  pacing and copied input/output/debug adaptation. It has no machine-name
  switch, independent lifecycle queue or guest-device state.
- `app-nxvm/profiles` owns each board's actual composition: device construction,
  wiring, clocks, memory constraints, firmware slots, fixed relative asset names
  and board-specific behavior.
  It constructs and destroys the selected machine through neutral device
  contracts; it does not depend on Common or Machine-adapter internals.
- `app-nxvm/devices` owns CPU, memory, bus, devices, reset,
  generic execution and faults and the sole guest
  timeline. Generic mechanisms know hardware contracts, not product names.
  T539 moves PIT 8253/8254 state and waveforms to `x86/devices/pit825x`,
  RTC/calendar/register state to `x86/devices/rtc146818`, individual PIC
  state/priority to `x86/devices/pic8259`, DMA to `dma8237`, AT controller and
  endpoints to `kbc8042`/`keyboard`/`ps2mouse`, and qualified XT register/serial
  mechanisms to `ppi8255`/`xtkeyboard`, and the FDC command/PCN/cause mechanism
  to `fdc8272`. NXVM retains the FDC PC registers, physical drive and record
  provider, cascade/source aggregation,
  port attachment, index/NMI latches, seed/checksum, clock conversion and
  IRQ/refresh/speaker wiring. Current records each batch's acceptance; other
  chip extractions remain pending.
- `common/machine` owns the shared execution/control protocol and paused-debug
  lease; `common/session` is the sole product-control reducer;
  `common/ui` binds Lib KVM and the Console broker.
- `x86/debug` owns Debug CLI continuations; `x86/xasm32` owns assembly and
  disassembly. Paused Debug operations go through Common Machine and the NXVM
  driver to Core, not a second machine path.
- `lib` owns platform/C-runtime services. Lib/Common/x86 remain product-neutral
  and source-shareable with SoftPC.
- `app-nxvm/firmware` owns project-authored guest BIOS source and offline ROM
  construction. Its build tool may consume x86 assembly and Lib file services,
  but the construction tool is not linked into the machine executable. Its
  generated guest ROM and BYOB vendor ROMs use one build-time embedding and
  immutable Core mapping route; no host BDA/IVT/reset
  service or software-interrupt interception is restored.

### Fixed Composition Without A New Framework

Build selection supplies one profile composition entry to the adapter:

```text
build-selected profile + compiled immutable firmware + App INI options
                              |
                              v
       Profile resolves its fixed assets, then constructs one frozen Core plan
                              |
                              v
           one Devices instance -> Machine adapter -> Common Machine
```

Profile owns hardware constraints and firmware asset resolution; App owns INI
syntax and runtime-media path resolution; Core owns
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

CPU identity, feature/timing tables and instruction dispatch remain Core-owned
and selectable by Core callers and repository-only CPU tests. Preserve all
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
`assets/nxvm/<profile>/` directory, alongside its generated `NXVM.ini`.
That is the only current executable location; `build/` remains compiler state
apart from historical evidence. The tracked executable/INI pair is adjacent
and updated only for the selected profile.

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

The owner-approved planning direction has three ordered stages. The first is
[T539](../proposals/m5-shared-chip-extraction.md), executing staged chip extraction;
the other two remain [implementation candidates](../states/QUEUE.md):

1. `x86/devices` owns independent chips, including CPU, PIC, PIT and DMA;
   each retains its state and internal timing. It does not own a PC profile,
   host executor, firmware workaround or peer chip's internals. Composition
   connects public memory/I/O cycles, signals and interrupt acknowledgement.
2. `x86/ibmpc` owns the four machines' proven common PC assembly mechanisms:
   board routing, wiring, construction/reset and one guest scheduler. It uses
   independent chip contracts; product-specific topology and asset selection
   stay in the App. No mirrored device state or second Common lifecycle loop.
3. Four independent Apps compose these shared capabilities: `app-mypcxt`
   (5160), `app-mypcat` (5170), `app-mypcdeskpro386` (Model 40), and
   `app-nxvm` (default 386). Later PC110 belongs to `app-mypc110` after its
   separate hardware qualification. No App depends on another App.

The x86 package therefore expands beyond tools to hardware and reusable PC
integration, while Lib/Common stay neutral. Chip and shared-board tests follow
their shared owner; product firmware/boot/INI tests follow their App. Preserve
existing CPU families, personalities and tests. Structural moves are not V30,
Raiden II, 486 or PC110 implementation and do not upgrade timing evidence.
New product scope names and deployment rules require separate Td governance
before the third candidate's cutover; current names/locations remain valid.
