# Fixed-Machine Products And Architecture Consolidation

## Purpose And Approval Boundary

Preserve an extensible multi-machine architecture with one build-fixed profile
per executable: IBM 5160 XT, IBM 5170 AT, an evidence-selected Standard 386DX
(DeskPro 386 is eligible), and later PC110. Preserve all existing CPU models,
profiles, selection tables and tests even when unused by these products.

This proposal revises the documentation-governance direction; it does not
admit a numeric implementation T or reopen closed T532. [Architecture](../design/ARCHITECTURE.md)
is normative; [research](../etc/research/fixed-machine-selection.md) records
source observations. Neither the Standard selection nor PC110 readiness is frozen.

## Configuration Contract

- One NXVM.ini beside the EXE, one parser and one schema for all machine builds.
  Follow the inspected SoftPC INI scope: memory, media paths and independent
  access modes, console/window and existing console_control semantics.
- INI does not select the machine or CPU and does not specify a startup mode,
  automatic start action or firmware boot order. The owner explicitly excluded
  those responsibilities. BIOS/CMOS and existing lifecycle commands retain them.
- Relative paths resolve from the INI directory; absolute paths remain valid.
  Keep readonly/direct/overlay semantics and physical slot order, not DOS letters.
  Preserve required multi-drive capability; freeze one simple slot-key convention
  in S1 rather than adding per-board parsers or silently dropping second drives.
- Missing memory uses the compiled board default; supplied values must fit that
  board. S1 defines one memory-unit syntax supporting sub-MiB XT RAM. SoftPC's
  current integer memory_mb alone cannot express it. Do not add parallel unit
  aliases, per-machine sections or silently clamp unsupported values.
- A shared file is not a claim that a single explicit memory/media combination
  suits all four boards. Report incompatible geometry/capacity clearly.
- Board firmware roles, layout and seed rules belong to the selected profile.
  S1 fixes their external deployment/path resolution without expanding INI into
  a hardware manifest. All protected bytes remain external; no baked-in BIOS,
  machine-local absolute path or hidden YAML firmware loader is acceptable.
- Replace the production YAML/catalog path completely at cutover, migrating
  integration inputs through the same INI loader. Preserve user-owned output
  configuration; do not overwrite it during builds or silently convert it.

## Ownership And Simplification

- `core/core` becomes `core/devices`, together with its mirrored tests. It keeps
  reusable CPU/controllers, memory/bus, reset, generic execution and the sole
  timeline. This is not permission to split one machine into device workers.
- `core/profile/{xt,at,standard,pc110}` owns actual board composition, including
  construction/wiring, clocks, memory constraints, firmware slots and necessary
  board behavior. Create future directories only with real implementation.
- Move board constructors now in `core/machine/machine.c`,
  `model40_composition.c` and lifecycle firmware switches to their profile owner.
  Keep the generic adapter's prepare/bind/execute/observe/release sequence once.
  Inject neutral asset/media services; Profile must not depend on adapter internals.
- Replace recursive profile parents, provenance-string mirrors and duplicated
  resolved states with direct frozen descriptions where caller evidence permits.
  Keep port/window/route conflict validation and real hardware constraints.
- `core/machine` adapts one assembled Devices instance to `common/machine`,
  retaining needed host-resource/media lifetime and pacing, not board policy,
  a second lifecycle reducer or simultaneous buffers for every board.
- App owns entry, INI values/paths, CLI and composition of Common plus the adapter.
  It does not build chips or duplicate profile validation. Lib/Common/x86 remain
  unchanged unless a concrete separately approved shared-contract gap appears.
- Keep FDD/HDD adapters where they own geometry/change semantics. Reuse Lib
  Storage and the existing Common queues, paused-debug lease and Lib KVM.
- XT and AT are retained, not retirement candidates. Do not delete Compaq
  CECG/WD/D4 or DeskPro construction while Standard selection is unresolved.
  Retire genuinely unrelated personalities only after a caller/coverage map;
  preserving a required non-Intel part is preferable to a false generic substitute.

## Proposed S Batches

- **S1 - baseline and complete contract.** Inventory every profile, constructor,
  reset/provider branch, asset role, config key, test and build target. Compare
  SoftPC INI behavior, freeze memory/slot syntax and external firmware resolution.
  Research DeskPro, Intel Model 302 and AMI alternatives against matching manuals,
  firmware, supported devices and implementation cost; record unresolved choices.
  XT/AT structural work need not wait for a new Standard board selection.
- **S2 - Devices source/test relocation.** Move reusable Core and matching tests,
  update includes/build/gates and remove former paths without changing hardware
  behavior or shared corpora. Preserve every CPU implementation and selection table.
- **S3 - Profile composition and Machine adaptation.** Move actual board wiring
  to Profile, flatten redundant resolution, unify lifetime/rollback and bind the
  same Common driver. Prove XT/AT and existing DeskPro regressions before removing
  old branches; no transitional second machine or firmware path remains at exit.
- **S4 - fixed builds and App INI cutover.** Use one parameterized build recipe
  selecting one composition per EXE. Implement the common INI entry, remove runtime
  machine/YAML selection, migrate every retained integration scenario, and update
  artifact/status checks together. Unit parser tests use code-owned inputs only.
  Expose only implemented targets; PC110 is not a placeholder bootable executable.
- **S5 - retirement and full closure.** Reconcile all config/asset/test/build rows,
  remove only proven unrelated paths and audit one construction/reset/media/display
  route. Run full suites and build both host architectures for each admitted
  runnable machine. Record real code reductions and unchanged shared-corpus hashes.
  Standard selection/hardware and PC110 implementation remain explicit receivers,
  never a fabricated usability claim from a successful structural build.

## Exit And Stop Conditions

Each S closes only with its complete unit suite and required affected integration
proof; T closure requires complete unit/integration and dual-architecture artifacts
under Execution. Keep original successful/failing scenario dispositions visible:
do not erase retained XT/AT coverage or relabel a failing baseline as retired.

One compiled profile, one INI parser, profile-owned composition, one Core clock
and one Common adapter must be demonstrated in code, not just directory names.
Missing firmware/hardware blocks that product's qualification, not permission
to select a substitute secretly. CPU preservation does not claim unimplemented
80188/486 support. No VDM, new plugin framework, scheduler, protected binary
import or user YAML mutation is part of this proposal.
