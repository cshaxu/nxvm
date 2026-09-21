# System Architecture

This is the concrete architecture authority; apply the
[Architecture Rules](../rules/ARCHITECTURE.md). The
[current baseline](../states/CURRENT.md) distinguishes implemented behavior
from this approved target.

## Product Shape

NXVM retains a multi-machine architecture. Standard and PC110 each link exactly
one fixed machine composition, with the same App, Common, Lib and x86 tooling.
YAML supplies assets and permitted session options, not another machine, CPU
population or controller topology. A machine identity may be checked for
mismatch; it is not a runtime selector.

Standard requires a source-qualified 386DX board whose identity is not yet
frozen. PC110 requires its own 486-class contract, not a renamed 386 profile.
Neither implies a VDM, mantle, DOS implementation or DLL product. Existing
release behavior remains the baseline until an implemented, verified cutover.

## Modules, Ownership, And Assembly

- `app` owns syntax, paths, product CLI, recording and the one composition root.
  It assembles Common Session/UI/Machine and the NXVM driver.
- `core/machine` is that driver: asset/media lifetime, bounded execution,
  pacing and copied input/output/debug adaptation. It has no machine-name
  switch, independent lifecycle queue or guest-device state.
- `core/profile` supplies immutable board configuration, topology, firmware-slot
  rules and necessary board-specific behavior. Standard and PC110 do not
  inherit from retired 5170 or DeskPro products.
- `core/core` owns CPU, memory, bus, devices, reset, faults and the sole guest
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

Build selection supplies one board description to one machine factory:

```text
build-selected profile + App options + externally loaded asset bytes
                              |
                              v
                one validated, frozen Core plan
                              |
                              v
             one Core instance and Common machine driver
```

Profile owns hardware constraints; App owns syntax/path resolution; Core owns
generic structural/state invariants. These are distinct checks, not copies of
one board rule in all three layers. Share constants or construction helpers
only where semantics match. Two fixed targets need no profile inheritance
engine, plugin registry, per-field provenance mirror or runtime machine registry.
Provenance belongs in evidence rather than repeated runtime strings.

Construction prepares assets and a plan, validates, publishes one live machine,
and rolls back through one resource owner on failure. Reset reuses the frozen
plan and Core reset path. The driver retains only the selected board's resources,
not simultaneous XT, PC/AT and Compaq ROM/state records.

### CPU Retention And Device Reduction

CPU identity, feature/timing tables and instruction dispatch remain Core-owned
and selectable by Core callers and repository-only CPU tests. Preserve all
existing models and tests even when no shipped machine uses them. New 188/486
coverage needs sources and implementation; an enum alias cannot turn 386 into
486. Fixed products choose their documented CPU once. Do not scatter build
macros through handlers or remove later CPUs' 16-bit, real-mode or VM86 semantics.

Device retirement follows the two-board inventory. Remove unneeded legacy
personalities and their configuration fields, not standard Intel chip behavior
or CPU-shared mechanisms. One VADP owner retains needed VGA/EGA mechanisms;
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
reduce files. ROM bytes are immutable external inputs; seed configuration
initializes Core-owned writable CMOS rather than a second BIOS/register mirror.

Guest writes flow through sole device state into copied snapshots, Common UI
and Lib KVM. Native handles, fonts and presentation are not guest-video owners.
Both executables retain the same lifecycle and UX.

## Runtime Admission Boundary

The [source policy](../etc/operations/policy/source-policy.md) owns asset and
redistribution handling. Profile-local ROM code means slot/mapping declarations
and authorized source, not permission to commit vendor binaries. Boot order and
POST remain firmware behavior; no generated BIOS, synthetic F1 or silent asset
fallback substitutes for missing hardware.

[Roadmap](ROADMAP.md) and [Current](../states/CURRENT.md) own sequencing and
implemented status. The [consolidation proposal](../proposals/m5-fixed-machine-products.md)
maps this design to observed code and bounded migration evidence.
