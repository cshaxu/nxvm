# T500 S2 Test Audit

## Registry Reconciliation

The configured Debug registry contains 302 CTests: 300 executable-backed
entries and two repository-generated ledger checks. The generated inventory
records all 345 test artifacts without owner-managed asset paths:
[inventory](t500-s2-test-inventory.json).

| Disposition | Count |
| --- | ---: |
| Registered repository-only unit route | 285 |
| Registered generated-result refactor decision | 2 |
| Registered external-integration route | 13 |
| Registered external shared-source refactor decision | 2 |
| Unregistered repository-only registration/removal decision | 34 |
| Unregistered external-integration registration decision | 6 |
| Unregistered platform-conditional registration decision | 3 |

The 302 registered entries reconcile exactly to 300 test executables plus the
two generated ledger checks. The 43 remaining test executables are not silently
dropped: each is present in the inventory with an explicit next disposition.
No deletion is accepted in S2; non-registration alone is not proof of dead
coverage.

## Input And Resource Findings

All 15 configured external-input CTests carry the existing `media` label; the
other 287 selected CTests take no owner-managed ROM, disk, firmware or HDD
argument. This makes the present non-media selection a conservative unit
baseline, not the final route name or test topology.

Two of those 287 ledger checks rewrite tracked CPU-result JSON when they run.
Their values pass, but the rewritten `form_id` values are a test-output side
effect, not a T500 CPU-semantic change. They therefore remain repository-only
but require systematic refactoring before they can be accepted as a clean unit
route: generated output must be compared or placed in the build tree without
rewriting tracked evidence on every run.

Seven host-thread lifecycle tests are `RUN_SERIAL`. Three console-profile tests
have individual build-tree workspaces; three generated ledger checks work from
the repository source directory. The remaining 296 use the standard build-tree
directory. S5 must retain only the demonstrated host-thread serialization and
owned workspaces, then prove whether any broader lock is still required.

Two external EGA DOS targets deliberately share one current source file. That
is a refactor decision, not a deletion: S3 must decide whether the source
contains two distinct behavior contracts that warrant a split or one redundant
target that can be removed with proof.

## Unregistered Candidates

The audit ran the first 15 available repository-only unregistered executables
without external input: ten exited successfully and five returned nonzero.
Three additional targets are platform-conditional and are unavailable in this
Windows configuration. Therefore S4 may not bulk-register these targets. S3
must give each one a concrete unit contract or a proven removal decision; the
six named BYOB/Windows/fdisk probes remain external-integration candidates.

## Acceptance Boundary

This is an inventory and classification result only. It changes no test source,
CMake registration, runtime behavior, external asset or test outcome. The
complete current repository-only baseline remains the 287-test non-media CTest
selection; it must pass again at S2 acceptance.
