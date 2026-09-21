# Project Status

## Current Work

## M5 Td S171 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance |
| Admission And Approval | The owner approved a thorough documentation audit and governance pass as Td on 2026-09-21, before resuming Queue implementation. The uncommitted T534 S1 preparation is withdrawn without CPU behavior or evidence work. |
| Objective | Reconcile every live design, rule, Queue proposal, current baseline and supporting-current document with T533's completed fixed-product/INI/sole-artifact architecture, while preserving historical records as historical. |
| Non-goals | Do not change production code, CMake, tests, assets, CPU/controller behavior, historical task facts, Queue order or protected external material. Do not rewrite closed history merely because terminology later changed. |
| Reference Baseline | [T533 history](../history/M5-T533-fixed-machine-products.md), [S5 convergence ledger](../etc/evidence/t533-s5-product-convergence-ledger.md), [System Architecture](../design/ARCHITECTURE.md), [Source Layout](../design/CODING.md), [Queue](QUEUE.md). |
| Candidate Proposal | No implementation proposal: this is owner-approved standalone governance. Its evidence records live-document findings and dispositions. |
| Files And ABI Surface | Documentation only: principal design, current state, Queue-linked proposals, source/build policy, supporting-document index and explicit historical labels. No source, ABI, runtime route or generated artifact changes. |
| Applicable Rules | [Documentation](../rules/DOCUMENT.md): one authority per topic and preserve historical terminology. [Execution](../rules/EXECUTION.md): Td packet, documentation governance and actual-diff review. [Architecture](../rules/ARCHITECTURE.md): one Profile/INI/asset route. [Source policy](../etc/operations/policy/source-policy.md): BYOB assets remain external. |
| Verification | Audit all non-history Markdown under `docs/design`, `docs/rules`, `docs/states`, `docs/proposals`, `docs/etc/operations`, `docs/etc/architecture`, `docs/etc/research` and the `etc` index for retired YAML/runtime-selection/deployment/layout claims; record every live hit and disposition; run documentation governance and `git diff --check`. |
| Expected Markers | One post-T533 description of fixed products, external BYOB firmware, adjacent INI, sole `assets/binary/<profile>/` deployment, retained machine scope and historical-document status; all Queue proposals start from that baseline. |
| Asset Needs | None. This Td neither reads nor modifies firmware, ROM, CMOS, media, fonts, manuals or external repositories. |
| Reporting Requirements | Record scope/search, every live finding, changed authority and rationale, historical exclusions, Queue/proposal update proof, verification, and explicit confirmation that the withdrawn T534 packet produced no implementation work. |
| Stop Conditions | Stop if a proposed correction would alter a closed historical fact, imply an unimplemented PC110/486 capability, require a source/build change, or reveal a competing live architecture that cannot be resolved by the principal authorities. |
| Exit Criteria | All live architecture/operational/proposal texts agree with the T533 baseline, retired supporting designs are unambiguously historical, the Queue has current context without a premature T534 admission, and documentation governance passes. |
| Original Owner Request | Treat T533 as whole-system simplification and cleanup; ensure all later Queue proposals and their context are current before more implementation work begins. |
| Similar-Issue Sweep | Search all live authorities and live proposal/supporting-context locations for the retired architecture vocabulary and paths; classify each as corrected, intentionally historical or not applicable in the evidence record. |

## Current Technical Baseline

- `vm-0-5-0533` is the current target. T533 produced eight stripped, optimized
  x64/x86 artifacts for its four fixed products; each product has only one
  executable location, `assets/binary/<profile>/`, with its adjacent generated
  INI. The preset-selected default pair is `nxvm_default_0_5_0533_x64.exe` and
  `nxvm_default_0_5_0533_x86.exe`. Paths, hashes and terminal dispositions are recorded in the
  [T533 convergence ledger](../etc/evidence/t533-s5-product-convergence-ledger.md).
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
| T533 | Closed by `99de6d11` and deployment correction `6d5a4828`: fixed XT, 5170, Model 40 and default PC/AT products each use one CMake-selected Profile, one external BYOB asset route and one adjacent NXVM.ini route. Common owns composed host-input ingress; HDD-only validation mutates the production media owner then resets. 335/335 repository-only unit and 20/20 optimized integration pass both serially and in parallel; dual stripped x64/x86 artifacts are verified only in `assets/binary/<profile>/`. [History](../history/M5-T533-fixed-machine-products.md) and [evidence](../etc/evidence/t533-s5-product-convergence-ledger.md). |
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
  [T533 consolidation record](../history/M5-T533-fixed-machine-products.md) records
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
