# Project Status

## Current Work

## M5 T533 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved independent execution of T533 on 2026-09-21. S1 acceptance `7fdb25ab` establishes the complete relocation ledger and admits the next bounded S within the approved proposal. |
| Objective | Atomically relocate reusable Core implementation and its mirrored repository-only tests from `core/core` to `core/devices`, and profile implementation/tests from singular `core/profile` to plural `core/profiles`, without any hardware, runtime, ABI or shared-corpus behavior change. |
| Non-goals | Do not move board construction from Machine, alter CMake target topology beyond renamed paths, change YAML/INI behavior, profile selection, asset loading, test assertions, firmware bytes, Common/Lib/x86, integration inputs or developer artifacts. |
| Reference Baseline | T533 S1 ledger at `14572230`, accepted at `7fdb25ab`; runnable baseline is `vm-0-5-0532`. |
| Candidate Proposal | [Fixed-machine products and architecture consolidation](../proposals/m5-fixed-machine-products.md), S2; [baseline ledger](../etc/evidence/t533-s1-fixed-machine-baseline-ledger.md). |
| Files And ABI Surface | Tracked source/test path moves below `src/core/core`, `src/core/profile`, `test/core/core`, `test/core/profile`; direct include/CMake/static-gate/doc links that spell the former paths. Public C interfaces, target names and runtime behavior remain unchanged. |
| Applicable Rules | Task Reading Set; Execution structural-relocation and full-unit closure requirements; Architecture owner/dependency direction; Coding minimal-diff/no-duplicate-path requirements; Documentation link/index requirements. Source policy is not triggered because no assets or third-party content move. |
| Verification | `git diff --check`; no former source/test owner roots or stale build references outside recorded historical evidence; configure/build from a fresh x64 tree; full repository-only unit suite; documentation governance. Use focused path/include/CMake sweeps only transiently. |
| Expected Markers | `src/core/devices` and `src/core/profiles` are the sole live owner paths; matching `test/core/devices` and `test/core/profiles` mirror them; CMake and static checks name only the new roots; every retained CPU/profile test remains registered. |
| Asset Needs | None. No `nxvm-assets` read, write, import, hash or manifest action is permitted in S2. |
| Reporting Requirements | Report moved owner sets, explicit non-moved Machine/App/profile-composition routes reserved for S3/S4, test count and gates, reviewable P1 and governance P2 commits. |
| Stop Conditions | Stop if a move would change an exported include/ABI, require a compatibility alias, alter a runtime profile route, leave a live former-root hit without an approved disposition, or expose an undisposed owner outside the frozen S1 ledger. |
| Exit Criteria | Former live roots are absent; the new source/test mirror is exact; all moved includes, CMake lists and static checks resolve from new names; full unit/documentation gates pass; actual-diff review confirms relocation only. |
| Original Owner Request | Retain all implemented machine products while restructuring to `core/devices`, `core/profiles` and `core/machine`, then later make each build fixed-profile with external BYOB assets and NXVM.ini runtime configuration. |
| Similar-Issue Sweep | Search all tracked production source, tests, CMake, scripts and active documentation for live `core/core`, `core/profile`, `test/core/core` and `test/core/profile` owner references; historical evidence is retained only when it describes its then-current baseline. |

## Current Technical Baseline

- `vm-0-5-0532` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0532_x64.exe` and `nxvm_0_5_0532_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, and `ui`. The separately selected `src/x86` corpus owns
  `xasm32` and the x86 Debug CLI; Common has no x86 dependency.
- S21 P1 `b35357ca` and P2 `e0d3b946` established the initial NXVM diagnosis.
  S22 replaces their private Common/Lib hunks with SoftPC's canonical corrected
  corpus and retains only the NXVM VADP adapter concern where still needed.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T533 S1 | Accepted at `14572230`: finite four-machine/CPU/configuration/build/test ledger, with every live route assigned to S2--S5; 336/336 repository-only unit tests and documentation governance passed. [Evidence](../etc/evidence/t533-s1-fixed-machine-baseline-ledger.md). |
