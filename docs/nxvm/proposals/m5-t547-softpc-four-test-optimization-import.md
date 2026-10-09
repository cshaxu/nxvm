# SoftPC Four-Test-Package Optimization Import

## Goal

Reconcile the complete current SoftPC/NXVM shared eight-component corpus into
one NXVM baseline. Preserve NXVM's newer, source-backed CPU/board repairs and
their regressions; import every remaining valid SoftPC correction without
weakening either corpus. The result keeps each shared test package independently
buildable and serializes only native desktop resources that genuinely conflict
across host-width build trees.

## Boundary

The authoritative upstream candidate is clean SoftPC commit `c6413911`.
Its actual test optimization changes `test/lib/CMakeLists.txt`,
`test/lib/console_broker_display_smoke.c`, and the shared
`test/register.cmake` helper. It does not change `test/common`.

SoftPC's broader `src/x86`, `test/x86` and `test/ibmpc` differences predate
NXVM T546 and omit NXVM CPU/board repairs. Each difference must receive a
ledger disposition: import a still-missing SoftPC correction, retain an NXVM
repair that is newer or stronger, or record a source-backed supersession. This
task must not overwrite, delete, or weaken NXVM-owned assertions.

## S1: Eight-Component Reconciliation And Native-Test Isolation

- Transfer the three same-path optimization changes from `c6413911`.
- Compare `src/{lib,common,x86,ibmpc}` and `test/{lib,common,x86,ibmpc}`
  against `c6413911`; produce a path-level disposition for every non-identical
  path. Import only valid missing SoftPC corrections while retaining NXVM T546
  source-backed repairs and regressions.
- Verify the four packages `test/{lib,common,x86,ibmpc}` separately: their
  CMake registration, manifests, package boundary gates, and selected test
  aggregates must remain valid on x64 and x86.
- Preserve the `src/x86`, `test/x86` and `test/ibmpc` T546 repairs and
  regressions; record their historical upstream divergence rather than
  replacing them with older SoftPC expectations.
- Update provenance/evidence and NXVM task state. No production source,
  firmware, media, INI, artifact, or App test is in scope.

## S2: Shared Four-Package Test Stability

- Treat S1's reconciliation as the fixed baseline; do not re-import or replace
  shared production code while diagnosing tests.
- Inventory each package's CTest registration, scratch ownership, native
  resource lock and static self-test invocation on both host widths.  Run each
  registered package aggregate once after a fresh configured build.
- Repair only the owner-local test setup, fixture cleanup or registration that
  demonstrably permits stale inputs, shared scratch state or an invalid host
  contract.  A production-path change is out of scope unless a current failure
  proves that path is the sole owner; report it before editing.
- Do not hide an intermittent test by retrying, sleeping, increasing a timeout,
  weakening an assertion or broadly serializing unrelated tests.  A long static
  test must complete through its correct registered path, not through a
  controller-imposed execution cap.
- Set the aggregate's default parallelism only to the highest observed safe
  level for the CPU-heavy package mix; do not alter individual test budgets.
- Preserve the independently importable `test/lib`, `test/common`, `test/x86`
  and `test/ibmpc` ownership boundary and update only their affected manifests
  and task evidence.

## S3: Shared Test Semantic Naming

- Remove task and subtask history identifiers from all current shared test
  filenames, CTest targets, internal test symbols, comments and result markers.
  Keep task identifiers only in task history and evidence.
- Audit all four package roots. `test/lib` and `test/common` require no rename
  when they contain no task-derived current names; complete the semantic rename
  in `test/x86` and `test/ibmpc` without moving tests between owners.
- Name each test for its owned behavior. In particular, the 80386 secondary
  integer timing test uses `machine_80386_secondary_integer_timing_smoke.c`,
  `machine-80386-secondary-integer-timing-smoke`,
  `secondary_integer_timing_*`, and `80386:SECONDARY-INTEGER-TIMING:OK`.
