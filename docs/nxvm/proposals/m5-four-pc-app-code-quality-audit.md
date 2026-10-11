# Four PC App Code Quality Audit

## Objective

Audit the four NXVM PC application owners as separate products:

- `app-my5160`
- `app-my5170`
- `app-mydeskpro386`
- `app-nxvm`

The audit evaluates application-owned source, tests, build wiring, configuration
loading, profile composition, firmware bindings, artifact rules, and
documentation against the established ownership boundaries and coding rules.

## Scope And Boundaries

Each App owns only its fixed machine identity, product identity, INI/request
loading policy, fixed profile composition, firmware binding, application-local
commands, tests, assets and documentation.

The audit must separately identify:

- duplicated implementation among the four Apps;
- code that belongs in the existing Core shared-PC layer;
- App code that illegally reaches a sibling App or bypasses a public Core or
  Shared boundary;
- stale, dead, misleading or duplicated configuration, build, test and
  documentation paths;
- incomplete error, teardown, asset and artifact contracts.

It must not make speculative architecture changes or change firmware/media/INI
runtime semantics without a reproduced contract defect. A real defect exposed
by one of the four App routes remains in T553 until it is repaired at its
actual owner and verified through every affected App route; it is not deferred
to a separate task merely because that owner is Core or a shared component.
The repair must preserve the inward dependency direction rather than moving
the owner into an App.

## Proposed S Tasks

### S1: Four-App Inventory And Ownership Ledger

Build a per-App source/test/CMake/document/asset inventory, a caller and
dependency map, and a duplicate-path ledger. Classify every candidate as
App-local, Core candidate, Shared candidate, valid machine distinction, or
retired/stale. Do not change production behavior in this S.

### S2: App Boundary And Build/Test Ownership Repair

Repair demonstrated App-local boundary, naming, build registration, manifest,
test ownership or documentation defects from S1. Keep each commit target-scoped;
transfer a confirmed Core or Shared receiving change to its own approved task.

### S3: App Lifecycle, Configuration And Artifact Contract Review

Review each App's request loading, fixed composition, failure propagation,
teardown, artifact selection and product-facing configuration behavior. Repair
only reproduced App-owned defects; record any cross-owner receiver separately.

### S4: Final Four-App Qualification

Re-run applicable x64/x86 build and App-owned unit/setup/integration routes,
manifest and documentation gates. Rebuild only those artifact pairs whose
runtime build input changed. Report residual distinctions and transferred work;
do not claim broader Core or Shared correctness.

### S5: Corrective Full-Route Quality Audit And Repair

Reopen the task after the incomplete S4 closure. Build a finite ledger of all
production, test, CMake and configuration paths reached by the four Apps;
audit code quality, failure paths, ownership, duplicate mechanisms and user
visible behavior. Repair every confirmed defect in this T at its actual owner,
including Core or a shared component when it is on a four-App route. Perform a
similar-issue sweep for each repaired mechanism, then requalify all affected
Apps on x64 and x86 before a later closure review.

#### S5 Audit Conclusions And Receiver Plan

The App-owned branch is now audited before receiver work begins:

- no App production source directly includes a peer App or reaches inward
  through an App-private Core path;
- the four `main` and fixed-binding implementations are intentionally small
  identity/firmware facts, not duplicated lifecycle, UI, configuration or
  machine-owner implementations; and
- the shared `core/product` route remains the sole common NXVM-family INI,
  compose, banner and Product-runner owner.

Accordingly, no App production rewrite is admitted merely to make the four
bindings look alike. The confirmed full-route defects are below their actual
Core or Core-test owners. S5 remains responsible for their repair and for any
additional demonstrated four-App-route defect; it does not close merely
because the App-owned source branch is clean.

The initial Core audit also establishes these facts:

- production Core has no direct include of an App root, and its existing
  `core/verify_corpus.cmake` correctly rejects an outer-component include;
- the old repository-wide dependency-DAG checker has not been migrated to the
  current `core/{chips,x86,board-base,board-xt,board-at,machine,product}`
  topology. It therefore reports designed directional Core edges as forbidden
  migration exceptions and cannot serve as architecture evidence;
- `test/core` contains a Model 40-only DOS diagnostic and private helper under
  a generic Core root, and a cross-App Console integration helper retains an
  NXVM-only name despite serving NXVM and DeskPro routes; and
- `test/core/setup/session_ini.c` unnecessarily compresses several existing
  `lib_status` failures to `LIB_STATUS_INTERNAL_ERROR`. The exact status
  contract of every remaining compression and ignored cleanup result must be
  classified before changing it; no blanket mechanical rewrite is allowed.

### S6: Core Dependency-DAG Gate Repair

Replace the pre-restructure owner classifier and migration allowlist with one
current Core component graph. Derive allowed direction from actual public
interfaces and CMake links, retain rejection of App/outer dependencies and
add a focused negative proof. Inspect every apparent reverse edge before
allowing it; do not turn the checker into an unconstrained allowlist.

### S7: Core Test Ownership And Shared-Harness Repair

