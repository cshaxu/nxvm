# M5 Unit And Integration Test Convergence

> Retained proposal for the completed T500 task.

## Purpose

Restore a fast, trustworthy development feedback loop without weakening the
release regression boundary. The existing mixed 302-entry CTest aggregate is
only a migration baseline: it combines repository-only checks with
owner-managed media/product scenarios and its name obscures that distinction.
It also builds every current smoke executable before running CTest. The active
configured build tree has overridden the intended four CTest jobs with one job,
turning development feedback into a serial command.

This candidate must replace that ambiguous aggregate with one repository-only
unit route and one explicit external-input integration route. It is a
test/build workflow repair, not a reduction of functional coverage.

## Dependency And Scope

This task follows the active Core event-deadline scheduler convergence task.
It changes only test registration, gate runners, build presets, the `test/`
source tree and the developer-toolchain documentation required to make those
routes usable. It
does not alter Core, VM, device, firmware, profile or product behaviour, and
it does not consume or replace the active task's test-repair scope.

The frozen initial universe is the existing GCC Debug registry: 302 CTests,
300 canonical smoke targets and two auxiliary checks. S1 records each entry's
input class:

- ordinary hermetic component smoke;
- media or product scenario smoke; or
- auxiliary generated-result/governance check.

An entry may move only between the two routes when S1 proves its actual input
contract. A slow or inconvenient test is never a reason to remove or relabel
it.

## Ownership And Design Constraint

CTest remains the sole executor and CMake remains the sole membership and
target-dependency owner. The bounded PowerShell aggregate remains responsible
only for an aggregate deadline and child cleanup. Do not add a second manifest,
parallel custom launcher, duplicated target list, per-developer script or
generic test framework.

The completed layout has one test-code root: repository-only modules are under
`test/` and follow the relevant `src/` ownership shape (`test/core/...`,
`test/vm/...` and the narrow shared `test/support/...` helper area).
External-asset scenarios are only under `test/integration/`. The task migrates
obsolete `tests/` locations instead of retaining both trees or adding a
forwarding CMake layer.

- **Focused regression** is selected anew for each S from the code, ownership
  boundary and failure mechanism changed by that S.  It is a command shape,
  not a fixed membership set, manifest or permanent fourth gate. It appears
  only in the active S packet, may include relevant unit or integration checks,
  and disappears at S closure; it supplements, never replaces, the unit gate.

- **Unit gate** contains every hermetic test that uses only repository-tracked
  source, generated build output and repository fixtures.  It accepts no
  owner-managed ROM, disk image, firmware path, network input or machine-local
  asset.  Every admitted S in this T runs the complete unit gate, not merely
  its focused regression.
- **Integration gate** contains the opt-in product/media scenarios whose
  contract requires owner-managed external ROM and disk-image inputs.  It is
  never silently included in the unit gate and it must never retain those
  paths, bytes, hashes or traces in repository evidence.
Complete verification is exactly the unit gate followed by the applicable
integration gate; it is not a third test set. For this T, the external-ROM and
disk-image profile probes are required closure evidence: they validate that the
new classification mechanics do not alter the product test boundary.

Parallelism is the normal policy for classes proven isolated.  A test with a
real shared mutable resource receives the narrowest CTest ownership mechanism
(owned working directory, resource lock or serial mark); it must not force the
whole suite to one job.  The configured job policy must be visible in the
supported preset/runner and must not silently fall back to a stale local cache
value.

## S Decomposition

1. **Documentation governance.** Adopt the transient-focused, per-S unit and
   per-T integration closure rule and the sole `test/` source-root contract,
   without a new fixed focused list or test framework. Establish the existing
   repository-only baseline command and run it before closing S1.
2. **Complete test audit and classification.** Audit every registered test and
   test-only target. Record whether it is retained, deleted as duplicate or
   obsolete, or requires systematic refactoring; classify its inputs as
   repository-only unit or external-ROM/disk integration, then identify shared
   resources and accidental build-all dependencies. Run the complete unit
   suite before closing S2.
3. **Component-owned test architecture.** From the audited `src` ownership
   graph, define the durable unit-test modules: CPU profile/instruction
   function-and-timing, controller function-and-timing, bus/transaction/memory,
   Core lifecycle, VM session/profile/CLI and Core--VM large-unit composition.
   Define the separate external integration corpus: BIOS/option ROMs, selected
   firmware/data, DOS boot media and HDD Windows 3.1 installer checkpoints.
   No test implementation batch starts before this plan is accepted. Run the
   complete unit suite before closing S3.
4. **Canonical route and test-module migration.** Implement the accepted audit
   dispositions and `test/` component modules through one CMake registration
   source; remove only proven duplicates/obsolete tests and obsolete aggregate
   runner/list logic. Run the complete unit suite before closing every S.
5. **Parallel isolation and closure.** Prove a bounded parallel job count on
   clean and dirty incremental builds, applying the narrowest isolation only to
   demonstrated shared resources. Run the complete unit suite, then the
   owner-managed external-ROM/disk integration suite, before T closure.

## Exit Criteria

- Every baseline check remains registered in exactly one of the unit or
  integration routes, except for a proven registration error.
- All retained test code is under `test/`; repository-only test modules mirror
  their `src` owner and external-asset scenarios are under `test/integration/`.
- A developer can run one affected smoke without building the other 299
  smoke executables.
- The fast route excludes only the explicitly catalogued scenario class and
  remains deterministic under its documented parallelism.
- The complete route runs with its verified, visible parallel policy; no
  global serial setting remains merely to mask a fixture collision.
- Every isolated exception names its resource and uses one owner-local CTest
  mechanism.
- Every S has passed the complete repository-only unit gate; focused checks
  supplement but never replace it.
- Clean and incremental timing evidence, complete unit tests, specialized
  checks and the owner-managed external-ROM/disk integration gate pass. The
  integration corpus covers BIOS/option ROMs, selected firmware/data, DOS boot
  disks and the HDD Windows 3.1 installer checkpoint without retaining assets.
  Documentation governance and actual-change review confirm that no production
  behaviour or test coverage was silently removed.
