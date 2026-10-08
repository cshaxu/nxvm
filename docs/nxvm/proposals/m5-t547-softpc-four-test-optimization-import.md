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

## Completion Standard

The imported resource lock serializes only native desktop tests, not every Lib
test. The display smoke captures the actual visible console rectangle rather
than assuming a fixed host viewport. Every source/test difference has a
recorded import, retention or source-backed-supersession disposition; all four
package manifests match their own trees, each package test registration remains
independent, and dual-width package verification passes. Any unrelated native
desktop failure is reported as such; it is not concealed through retries or
test removal.
