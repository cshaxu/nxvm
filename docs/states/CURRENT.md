# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | New T535 S1 |
| Admission And Approval | Owner approved on 2026-09-22: admit a new T to import SoftPC's latest six shared source/test components. Scope is the committed SoftPC revision only; ordinary commits and pushes are permanently approved by the owner. |
| Objective | Freeze and audit the committed SoftPC six-tree corpus before copying it, recording exact provenance, file/hash delta, public/build impact and NXVM-only adaptation boundary. |
| Non-goals | Do not copy uncommitted SoftPC media; do not import SoftPC App/MVDM code, firmware, media or binaries; do not enable audio, alter guest/device behavior, or edit a shared-tree file for NXVM-specific behavior. |
| Reference Baseline | NXVM `f5170a5d`, current `vm-0-5-0534`; SoftPC committed `1c5a47146dd4fd87b09423b7a7b960becb50cd67` (`1c5a4714`). Its dirty media paths are excluded. |
| Candidate Proposal | [Canonical SoftPC six-component refresh](../history/M5-T535-canonical-softpc-six-component-refresh-proposal.md). S1 consumes the frozen six-tree inventory batch. |
| Files And ABI Surface | Audit only: `src/{lib,common,x86}`, `test/{lib,common,x86}`, their manifests/CMake files and NXVM caller/build references. Potential added public Lib ABI is `lib_audio_stream_*`; no implementation source changes in S1. |
| Applicable Rules | `AGENTS.md`; Task Reading Set; Execution, Architecture, Coding and Document rules; source policy. Evidence will record project-owned MIT provenance, exact commit and no independent notice. |
| Verification | Relative-path/content-hash comparison across all six trees; inspect upstream CMake/manifests/public interfaces; static NXVM caller/build search; documentation governance and `git diff --check`. |
| Expected Markers | Evidence names upstream commit, excludes dirty media, classifies every delta and states whether an NXVM adapter change is necessary; x86 remains exact. |
| Asset Needs | None. SoftPC source is read-only comparison material; no firmware, media, ROM or external runtime asset is read or copied. |
| Reporting Requirements | Report the frozen source revision, complete delta classification, blockers and proposed S2 boundary. Report no runtime behavior claim from S1. |
| Stop Conditions | Stop and seek direction for an independent notice/license, an uncommitted source dependency, protected asset, required product behavior change, or a shared-tree local fork requirement. |
| Exit Criteria | Durable [S1 audit](../etc/evidence/t535-s1-softpc-six-component-audit.md), approved import plan with exact corpus boundary, passing documentation/diff checks and a clean reviewable P commit. |
| Original Owner Request | “准入新的t任务 从softpc项目引入最新的六组件” |
| Similar-Issue Sweep | Search all six source/test roots plus build manifests for stale/cross-product references and every NXVM source/build consumer for a shared-tree private include or locally diverged public contract. |

## Current Technical Baseline

- `vm-0-5-0534` is the current target. T534 produced eight stripped, optimized
  x64/x86 artifacts for its four fixed products; each product has only one
  executable location, `assets/binary/<profile>/`, with its adjacent generated
  INI. The preset-selected default pair is `nxvm_default_0_5_0534_x64.exe` and
  `nxvm_default_0_5_0534_x86.exe`. Paths, architectures, hashes and gate results are in the
  [T534 artifact record](../etc/evidence/t534-s50-artifact-verification.md).
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, and `ui`. The separately selected `src/x86` corpus owns
  `xasm32` and the x86 Debug CLI; Common has no x86 dependency.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T534 | Closed at `f5170a5d`: 53 bounded repairs exhaust the App/Core convergence ledger without a second production owner or unclassified finding. Current 0534 x64/x86 artifacts for default, 5170, XT and Model 40 are verified in their sole profile directories. 333/333 repository-only unit, default parallel 20/20 integration, 5170 3/3, XT 1/1 and Model 40 3/3 integration pass. [History](../history/M5-T534-app-core-code-quality-remediation.md), [closure ledger](../etc/evidence/t534-app-core-code-quality-ledger.md) and [artifact record](../etc/evidence/t534-s50-artifact-verification.md). |
| T533 | Closed by `99de6d11` and deployment correction `6d5a4828`: fixed XT, 5170, Model 40 and default PC/AT products each use one CMake-selected Profile, one external BYOB asset route and one adjacent NXVM.ini route. Common owns composed host-input ingress; HDD-only validation mutates the production media owner then resets. 335/335 repository-only unit and 20/20 optimized integration pass both serially and in parallel; dual stripped x64/x86 artifacts are verified only in `assets/binary/<profile>/`. [History](../history/M5-T533-fixed-machine-products.md) and [evidence](../etc/evidence/t533-s5-product-convergence-ledger.md). |
| T532 | Closed at `0b8918ee`: S1 froze the full map; S2 implementation `66bba6f3` relocated App/Core and repository-only test owners, and retired VDM; S3 proves no former root remains, 336/336 unit and 42/42 integration pass, and stripped 0532 x64/x86 artifacts are byte-identical in both required locations. [History](../history/M5-T532-nxvm-single-product-layout.md) and [evidence](../etc/evidence/t532-s3-single-product-layout-closure.md). |
| T531 | Closed at owner direction on 2026-09-20 after S28 implementation `20c1e71b`: the final canonical SoftPC `2b17749a` six-tree Lib/Common/x86 source-and-test refresh is exact; 338/338 unit, specialized/documentation gates and stripped 0531 x64/x86 artifacts pass. The full external integration remains 41/42: `integration.vm-dos-keyboard-smoke` stays explicitly transferred to [TODO(High)](TODO.md), not relabelled green. [History](../history/M5-T531-shared-common-product-convergence.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |
| T528 | Closed under the owner's 2026-09-12 exception after `9d892446`: all VM-machine duplicate owners are removed, 299 unit cases, specialized gates, documentation governance and stripped x64/x86 artifacts pass. The known 39/42 external-integration result is transferred unchanged to the external-ROM Model-40/IBM-5170 boot-chain TODO; it is not represented as green. [History](../history/M5-T528-vm-machine-owner-cleanup.md) and [evidence](../etc/evidence/t528-s7-task-closure-attempt.md). |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |

## Recent Governance

- **M5 Td S171 P1 `3631f80f`, P2 governance closure:** audited the entire live
  architecture/operation/proposal context against completed T533. It corrects
  the former YAML-as-current, target-as-current and old-deployment language;
  labels retired supporting records without rewriting their facts; and gives
  every Queue proposal the fixed Product/BYOB/NXVM.ini/sole-artifact baseline.
  The draft T534 packet was withdrawn before CPU work. Documentation governance
  and actual-diff review pass; no code, build, test, asset or artifact changed.

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
