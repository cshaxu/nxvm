# Project Status

## Current Work

## M5 T531 S26 Accepted

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; accepted on 2026-09-18. |
| Admission And Approval | Owner approved this continuation S on 2026-09-18: import and connect the latest SoftPC shared corpus; YAML must define floppy and fixed-disk access modes. Owner accepted S closure while retaining T531 open. |
| Objective | Replace NXVM's four shared trees byte-for-byte with SoftPC `95c467a73e448ca7953f8d1b776843b26316651b`; directly adopt its modeful Common removable-media contract, and make YAML media entries resolve one immutable Storage mode for each FDD/HDD. |
| Non-goals | No SoftPC write, no NXVM-specific branch in Lib/Common, no Core/controller semantics change, no second mode enum or compatibility mode path, no user-output YAML edit, and no persistence/commit semantics change. Omitted YAML mode remains the existing overlay behavior. |
| Reference Baseline | NXVM `21b33e67`; clean committed SoftPC `95c467a73e448ca7953f8d1b776843b26316651b`; S25 is the immediately preceding canonical corpus import. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), S26. |
| Files And ABI Surface | Complete `src/lib`, `src/common`, `test/lib`, and `test/common` imports; NXVM YAML request/config parser, App-to-VM-machine composition and VM-machine FDD/HDD opening adaptation; task evidence and task history. |
| Applicable Rules | [Execution](../rules/EXECUTION.md), [Architecture](../rules/ARCHITECTURE.md), [Coding](../rules/CODING.md), [Documentation](../rules/DOCUMENT.md), [System Architecture](../design/ARCHITECTURE.md), [Source Layout](../design/CODING.md), [Product UX](../design/UI.md), and the source policy. Common remains product-neutral; App owns YAML parsing, VM-machine owns media opening, and platform APIs remain inside Lib. |
| Verification | Before/after four-tree corpus diff; Lib/Common manifests and corpus gates; YAML parsing/default/invalid-mode and all FDD/HDD mode unit coverage; retired-path scans; both x64/x86 Release builds; complete repository-only unit suite; external integration suite; actual-diff review and documentation governance. |
| Expected Markers | SoftPC revision `95c467a7`; exact corpus equality for all four imported trees; `media.floppy[]`/`media.fixed_disk[]` parse one `direct|readonly|overlay` mode per path with overlay default; Common transfers the selected removable mode once to VM-machine; no platform API leakage outside Lib. |
| Asset Needs | None; repository-only tests only. External integration uses its established external assets without modifying them. |
| Reporting Requirements | Report the frozen source revision, exact YAML/VM-machine adaptation, corpus identity, test/build outcomes, line delta, and every remaining non-green integration row without pre-existing attribution. |
| Stop Conditions | Stop for an incompatible public contract that needs a new product-policy seam, a required Core semantic change, a source-license conflict, or a regression that cannot be repaired at the NXVM adapter boundary. |
| Acceptance | Four-tree equality, mode-path coverage, 336/336 unit, both Release artifacts, actual-diff review and documentation governance pass. Full integration recorded 41/42 with the Model 40 row passing on immediate isolated replay; evidence records the non-deterministic observation. T531 remains open until its whole-task two-product execution gate is separately satisfied. |
| Original Owner Request | Admit a new S to import and connect the latest SoftPC shared components. YAML must define analogous access-mode configuration for floppy and hard-disk media. |
| Similar-Issue Sweep | Treat every FDD/HDD construction and removable-media call site as one mode-propagation family; inspect parser, request, config, Common driver, startup and replacement paths so no image silently chooses a second access policy. |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.
- S21 P1 `b35357ca` and P2 `e0d3b946` established the initial NXVM diagnosis.
  S22 replaces their private Common/Lib hunks with SoftPC's canonical corrected
  corpus and retains only the NXVM VADP adapter concern where still needed.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S26 | Accepted at `0840a34b`: imported exact SoftPC `95c467a7` Lib/Common source and tests; added one per-media YAML Storage mode (`direct`, `readonly`, `overlay`, default overlay) through App, Common and VM-machine with no parallel opener. [Evidence](../etc/evidence/t531-s26-modeful-media-import.md). |
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
