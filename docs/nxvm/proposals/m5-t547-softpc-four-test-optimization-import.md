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

## Completion Standard

The imported resource lock serializes only native desktop tests, not every Lib
test. The display smoke captures the actual visible console rectangle rather
than assuming a fixed host viewport. Every source/test difference has a
recorded import, retention or source-backed-supersession disposition; all four
package manifests match their own trees, each package test registration remains
independent, and dual-width package verification passes. Any unrelated native
desktop failure is reported as such; it is not concealed through retries or
test removal.
