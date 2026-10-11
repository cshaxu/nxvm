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

## Exit Criteria

- Every four-App file is accounted for in the ownership ledger.
- Confirmed defects reached by a four-App route are repaired at their actual
  owner in this T, with owner-local tests and affected-App regression proof.
- No App-local compatibility patch or separate-task deferral substitutes for a
  confirmed Core/Shared repair.
- No App production or test path depends on a sibling App.
- Relevant x64/x86 App routes, manifests and documentation gates pass.
- Artifact updates correspond only to changed executable inputs.
