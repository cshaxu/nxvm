# Project Status

## Current Work

| Task | Compact result |
| --- | --- |
| T531 S13 | Canonical SoftPC `54845ac` Lib/Common/test-corpus refresh accepted. P1 `4e7b4a26` imports the exact four trees and direct NXVM adaptations; P2 records equality, gates and dual-architecture acceptance. |
| T531 S14 | Accepted raw-Console prompt correction at P1 `9e7b4eb2`; Common ownership now suppresses prompt publication during guest input. [Evidence](../etc/evidence/t531-s14-raw-console-monitor-ownership.md). |
| T531 S15 | Accepted P1 `5c648c35`: FDD/HDD Core-media adaptation now resides at `src/vm/machine/media` with the one retained `vm-media` target; direct include/CMake paths and obsolete DAG exceptions are removed. [Evidence](../etc/evidence/t531-s15-vm-machine-media-layout.md). |

## M5 T531 S16 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T531 S16. |
| Admission And Approval | Owner approved on 2026-09-14: merge `src/vm/product` into `src/vm/app`, organize it by configuration, commands and composition, remove command-layer runtime-lifecycle imports, import SoftPC's committed four shared trees, build, test, commit and push. |
| Objective | Retain one NXVM App root that owns product configuration, command policy, recorder and composition; retain VM-machine creation and Common execution/UI ownership at their existing owners. |
| Non-goals | No Common behavior redesign, Core/device semantic change, new queue, public machine-state escape, external asset change, SoftPC write, or task closure. |
| Reference Baseline | Accepted T531 S15 P2 `c9f8576b`; SoftPC committed baseline `00d4461d0754ae2a7bd8684aa06df6541f83fef9`. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), S16. |
| Files And ABI Surface | Exact import of SoftPC `src/lib`, `src/common`, `test/lib`, `test/common`; relocate `vm/product` into `vm/app`, update CMake/tests/static gates and documentation. Public NXVM product symbols may be renamed to `vm_app_*`; retain one direct App-to-public-VM-machine route. |
| Applicable Rules | [Architecture](../rules/ARCHITECTURE.md): one owner and neutral dependency direction. [Coding](../rules/CODING.md): delete retired paths and forwarding layers. [Source policy](../etc/operations/policy/source-policy.md): import only the named committed project-owned corpus. [Execution](../rules/EXECUTION.md): structural relocation, complete unit suite, artifacts and actual-diff review. |
| Verification | Prove exact four-tree equality to the named SoftPC commit; build affected App/VM targets; run focused App command/catalogue/recorder tests; run full repository-only unit suite; prove zero live `vm/product`, `vm-product`, and command-to-runtime-lifecycle path; run documentation/dependency gates; build stripped x64/x86 0531 artifacts in both required locations. |
| Expected Markers | `src/vm/app` is the sole NXVM product configuration/command/composition root; `vm-app` is its sole CMake target; VM-machine remains the only machine builder; Common remains sole execution/UI owner. |
| Asset Needs | None. |
| Reporting Requirements | Report the frozen SoftPC revision and exact-tree comparison, retired names/paths, retained owner route, verification, artifact hashes, commit and push. T531 remains open. |
| Stop Conditions | Stop for a non-identical required SoftPC import that cannot be directly adapted, a needed Common public API change, a behavior regression, unknown source provenance, or a machine implementation dependency from App command policy. |
| Exit Criteria | One pushed implementation P and governance acceptance P after actual-diff review, exact import proof, complete unit success and dual artifacts; retain compact T531 progress rather than close it. |
| Original Owner Request | Consolidate `src/vm/app` and `src/vm/product` into App by responsibility, preserve the configuration/machine/Common boundaries, and import SoftPC's shared four-component corpus. |
| Similar-Issue Sweep | Search tracked CMake, source, tests and static gates for `vm/product`, `vm-product`, direct command-to-runtime lifecycle imports and duplicate request-to-machine construction. Retire every same-semantic route or record an explicit distinct owner. |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S11 | Accepted at `3a275380`: imported the exact 94-file SoftPC `987d82e` KVM Lib corpus, deleted all retired UI component paths, and directly cut Common, VM, CMake and tests to the canonical contracts. Lib boundaries, x64/x86 optimized 0531 builds, 299/299 unit and documentation governance pass. [Closure evidence](../etc/evidence/t531-s11-canonical-kvm-lib-refresh.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |
| T528 | Closed under the owner's 2026-09-12 exception after `9d892446`: all VM-machine duplicate owners are removed, 299 unit cases, specialized gates, documentation governance and stripped x64/x86 artifacts pass. The known 39/42 external-integration result is transferred unchanged to the external-ROM Model-40/IBM-5170 boot-chain TODO; it is not represented as green. [History](../history/M5-T528-vm-machine-owner-cleanup.md) and [evidence](../etc/evidence/t528-s7-task-closure-attempt.md). |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 Td S166 P1:** queued two owner-approved, unnumbered candidates in
  dependency order: VM-machine owner cleanup, then Common-machine executor
  completion. Both use the audited NXVM/SoftPC two-consumer criterion; neither
  allocates or admits a numeric implementation task.

- **M5 T527 S9 P2:** records independent corpus verification, complete test
  closure, artifact identity, proposal retention, and task closure after
  implementation `2c596f2c`.