- Update registration and manifests atomically. No production source, public
  API, executable input, firmware/media/INI, App code or MyNES artifact is in
  scope.

## S4: Full Project Qualification

- Treat S1-S3 as the fixed source baseline. Do not make a feature, ownership,
  artifact, firmware, media, INI or MyNES change while qualifying it.
- Build and run the complete repository-only unit suite once on x64 and once
  on x86, including all four independently owned shared packages and both
  NXVM and MyNES product packages.
- Run each registered external integration suite once per configured host
  width when its immutable external inputs are available; record any missing
  prerequisite or failure as a result, not a reason to retry, weaken, or
  change a test.
- Record commands, counts, pass/fail results and every unexecuted desktop or
  external gate. This is qualification evidence only; it creates no new
  executable artifact because it changes no executable input.
- If qualification exposes a deterministic product test reference made stale by
  S3's accepted semantic rename, update that reference to the current semantic
  identity without weakening its assertion, then re-run the affected complete
  unit qualification. A user-owned MyNES executable rebuilt from its current
  source is reconciled as a separate MyNES artifact commit only; it does not
  alter the qualification baseline or runtime source.

## S5: Shared IBM PC Product Replacement

- Rename the legacy `src/ibmpc/product` out of the live route, import the
  owner-controlled SoftPC `src/app-softpc/product` command and keyboard base,
  then delete the temporary legacy directory before closure.
- Preserve SoftPC as canonical for Debug commands, hotkey names/actions and
  raw-Console help.  Do not retain the old NXVM parser, snapshot grammar or a
  second hotkey implementation.
- Make the only App seams explicit: opening text/config filename,
  fixed machine factory, App-specific INI adaptation, and an optional
  synchronous extra-command registry.  `INFO`/`SPEED` and `floppy` use that
  registry; they must not become Product dispatcher cases.  Product owns the
  complete help layout: App help is inserted after `exit` and before the shared
  hotkey section, while App handlers run only for a shared-unrecognized line.
- Run a complete similar-route sweep for old command/snapshot/hotkey/overlay
  paths.  Repair stale test fixtures so they link the sole production Product
  implementation rather than compiling private copies.
- Verify source and test manifests, IBM PC dependency gate, Product tests on
  x64/x86, all four PC App x64/x86 artifact builds, then the existing T547
  complete unit/integration qualification.  Do not modify the owner INI.

## S6: NXVM-Family Product Ownership

- Adopt SoftPC S16's ownership boundary without importing its App runtime:
  `ibmpc/product` remains portable, while the NXVM.ini grammar, executable-path
  lookup, four-App machine-factory adaptation, NXVM identity and NXVM-only
  command extensions move to `src/app-base/product`.
- Move the corresponding direct INI and factory unit tests to
  `test/app-base/product`.  They are NXVM-family tests, not generic IBM PC Product
  tests.  The individual `app-*` roots retain only their fixed bindings and
  their own product/integration tests.
- Remove `ibmpc/nxvm`, its CMake target, IBM PC manifest entries and its
  dependency-gate member.  Do not leave forwarding headers, compatibility
  targets, duplicate INI parsers or a Product dependency on NXVM support.
- Add one NXVM-family source/test manifest and retain package-local C11,
  Types and inward-boundary checks.  Update only affected NXVM architecture and
  source-layout authorities.
- Verify the NXVM-family manifest, IBM PC manifest/dependency gate, both moved
  unit tests on x64/x86, and all four App build receivers.  This S does not
  claim T-level full unit/integration qualification. Rebuild the existing eight
  0546 artifacts when relocation changes their link input; do not create a new
  artifact version.

## Completion Standard

The imported resource lock serializes only native desktop tests, not every Lib
test. The display smoke captures the actual visible console rectangle rather
than assuming a fixed host viewport. Every source/test difference has a
recorded import, retention or source-backed-supersession disposition; all four
package manifests match their own trees, each package test registration remains
independent, and dual-width package verification passes. Any unrelated native
desktop failure is reported as such; it is not concealed through retries or
test removal.
