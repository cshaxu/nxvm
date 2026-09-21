# System Architecture

This is the concrete architecture authority; apply the
[Architecture Rules](../rules/ARCHITECTURE.md). The
[current baseline](../states/CURRENT.md) distinguishes implemented behavior
from this approved target.

## Product Shape

NXVM retains an extensible multi-machine architecture. XT, AT, DeskPro 386,
default PC/AT and later PC110 each link one fixed machine composition, using the same
App, Common, Lib and x86 tooling. Build configuration selects that executable's
external BYOB asset root; the selected Profile determines its fixed firmware
roles and relative asset names. A shared NXVM.ini configures supported memory,
media paths/access and presentation, not machine identity, firmware paths, CPU
population, controller topology, startup actions or firmware boot order. It
replaces YAML at the implementation cutover, not through a permanent parallel loader.

All implemented machines and their required personalities remain supported;
new Standard-board selection is not a migration prerequisite. PC110 requires
its own 486-class contract, not a renamed 386 profile.
This implies no VDM, mantle, DOS implementation or DLL product. Existing
release behavior remains the baseline until an implemented, verified cutover.

## Modules, Ownership, And Assembly

- `app` owns INI syntax, runtime-media paths, product CLI, recording and the one composition root.
  It assembles Common Session/UI/Machine and the NXVM driver.
- `core/machine` is that driver: asset/media lifetime, bounded execution,
  pacing and copied input/output/debug adaptation. It has no machine-name
  switch, independent lifecycle queue or guest-device state.
- `core/profiles` owns each board's actual composition: device construction,
  wiring, clocks, memory constraints, firmware slots, fixed relative asset names
  and board-specific behavior.
  It constructs and destroys the selected machine through neutral device
  contracts; it does not depend on Common or Machine-adapter internals.
- `core/devices` (currently `core/core`) owns CPU, memory, bus, devices, reset,
  generic execution and faults and the sole guest
  timeline. Generic mechanisms know hardware contracts, not product names.
- `common/machine` owns the shared execution/control protocol and paused-debug
  lease; `common/session` is the sole product-control reducer;
  `common/ui` binds Lib KVM and the Console broker.
- `x86/debug` owns Debug CLI continuations; `x86/xasm32` owns assembly and
  disassembly. Paused Debug operations go through Common Machine and the NXVM
  driver to Core, not a second machine path.
- `lib` owns platform/C-runtime services. Lib/Common/x86 remain product-neutral
  and source-shareable with SoftPC.

### Fixed Composition Without A New Framework

Build selection supplies one profile composition entry to the adapter:

```text
build-selected profile + build-provided BYOB asset root + App INI options
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
not simultaneous XT, PC/AT and Compaq ROM/state records. Moving directories
alone is insufficient: board constructors currently in Machine move to Profile;
Machine retains execution, host-resource lifetime and Common adaptation only.
Injected asset/media services use neutral contracts, avoiding a Profile-to-
Machine dependency cycle. Adding a board needs a composition and build entry,
not another Common queue, App parser or generic-device machine-name branch.

The build root is a local CMake input, not a tracked absolute path, ROM payload
or runtime configuration selector. A build validates that the selected
profile's declared external asset set is present and identifies the expected
slot, size and hash. The generated executable may retain its local root as its
only firmware lookup base: a recipient who compiles must supply its own lawful
root. Missing, mismatched or unreadable firmware is a hard startup/build
validation failure; there is no embedded, YAML, INI or cross-profile fallback.

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
reduce files. ROM bytes are immutable external inputs located below the
build-provided root; seed configuration
initializes Core-owned writable CMOS rather than a second BIOS/register mirror.

Guest writes flow through sole device state into copied snapshots, Common UI
and Lib KVM. Native handles, fonts and presentation are not guest-video owners.
All executables retain the same lifecycle and UX. The shared INI is one format
and parser, not a promise that every memory size or disk fits every board.
Omitted values use selected-profile defaults; explicit unsupported values fail
clearly rather than selecting another board or silently changing hardware.

Each selected product deploys once to the ignored
`assets/binary/<profile>/` directory, alongside its generated
`NXVM.ini`. That is the only current executable location; `build/` remains
compiler state apart from historical evidence, and `assets/sessions/` retains
only checked-in INI templates and integration inputs for current work.

## Runtime Admission Boundary

The [source policy](../etc/operations/policy/source-policy.md) owns asset and
redistribution handling. Profile-local ROM code means slot/mapping declarations
and authorized source, not permission to commit vendor binaries. Boot order and
POST remain firmware behavior; no generated BIOS, synthetic F1 or silent asset
fallback substitutes for missing hardware.

[Roadmap](ROADMAP.md) and [Current](../states/CURRENT.md) own sequencing and
implemented status. The [T533 consolidation record](../history/M5-T533-fixed-machine-products.md)
maps this design to observed code and bounded migration evidence.
