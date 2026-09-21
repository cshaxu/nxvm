# Fixed-Machine Products And Architecture Consolidation

## Purpose And Approval Boundary

Preserve an extensible multi-machine architecture with one build-fixed profile
per executable: all implemented IBM 5160 XT, IBM 5170 AT, DeskPro 386 and
default PC/AT machines, and later PC110. Preserve all existing CPU models,
profiles, selection tables and tests even when unused by these products.

This proposal revises the documentation-governance direction; it does not
admit a numeric implementation T or reopen closed T532. [Architecture](../design/ARCHITECTURE.md)
is normative; [research](../etc/research/fixed-machine-selection.md) records
source observations. Standard-board selection is no longer a prerequisite or
replacement goal. PC110 remains future implementation, not part of this structural
task's bootability claim. This task preserves all current model variants.

## Configuration Contract

- Each build receives exactly one local `NXVM_PROFILE_ASSETS_ROOT`, locating a
  user-supplied `nxvm-assets` root. The compiled profile identifies required
  firmware/CMOS/font roles and relative files through its manifest; CMake checks
  their presence, size and hash and generates an ignored local root binding.
  Firmware remains external at runtime and is never copied into the EXE.

- One NXVM.ini beside the EXE, one parser and one schema for all machine builds.
  Follow the inspected SoftPC INI scope: memory, media paths and independent
  access modes, console/window and existing console_control semantics.
- INI does not select the machine, CPU or firmware assets and does not specify a startup mode,
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
  suits every board. Report incompatible geometry/capacity clearly.
- Board firmware roles, layout, seed rules and manifest belong to the selected
  profile. All protected bytes remain external. The CMake root is local build
  configuration, not a committed machine-local path, payload embedding, INI
  firmware key or hidden YAML loader.
- Replace the production YAML/catalog path completely at cutover, migrating
  integration inputs through the same INI loader. Preserve user-owned output
  configuration; do not overwrite it during builds or silently convert it.

## Ownership And Simplification

- `core/core` becomes `core/devices`, together with its mirrored tests. It keeps
  reusable CPU/controllers, memory/bus, reset, generic execution and the sole
  timeline. This is not permission to split one machine into device workers.
- `core/profile` migrates to `core/profiles`, with mirrored tests and no permanent
  singular alias. `core/profiles/{xt,at,model40,default_profile,pc110}` owns composition, including
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
- All current machines are retained, not retirement candidates. Preserve default
  PC/AT, Compaq CECG/WD/D4 and other required personalities without conditioning
  them on a future board choice. Remove only proven dead/duplicate mechanisms;
  no live machine feature or regression is removed to make the tree smaller.

## Proposed S Batches

- **S1 - baseline and complete contract.** Inventory every profile, constructor,
  reset/provider branch, asset role, config key, test and build target. Compare
  SoftPC INI behavior, freeze memory/slot syntax, profile asset manifest and
  CMake-root validation/binding contract.
  Map every current XT/AT/DeskPro/default variant, including admitted CPU choices,
  to retained build/config/test coverage. New-board research is outside this task.
- **S2 - Devices and Profiles source/test relocation.** Move reusable Core and matching tests,
  and rename the singular Profile owner to plural Profiles,
  update includes/build/gates and remove former paths without changing hardware
  behavior or shared corpora. Preserve every CPU implementation and selection table.
- **S3 - Profile composition and Machine adaptation.** Move actual board wiring
  to Profile, flatten redundant resolution, unify lifetime/rollback and bind the
  same Common driver. Prove XT/AT/DeskPro/default regressions before removing
  old branches; no transitional second machine or firmware path remains at exit.
- **S4 - fixed builds and App INI cutover.** Use one parameterized build recipe
  selecting one composition per EXE and one required BYOB assets root. Implement
  profile-manifest validation/root binding and the common INI entry, remove runtime
  machine/YAML/firmware selection, migrate every retained integration scenario, and
  update artifact/status checks together. Unit parser tests use code-owned inputs only.
  Expose only implemented targets; PC110 is not a placeholder bootable executable.
- **S5 - duplicate cleanup and full closure.** Reconcile all config/asset/test/build rows,
  remove only proven dead/duplicate paths and audit one construction/reset/media/display
  route. Run full suites and build both host architectures for each admitted
  runnable machine. Record real code reductions and unchanged shared-corpus hashes.
  Every implemented machine and variant remains accounted for, not replaced by
  a selected Standard board. PC110 remains in its separate implementation proposal,
  never a fabricated usability claim from a successful structural build.

## Exit And Stop Conditions

Each S closes only with its complete unit suite and required affected integration
proof; T closure requires complete unit/integration and dual-architecture artifacts
under Execution. Keep original successful/failing scenario dispositions visible:
do not erase any existing machine coverage or relabel a failing baseline as retired.

One compiled profile, one validated BYOB root, one INI parser, profile-owned composition, one Core clock
and one Common adapter must be demonstrated in code, not just directory names.
Missing firmware/hardware blocks that product's qualification, not permission
to select a substitute secretly. CPU preservation does not claim unimplemented
80188/486 support. No VDM, new plugin framework, scheduler, protected binary
import or user YAML mutation is part of this proposal.
