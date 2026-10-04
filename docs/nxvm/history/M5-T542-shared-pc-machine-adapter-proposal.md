# Shared PC Machine Adapter And Construction Helpers

## Admission And Goal

The owner admits M5 T542 on 2026-10-04 after closed T541 at
55f9fb1920af9316c6102b7aff5550fe1d7b6dfc. Extract the remaining mechanisms
shared by the four fixed PC builds before the independent App split. Today
they are one implementation under app-nxvm, not four duplicate Apps. Prevent
the later split from copying that implementation or importing a peer App.

Consumers are default, IBM 5160, IBM 5170 and DeskPro Model40 on x64 and x86.
Preserve firmware, hardware, INI, lifecycle, debugger, media and timing behavior.
Extraction is not hardware qualification.

## Receiving Owners And Hard Boundaries

- x86/product receives the PC-specific Core-to-Common Machine adapter. Its
  cohesive machine implementation owns bounded execution adaptation, pacing,
  copied input/frame/debug conversion, media-resource lifetime and factory
  preparation. Reuse Common's sole worker, FIFO, generation and paused lease;
  do not introduce another control loop or universal executor framework.
- x86/ibmpc-common receives proven PC construction helpers: floppy geometry,
  rate/double-step behavior, Option ROM validation and board-contract checks.
  It does not acquire host file access, Common control or App identity.
- App profiles retain fixed topology, constraints, clocks, ROM roles/layouts,
  CMOS contents, HDC personality/geometry choices and genuine Model40 D4.
  App firmware retains project-authored BIOS assembly and its offline build.
- App product retains immutable identity and fixed composition/factory binding.
  Shared code cannot include App headers or select machine names. Replace the
  all-profile plan union/dispatch with build-selected composition, not another
  runtime registry. The minimum public contract is frozen in S1, not guessed
  independently by each migration batch.

One owner prepares, publishes and rolls back each machine/resource candidate.
Core's attachment remains the board teardown owner; the adapter borrows its
handle. Model40 D4/FDC observations leave the generic adapter layout and stay
at their actual profile/board boundary, without a side registry. Public
construction consumes validated values and bounded operations, not mutable
App layouts. Chip state and guest time retain their existing owners.

Excluded: Lib/Common source/tests, MyNES, owner INIs, external masters, new
Apps or artifact-directory cutover, new ROM acquisition, CPU algorithm rewrites
and timing-grade upgrades. Existing chip/Core/family mechanisms are preserved;
only bounded integration changes required by the admitted extraction qualify.

## Planned S Sequence

Briefs are admitted sequentially from the S1 ledger and preceding evidence,
never as eight simultaneous packets. CURRENT.md owns the active S status.

1. **S1: Inventory and contracts.** Map all Machine/media/profile-helper/factory
   capabilities and test/build/tool consumers to receivers. Freeze dependencies,
   construction, lifetime and rollback. Exit: complete ledger, no unclassified
   path, concrete bounded interfaces and exact migration/verification batches.
2. **S2: PC construction helpers.** Relocate the floppy model, Option ROM
   validator and board-contract checks with callers/tests. Exit: one helper
   owner, no Shared-to-App dependency, unchanged four-board constraints.
3. **S3: PC media adapter.** Extract FDD/HDD providers and lease lifecycle.
   Preserve distinct geometry, address marks, protection and generation; Lib
   remains the only file/lock/direct/readonly/overlay implementation. Exit:
   all builds use one provider implementation; replacement/eject/failure
   cleanup preserve semantics and retire the old paths.
4. **S4: Input and display conversion.** Extract scan-set/mouse mapping,
   CP437, cursor geometry and copied frame conversion. Remove intermediate
   carriers only where direct conversion preserves the full contract. Exit:
   one ingress/frame path; unchanged text/graphics/palette/cursor behavior,
   no native handles or second guest-video state.
5. **S5: Profile observation ownership.** Remove Model40 D4 handles and FDC
   terminal observations from generic Machine state. The existing prepared
   Profile context owns those facts, exposes only copied observations, and
   invalidates/revokes them at successful reset/Core teardown. Preserve all
   Model40 probe predicates. This prerequisite is separated after S4 actual
   source review; no new registry or execution path is introduced.
6. **S6: Execution and debug adapter.** Move the cohesive remaining Machine
   owner: Common driver, bounded runner, pacing, HLT progress, reset/fault
   and paused-debug/budget completion. Keep Model40 attachment local. Exit:
   one adapter/Common worker/Core clock; no App include or model state in
   Shared. Do not generalize this into a worker shared with CCPU.
7. **S7: Fixed composition and Product binding.** Connect the four build-selected
   profiles to the shared adapter/factory; retire all-profile plan union and
   runtime dispatch, preserving real profile/ROM rules. Exit: four working
   fixed compositions, no peer-App imports or permanent forwarding shim.
8. **S8: Build/test and obsolete-path audit.** Reconcile source lists, manifests,
   standalone x86 builds, dependency gates and test ownership. Earlier batches
   must already repair their direct builds/tests; S8 cannot defer that duty.
   Exit: Shared builds without App source; old shared paths/targets removed;
   original assertions and every integration context remain accounted for.
9. **S9: Whole-task acceptance.** Review every ledger member and actual diff,
   unique state owners, code-size change, complete units/integration/manifests
   and artifacts. Exit: no unresolved extraction member, all gates pass,
   eight verified optimized stripped 0542 EXEs delivered, and the App-split
   proposal has no missing shared receiver. Do not automatically admit it.

## Convergence, Verification And Delivery

The [T542 record](../history/M5-T542-shared-pc-machine-adapter.md) holds the
durable convergence ledger: finite capability universe, original/receiving
paths, callers, state/failure owners, retained differences, regression owners,
batch and disposition. Accepted entries require proof; App-retained entries
require a concrete reason/owner. Unresolved shared mechanisms block closure
rather than being hidden by boot success or transferred to the App split.

Each code-changing S repairs direct consumers, updates affected manifests,
runs full repository-only units plus applicable regressions, reviews actual
diff and records added/removed/net code and retained production paths. Units
use code-owned inputs, not external ROM/INI/media. S1 design does not rebuild
unchanged EXEs or manufacture runtime evidence.

T closure requires full x64/x86 units, all preserved 58 integration contexts
once each with established predicates/inputs, standalone Shared checks, six
valid manifests and dependency/document gates. Code-changing batches rebuild
affected NXVM fixed profiles on both widths under assets/nxvm/<profile> with
T542 identity; owner INIs stay intact. MyNES is not an adapter consumer and
stays unchanged. Deliver separate target-scoped Shared and NXVM P commits
with immediate pushes and source/hash evidence.

## Stop Conditions And Simplicity

Stop affected work for required Lib/Common changes, new product behavior,
lost machine/CPU/test capability, new asset/license assumptions or broader
construction authority. Report newly discovered timing downgrades/L1 under
the owner's escalation requirement. Never silently reduce acceptance.

Shared semantics have one implementation; genuine differences stay visible.
No profile inheritance, plugins, device framework, duplicate queue, mirrored
state or forwarding-only wrapper. A material positive code increase needs a
named boundary and explanation; moving files is not code deletion.
