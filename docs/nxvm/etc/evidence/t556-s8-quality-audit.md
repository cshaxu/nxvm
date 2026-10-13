# T556 S8 - Comprehensive Quality Audit

## Baseline And Claim Boundary

Read-only implementation baseline: pushed `2df276f8c`, containing S7 repair
`307cf1bd6`. The owner accepted S7 and admitted S8 on 2026-10-13; confirmed
repairs belong to S9. This report is a quality/ownership audit, not renewed
instruction, electrical or guest compatibility certification.

The finite inventory is all 261 production C/H files in Core (187), MyNES
(33), My5160 (8), My5170 (7), DeskPro (18), and NXVM (8); their build/test
registration and current product authorities; and read-only Shared-six
manifest/dependency boundaries. Whole-inventory scans cover includes, names,
duplicate whole files and registration. Detailed manual review concentrates on
the moved Machine/Chips contracts, composition/media ownership, and the S7
memory/FDC/video changes. It is not a claim to have re-proved every instruction
handler or every line of all hardware algorithms.

## Confirmed Findings And S9 Disposition

### A1 - P1: Core test identity and owner are wrong

`test/core/x86_tests.cmake:249` and subsequent registrations pass `x86` and
an absolute source path to `shared_register_test`. `board_tests.cmake:25`
likewise passes absolute paths. The configured x64 inventory contains 156
absolute-path test names, of which 120 are Core cases labeled `x86`.
For example, the PIT case is
`x86.O:/repos.hobby/nxvm/test/core/chips/pit825x/pit_8253`.
This contaminates Shared-only selection and makes identities checkout-specific.

`test/core/verify_test_route_partition.cmake` counts owner labels but does not
check whether that owner matches the registering tree. Its passing result
therefore does not disprove this defect.

S9: repair Core callers to use Core-relative paths and the Core owner; update
target aggregates and direct test references together. Strengthen the existing
Core receiver with a negative ownership/name case. Do not modify
`test/register.cmake` or Shared-six files. Preserve every assertion and case.
Estimated counted test/build delta: +180/-155, net +25, primarily replacements.

### A2 - P2: DeskPro Chips/Machine split is incomplete

`chips/d4_memory.c` still constructs and installs memory routes, configures
Core parity observation and calls `core_machine_set_memory_alias_decode`.
`chips/d4_memory.h` stores a Core handle; `machine/d4_platform.h` embeds its
private state via a non-interface header. Register logic and Core attachment
are not separated as the T556 proposal requires. The current top-level DAG
collapses the complete App into one owner, so it cannot detect this split.

S9: keep D4 register/decoder semantics in Chips; let Machine own Core routes,
attachment and application of decoder results. Establish only the bounded
chip operations actually consumed; avoid a new manager, mirrored register
state or forwarding-only framework. Separate register tests from attachment
failure tests and extend the existing DeskPro boundary check. Preserve the
accepted no-F1 boot behavior. Estimated production +100/-100; tests +80/-40.

### A3 - P1: D4 configuration can hide a missing alias

`chips/d4_memory.c:214` discards the status from applying reset decode.
`core_machine_d4_memory_configure` has already installed routes and set
`configured` before calling this void reset, and then returns OK. The called
Core operation returns INVALID_ARGUMENT when the expected alias is absent.
The existing `d4_memory_transaction_smoke.c` uses
`test/core/support/memory_registration_fixture.c`, whose synthetic Core has
no such alias, yet expects successful configuration. Thus the current test
actually permits an incomplete attachment to be published as successful.
The normal Model40 plan installs the alias; this is a construction/failure
contract defect, not evidence that the accepted boot is still failing.

S9: validate/apply the required binding before publishing configuration and
preserve the existing rollback owner. Reset after valid construction may rely
on the immutable installed binding; do not invent a recovery loop or second
status flag. Add missing-alias rejection and successful real-alias fixtures,
including unchanged resources after failure. Implement with A2 at one owner.
Additional estimate: production +15/-5; tests +35/-5.

### A4 - P2: Moved owner vocabulary and boolean intent remain incomplete

MyNES `machine/driver_interface.h` still exports `core_driver_*`; its moved
debug protocol retains `core_debug_*`. DeskPro moved chip/platform headers
retain Core ownership guards/names and `model40_private.h` still uses a
privacy suffix despite interface-based visibility. D4 boolean state such as
`configured`, `present`, `d4_platform_nmi_signaled` and `d4_refresh_hold_pending`
is still `lib_u8`, while adjacent speed state correctly uses `lib_bool`.
These are naming/type-intent issues, not proof of wrong runtime values.

S9: inventory callers before a mechanical owner-local rename; use `lib_bool`
for actual logical state, retaining integer widths for registers, masks,
addresses and counts. Do not rename unchanged Core/chip symbols merely to
match a directory or alter snapshot encoding. Separate MyNES and NXVM commits.
Estimate: 250--450 paired replacement lines; near-zero net production change.