| T532 | Closed at `0b8918ee`: S1 froze the full map; S2 implementation `66bba6f3` relocated App/Core and repository-only test owners, and retired VDM; S3 proves no former root remains, 336/336 unit and 42/42 integration pass, and stripped 0532 x64/x86 artifacts are byte-identical in both required locations. [History](../history/M5-T532-nxvm-single-product-layout.md) and [evidence](../etc/evidence/t532-s3-single-product-layout-closure.md). |
| T531 | Closed at owner direction on 2026-09-20 after S28 implementation `20c1e71b`: the final canonical SoftPC `2b17749a` six-tree Lib/Common/x86 source-and-test refresh is exact; 338/338 unit, specialized/documentation gates and stripped 0531 x64/x86 artifacts pass. The full external integration remains 41/42: `integration.vm-dos-keyboard-smoke` stays explicitly transferred to [TODO(High)](TODO.md), not relabelled green. [History](../history/M5-T531-shared-common-product-convergence.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |
| T528 | Closed under the owner's 2026-09-12 exception after `9d892446`: all VM-machine duplicate owners are removed, 299 unit cases, specialized gates, documentation governance and stripped x64/x86 artifacts pass. The known 39/42 external-integration result is transferred unchanged to the external-ROM Model-40/IBM-5170 boot-chain TODO; it is not represented as green. [History](../history/M5-T528-vm-machine-owner-cleanup.md) and [evidence](../etc/evidence/t528-s7-task-closure-attempt.md). |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 Td S170 P1:** records one external BYOB firmware route for each future
  compiled profile: a CMake-provided local `NXVM_PROFILE_ASSETS_ROOT` is
  validated against the Profile manifest and emitted only as an ignored local
  build binding. Firmware, CMOS and fonts stay outside both repository and EXE;
  `NXVM.ini` only configures runtime media and presentation. Documentation gate
  and actual-diff review pass; no source, build, test, asset or artifact changed.

- **M5 Td S169 P1:** revises current goals, architecture, layout, proposals,
  queue and future artifact naming to preserve every implemented machine and
  use plural core/profiles plus mirrored tests. Standard-board selection and
  model retirement are removed from the structural proposal; guest qualification
  now covers the retained machines. Historical scope notices preserve earlier
  facts without authorizing deletion. Documentation/architecture governance
  guided the authority split; full documentation gate and actual-diff review
  pass. No source, test, asset, configuration or artifact changes.

- **M5 Td S168 P1:** supersedes the two-product retirement scope with retained
  XT/AT, candidate Standard 386DX (including DeskPro) and later PC110. The
  queue-head proposal defines five S batches for Devices relocation, profile-
  owned composition, Common adaptation and fixed-build/INI cutover. Read-only
  SoftPC configuration inspection confirms no startup/boot-mode key; NXVM.ini
  likewise excludes both. Documentation and architecture governance keep one
  authority per concern; archived XT/AT evidence has explicit current receivers.
  Full documentation gate and actual-diff review pass. Only README/docs changed;
  source, tests, assets, build configuration and 0532 artifacts remain untouched.

- **M5 Td S167 P1 (scope superseded by S168):** governed two fixed-machine products within one multi-machine
  architecture and preserves all CPU families. The queue-head
  [consolidation proposal](../proposals/m5-fixed-machine-products.md) records
  inspected simplifications and five S batches; 43 old proposals and all debt
  have explicit receivers/retirements. [Research](../etc/research/fixed-machine-selection.md)
  records Standard firmware gaps, PC110 prerequisites and external manuals.
  Documentation gate/self-tests and actual-diff review pass. No production,
  CMake, test or YAML changes; current artifacts and the closed T532 remain unchanged.

- **M5 Td S166 P1:** queued two owner-approved, unnumbered candidates in
  dependency order: VM-machine owner cleanup, then Common-machine executor
  completion. Both use the audited NXVM/SoftPC two-consumer criterion; neither
  allocates or admits a numeric implementation task.

- **M5 T527 S9 P2:** records independent corpus verification, complete test
  closure, artifact identity, proposal retention, and task closure after
  implementation `2c596f2c`.
