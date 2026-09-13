# Project Status

## Current Work

## M5 T530 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved T530 in this thread on 2026-09-13: exact SoftPC Lib import, Common-first adaptation, enumerated necessary non-Common adaptation, build/test/push then owner test; one-session coordinator/executor mode is approved. |
| Objective | Replace NXVM `src/lib` byte-for-byte with SoftPC `3cabea6a6a0ce931a5480f3549599ebb74f3681d` (`shared-t55-s13-p1`), adapt all consumers through public contracts, and deliver x64/x86 0530 artifacts for owner testing. |
| Non-goals | No SoftPC app/MVDM/media/firmware/test import; no Lib fork, compatibility facade, duplicate host path or unrelated Core/profile semantic change. |
| Reference Baseline | NXVM `cc5fd4a7`; SoftPC `3cabea6a6a0ce931a5480f3549599ebb74f3681d`, clean worktree, verified manifest and component DAG. |
| Candidate Proposal | [M5 canonical SoftPC Lib refresh](../proposals/m5-canonical-softpc-lib-refresh.md). |
| Files And ABI Surface | Entire `src/lib/`; its CMake/manifest/verification scripts; all `src/common`, NXVM source/test/CMake public-Lib consumers; provenance, evidence and current task records. |
| Applicable Rules | Task Reading Set; source policy; System Architecture; Source Layout; Architecture, Coding, Execution and Documentation Rules. |
| Verification | Exact tree hash comparison and manifest/DAG; private-header and duplicate-route sweeps; fresh x64/x86 build; complete repository-only unit suite; applicable Lib/Common/static/documentation gates; external integration suite; stripped 0530 artifact SHA/PE checks. |
| Expected Markers | 93 imported files match SoftPC exactly; no external private-Lib include; Common consumes only `*_interface.h`; every non-Common adaptation is in evidence with contract cause. |
| Asset Needs | No assets imported.  Existing owner-managed integration assets only; 0530 artifacts in ignored `build/output` and `assets/sessions`. |
| Reporting Requirements | Report import provenance, complete consumer ledger, all non-Common adaptations, verification, source/test line accounting and pushed P commit; await owner artifact testing before T closure. |
| Stop Conditions | Stop for source/license conflict, an unreviewed public ABI semantic gap, direct platform leakage, duplicate production route, or changed integration failure class. |
| Exit Criteria | Exact corpus plus all consumer cutover is committed/pushed; required verification and 0530 artifacts exist; T remains open awaiting owner test response. |
| Original Owner Request | Import SoftPC Lib byte-identically, update Common, report any non-Common adaptation caused by Lib, then build, test, push and await owner testing before closure. |
| Similar-Issue Sweep | Scan all tracked production/test/CMake consumers of old Lib public/private headers and direct platform calls; classify every hit as migrated, deleted or not applicable in evidence. |

## Current Technical Baseline

- `vm-0-5-0530` is the current target. Its stripped Release artifacts will be
  `nxvm_0_5_0530_x64.exe` and `nxvm_0_5_0530_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
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