### A5 - P2: Current authorities disagree with the code and delivery

- NXVM CODING still identifies `test/{lib,common,x86,ibmpc}`, the sole ibmpc
  Machine/Product, and a family observer in `test/app-nxvm/integration`.
  The actual owners are Lib/Emulator/x86, private Core, and `test/core/setup`.
- NXVM ARCHITECTURE still describes every App's `product/` path although
  three Apps are now flat. Source policy names retired App `profiles/` paths
  and `ibmpc/board-common` as current firmware owners.
- MyNES CODING still declares Common source/tests and outdated grammar
  ownership. Its CURRENT opens with T555 S6 and retains a T550 active-shaped
  reading packet rather than clearly identifying accepted T556 S2 receipt.
- T556 main history still opens with S3 active and lacks S6/S7 reconciliation.
  NXVM CURRENT mixes prior live-looking packets and progress beneath the
  current task. Queue retains closed proposals as apparently active references.
  Local link checking finds no missing files: this is semantic staleness, not
  a missing-file failure.
- Eight PC artifacts still carry 0555 in T556. The execution rule normally
  binds changed executable revision to the active numeric T; no explicit
  T556 retained-version exception was found in the task evidence. Reconcile
  the next delivery to 0556 rather than silently treating this as compliance.
  MyNES's independent 0044 receiving identity is a separate product baseline.

S9: update current authorities and compact active packets; retain historical
evidence without rewriting old outcomes. Reconcile versions, banner/build
references and deployed pairs together. Expected documentation delta is
approximately +100/-150, excluding moving retained history verbatim.

### A6 - P2: Shared rule vocabulary needs a separate governance receiver

`docs/rules/DOCUMENT.md` still declares `src/{lib,common,x86}`. The existing
NXVM TODO already carries Shared-package vocabulary debt, but its explanation
also predates the current Lib/Emulator/x86 corpus. S9 may update that debt
description, not the rule itself: EXECUTION permits rule changes only under
a standalone Td. No Shared-six code change is requested by this audit.

## Verified And Retained Boundaries

- Fresh documentation structural gates pass for NXVM and MyNES. These do not
  validate semantic currency or all local links.
- Fresh x64 and x86 static CTest selections each pass 18/18: all six shared
  manifests, Core source/test manifests, registered corpus/types/ownership
  and route checks in the selection. The selection uses
  `^(lib|emulator|x86|core)\.(manifest|test-manifest|corpus|test-ownership|test-types-boundary|test-route-partition|app-test-support-boundary)$`.
- `Verify-DependencyDag.ps1` passes. A complete production include scan finds
  no Core-to-App or peer-App includes. MyNES Machine consumes the Core public
  machine interface rather than Core's mutable layout.
- SHA-256 grouping finds no byte-identical complete C/H files among the 261
  inspected production files. This does not assert that all repeated fragments
  or semantically duplicate algorithms are absent.
- Product guide/design/state Markdown local file links resolve in the checked
  set. Historical links and terms are not mechanically rewritten.
- Public six have no diff from pre-layout `8f195943b`; this audit changes no
  production, test, build, INI, firmware, media, snapshot or executable input.
- Exactly ten EXEs remain in the five flat artifact roots. Presence does not
  replace source/version qualification.

## Qualification Still Required

The current unified x64 catalog has 607 tests: 591 units (including 45 MyNES)
and 16 integrations. Integrations are configuration-dependent; this catalog
alone is not the complete four-PC-profile qualification universe.
S9 must enumerate and run every admitted profile route plus MyNES on both
widths using the existing persistent builds, without removing cases or
counting an unregistered profile as passed.

S7's named dynamic evidence is retained, not re-run or relabeled here: normal
DeskPro boot passes both widths; all 546 main units have passing evidence per
width, with isolated retries for static-test timeouts; MyNES 45 units pass per
width. Neither final parallel S7 main run was a single all-green run.
This S8 ran static gates only, not new full unit/integration or manual desktop
qualification. There is no claim of a full-repository all-green S8 run.

## S9 Order And Exit

1. Correct Core test registration and strengthen its existing ownership gate.
2. Finish DeskPro D4 register/attachment separation and atomic validation,
   preserving the existing Core dispatcher and direct boot route.
3. Finish bounded moved-owner naming/boolean cleanup, with separate MyNES P.
4. Reconcile product authorities, packet/history and artifact identities.
5. Run dual-width complete units and all admitted integrations, rebuild and
   verify affected artifact pairs, review actual diffs and push per target.

S9 may not turn the audit into unrelated hardware work. New unproven concerns
need evidence and bounded disposition; Shared-six changes still stop for
owner review. Shared-rule vocabulary has its explicit separate Td receiver.
T556 remains open until its original whole-task qualification predicate is met.
