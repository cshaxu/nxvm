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

It must not make speculative architecture changes, alter Shared or Core merely
because a common-looking pattern exists, or change firmware/media/INI runtime
semantics without a reproduced contract defect and a separately admitted
receiving task.

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

## Exit Criteria

- Every four-App file is accounted for in the ownership ledger.
- Confirmed App-owned defects are repaired with owner-local tests.
- Shared/Core candidates are transferred rather than patched in an App.
- No App production or test path depends on a sibling App.
- Relevant x64/x86 App routes, manifests and documentation gates pass.
- Artifact updates correspond only to changed executable inputs.
