# Project Status

## Current Work

## M5 T533 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved independent T533 execution on 2026-09-21. S4 implementation `7874b683` is reviewed as the fixed-profile/INI boundary; this admitted S5 completes its required product convergence and task closure. |
| Objective | Close the fixed-product conversion: prove all four canonical builds, repair the Model 40 boot-chain failure or transfer it truthfully, remove any remaining duplicate deployment/configuration path, and produce the required optimized dual-architecture artifacts. |
| Non-goals | Do not add runtime machine/CPU/firmware selection, embed or commit BYOB assets, alter BIOS boot order through INI, retire any retained machine, create PC110 placeholders, or retain a YAML compatibility route. |
| Reference Baseline | T533 S4 implementation `7874b683`; [S5 convergence ledger](../etc/evidence/t533-s5-product-convergence-ledger.md); [M5 fixed-machine products](../proposals/m5-fixed-machine-products.md), S5. |
| Candidate Proposal | [M5 fixed-machine products](../proposals/m5-fixed-machine-products.md), S5 duplicate cleanup and full closure. |
| Files And ABI Surface | Fixed CMake product/build/deployment boundary, Profile-to-Machine composition, App INI entry, integration fixtures and any single shared Core device owner exposed by the Model 40 failure. |
| Applicable Rules | [Architecture](../design/ARCHITECTURE.md): Profile is the sole board/firmware owner and App owns runtime INI only. [Coding](../design/CODING.md): one parser, one construction route, repository-only unit tests and external assets only in integration. [Source policy](../etc/operations/policy/source-policy.md): BYOB firmware/media remain external and manifest-validated. [Execution](../rules/EXECUTION.md): complete unit per S; full integration and dual artifacts at task closure. |
| Verification | Execute the ledger: configure each product against its manifest; run all repository-only units; run every product's registered integrations; replay all affected profiles after a shared repair; verify no YAML/catalog/deployment duplication; create and inspect all required optimized stripped x64/x86 artifacts with matching INI companions. |
| Expected Markers | Four build-selected products with no runtime alternate; one unchanged adjacent NXVM.ini per artifact; no configuration overwrite between products; all qualified product rows reach their declared terminal; no duplicate construction/reset/media/display path. |
| Asset Needs | Local user-provided `nxvm-assets` only. Never import or commit ROM, CMOS, font or guest-media bytes. |
| Reporting Requirements | Record every ledger disposition, actual code-size/path result, Model 40 root cause and complete affected-profile replay, all artifact paths/hashes, and every lawful transfer. Do not claim a protected-asset import or an unrun integration result. |
| Stop Conditions | Stop for a missing legal BYOB manifest contract, no lawful INI mapping, a required firmware semantic unavailable from its profile, a failure that cannot be localized to an owner, or any required artifact that would overwrite another product's configuration. |
| Exit Criteria | Every S5 ledger member is accepted with evidence or explicitly transferred by owner-approved normal process; all four product builds use one INI path without YAML/catalog; complete unit/integration and dual-architecture artifacts pass; task closure audit proves the original fixed-profile request. |
| Original Owner Request | Retain XT, AT, DeskPro 386 Model 40 and default PC/AT, but stop exposing a combinatorial collection of same-machine CPU/floppy sessions: each retained machine gets one most suitable canonical profile. Keep CPU implementations and tests. Build selects a fixed profile; NXVM.ini owns only runtime options. |
| Similar-Issue Sweep | Sweep all production App/CMake/session construction and all integration support/tests for runtime profile, CPU, firmware, YAML/catalog and copied-media paths. Each hit is migrated to the generated fixed binding/INI route, retained only as a generic Core test, or explicitly transferred; no product compatibility loader remains. |

## Current Technical Baseline

- `vm-0-5-0533` is the current target. Its T533 fixed-default product artifact
  names are `nxvm_default_0_5_0533_x64.exe` and
  `nxvm_default_0_5_0533_x86.exe`; neither is claimed produced until the
  required dual-architecture Release closure succeeds.
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
| T533 S3 | Accepted at `2a589dc7`: Profiles now construct the one frozen board/firmware plan for XT, 5170, Model 40 and default PC/AT; Machine consumes it without board or firmware branches. Full repository-only unit, documentation and dependency gates pass. [Evidence](../etc/evidence/t533-s3-profile-machine-plan.md). |
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
