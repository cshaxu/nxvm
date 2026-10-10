# M5 Test-Name Normalization

## Goal

Normalize every active NXVM and Shared test identity so a source filename,
test target, CTest name, success marker and diagnostic describe the behavior
and its owning component, never the task that happened to create it.  The
result must remain straightforward for SoftPC to import from the Shared trees
without product names, historical task identifiers or build-local aliases.

## Scope

The live scope is the tracked NXVM/Shared test and registration graph:

- `test/lib`, `test/emulator` and `test/product`;
- `test/core` and the four PC App test roots;
- their CMake registrations, component-local manifest entries, test-only
  helpers, result markers and live static verifiers.

Historical task records, evidence ledgers, accepted artifact names, public
production APIs and MyNES-private test sources are out of scope.  Shared test
changes will be reviewed against every receiving App, but no MyNES source or
test file may be changed under this NXVM task.

## Canonical Identity Contract

Each active test has one behavior identity:

- C/CMake filenames use the established lowercase, underscore-separated
  behavior name and remain in their actual owning component directory.
- Ninja targets use lowercase dash-separated owner/behavior names.
- CTest names use the existing component-qualified dotted form.
- Explicit success markers use uppercase component/behavior text and contain
  no milestone, task, subtask, temporary experiment, retired path or profile
  aggregator identity unless that fact is the asserted behavior.
- Helpers and negative gates name the forbidden condition or owner contract,
  rather than the migration task that introduced them.

The task does not mechanically rename a correct behavior identity merely to
make styles look uniform.  A name is changed only when it is historical,
misowned, ambiguous, implementation-shaped where the assertion is behavioral,
or inconsistent with the same test's registered identity.

## Plan

1. **S1 — Inventory and mapping.** Freeze every live identity and all its
   references, classify each as retain or rename, and record one old-to-new
   mapping before editing a test.  Verify no active route already has a name
   collision.
2. **S2 — Shared test identities.** Normalize Lib, Emulator and Product
   source/target/CTest/marker identities and their manifests without reaching
   into a consumer App.
3. **S3 — Core identities.** Normalize Core chip, x86, board and machine
   identities; preserve timing-ledger semantics while removing task-shaped
   names from active sources and registrations.
4. **S4 — PC App unit identities.** Normalize each fixed App's profile,
   composition and machine-unit identities while retaining the selected model
   as an asserted behavior where it is genuinely App-owned.
5. **S5 — PC App integration identities.** Normalize active integration
   helper names, registration helpers, diagnostic markers and success markers.
   Preserve their assertions, external inputs and historical evidence paths.
6. **S6 — Closure.** Run static collision/provenance sweeps, all relevant
   component manifests and complete repository-only Unit suites on x64/x86.
   This is test/build-only work; artifacts are rebuilt only if a runnable
   product input changes, which is not expected.

## Boundaries

No test assertion, fixture semantics, production code, component dependency,
firmware/media input or external integration behavior may change merely for a
rename.  Every removed old identity must have an exact new receiver in the
S1 mapping.  Do not retain compatibility aliases or duplicate CTest entries.

## Exit

The complete live scope has an audited one-to-one identity mapping; no active
name embeds historical task provenance; all manifests and component gates pass;
and complete repository-only Unit suites pass on both supported architectures.