Move the Model 40-only diagnostic and its private profile helper to the
MyDeskPro386 test owner. Rename or narrow the shared Console process helper so
its name describes its cross-App PC behavior while retaining the one shared
`NXVM.ini` contract. Update registrations, manifests and route/ownership
gates; prove both affected App routes and the independent Core package remain
correct.

This S also removes stale internal path terminology from Core comments. It
does not mechanically rename stable `vm_machine_*`, `vm_app_*`, `vm_profile_*`
or `vm_at_*` contracts merely because the repository directories moved: those
are current, selected-binding or hardware-domain names. The only symbol rename
is the demonstrated cross-App test helper whose `nxvm_` name falsely claims
single-App ownership.

### S8: Core Test Failure-Semantics Repair

Make `session_ini` propagate the existing `lib_status` of composition and
overlay transformation failures where no boundary translation is required.
Classify adjacent status compression and ignored results by contract, changing
only demonstrated lossy paths. Add failure injection at the helper boundary
and sweep every caller that receives the same status.

### S9: App-Facing Path-Reference Cleanup

Repair the confirmed obsolete `src/vm/profile/default_profile/firmware/`
reference in the NXVM firmware documentation and sweep the four App roots for
the same retired-layout wording. This is an App-facing documentation receiver,
not a pretext to rename selected-binding symbols or change fixed hardware
composition. Run the affected documentation/manifest gates.

### S10: Corrective Full-Route Qualification And Closure Review

Run the repaired dependency/ownership/manifest gates, then all four App
x64/x86 unit, setup and integration routes using the reusable build trees.
Rebuild only executable pairs whose production inputs changed. Perform an
actual-diff and clean-worktree review before proposing T closure.

### S11: Windows HDD Checkpoint Responsibility Repair

Keep the Windows checkpoint focused on guest-visible C-drive availability and
the BIOS BDA fixed-disk count. Remove its redundant dependence on the mutable
board ATA command counter and controller test fixture; the existing HDD boot
integration receiver remains the sole owner of direct ATA-command proof.
Run both integrations sequentially on x64/x86 before resuming final
qualification.

#### S10 Completion Evidence

The reusable default x64 and x86 trees each pass all 567 registered CTest
routes. The profile-specific integration receivers also pass on both widths:
My5160 2/2, My5170 4/4, and MyDeskPro386 4/4. Lib/Core manifest, Core corpus,
dependency-DAG and documentation-governance gates pass on both widths. S10 and
S11 change only test/support and documentation inputs; no executable build
input changed, so the deployed artifacts require no rebuild for this closure.

### S12: Corrective Core And Four-App Quality Audit

The prior S10 qualification proves every registered route passed, but it does
not itself prove that Core and every four-App production owner received a
complete code-quality audit. Freeze a current owner ledger for `src/core`, the
four `src/app-*` roots, their matching tests and CMake registrations. For each
owner group, inspect dependency direction, public-interface exposure, lifecycle
and failure boundaries, duplicate/parallel mechanisms, configuration/asset
boundaries, test ownership and CMake registration. Classify each row as
verified-clean, confirmed defect, not applicable, or transferred; a confirmed
defect must name its source/caller evidence and bounded repair receiver.

This is read-only except for the ledger and task records. It does not treat a
small test count, a historical record, a large file or a passing suite as proof
of a defect or of completeness. It does not edit Lib, Emulator, x86, firmware,
media, INIs, snapshots or artifacts. A later repair S requires its own explicit
scope and similar-issue sweep.

The discovered Core/x86 test-fixture class also requires a read-only scan of
all repository test packages for exact duplicate fixtures and cross-package
test includes. This is classification only: it does not broaden S12 into a
Shared code or test modification.

### S13: Core Shared-Test Fixture And INI-Owner Documentation Repair

Move the two exact Core-owned fixture duplicates (`exception_fixture.h` and
`memory_alias_fixture.h`) to the existing `test/core/support/` owner, update
all Core/x86 and Core/board-base callers, and remove both former copies. The
fixtures remain Core-test infrastructure: they consume the public Core/x86
contract but serve both Core/x86 and Core/board-base tests, so they do not
belong in either child owner or in the repository-level shared x86 corpus.

Correct the stale Architecture paragraph so that it matches the implemented
owner: `core/product` owns the one NXVM.ini grammar and executable-relative
runtime-media path resolution; Apps own fixed identity and profile bindings.
Do not modify production source, Shared Lib/Emulator/x86/Product, Apps,
assets, INIs or deployed executables. Update the Core test manifest and prove
the affected Core units plus Core manifest/corpus and documentation governance
gates on x64 and x86.

## Exit Criteria

- Every four-App file is accounted for in the ownership ledger.
- Confirmed defects reached by a four-App route are repaired at their actual
  owner in this T, with owner-local tests and affected-App regression proof.
- No App-local compatibility patch or separate-task deferral substitutes for a
  confirmed Core/Shared repair.
- No App production or test path depends on a sibling App.
- Relevant x64/x86 App routes, manifests and documentation gates pass.
- Artifact updates correspond only to changed executable inputs.

## Closure

T553 closed on 2026-10-10 after S1--S13. The final corrective audit and
repair are recorded in
[M5-T553-four-pc-app-code-quality-audit.md](../history/M5-T553-four-pc-app-code-quality-audit.md).
