# Project Status

## Current Work

### M6 T41 S13 SoftPC Raw-CRT Repair and Six-Corpus Reconciliation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner admitted M6 T41 S13 on 2026-09-24: NXVM remains the canonical six-component corpus; import SoftPC's verified Raw-CRT negative-test repair and audit every remaining six-corpus difference rather than reverting NXVM to an older SoftPC revision. |
| Objective | Make the x86 negative gate genuinely exercise forbidden raw CRT use, then produce a complete, byte-level NXVM-versus-SoftPC six-component difference ledger with each difference's owner and disposition. |
| Non-goals | Do not replace NXVM Shared code with SoftPC wholesale; do not delete NXVM-only process support, test aggregation, or x86 compatibility surfaces merely for parity; do not modify MyNES or product code; do not import SoftPC's uncommitted worktree. |
| Reference Baseline | NXVM Shared P1 `b94ea4ffe` is canonical. Clean SoftPC `2ef228f8` is the reproducible source for `test/x86/verify_negative.cmake`; its live worktree is clean and may be used only to compare current six-corpus differences, never to replace NXVM canonical behavior wholesale. |
| Candidate Proposal | [M5 T535 canonical SoftPC six-component refresh](../history/M5-T535-canonical-softpc-six-component-refresh-proposal.md), as constrained by this owner-approved reconciliation brief. |
| Files And ABI Surface | Shared P1 may change only `test/x86/verify_negative.cmake`, `test/x86/MANIFEST.sha256`, `src/lib/README.md` and `src/lib/MANIFEST.sha256`. NXVM P2 may change this packet, import provenance and S13 evidence only. Every other difference is inventory-only unless the owner separately admits its receiver. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `ARCHITECTURE.md`, `CODING.md`, `DOCUMENT.md`; NXVM `ARCHITECTURE.md`, `CODING.md`, and `etc/operations/policy/source-policy.md`. One valid negative probe must call raw `memcpy`, not the permitted Lib facade. The ledger distinguishes committed SoftPC evidence from uncommitted comparison-only observations. |
| Verification | Run the x86 negative gate before and after the repair; verify all six manifests; compare tracked file inventories and SHA-256 contents across `src/{lib,common,x86}` and `test/{lib,common,x86}` against clean SoftPC `2ef228f8`; classify every mismatch; run complete repository-only x64 and x86 unit suites and NXVM documentation governance. |
| Expected Markers | The Raw-CRT fixture is `memcpy(0,0,0)` and fails for `Raw CRT`; `lib_memory_copy` remains allowed; no unclassified six-corpus mismatch remains; NXVM-only differences remain intact unless separately admitted. |
| Asset Needs | None. |
| Reporting Requirements | Report the exact imported SoftPC commit/path, before/after negative-gate result, inventory/content mismatch counts for both SoftPC baselines, every difference disposition, manifests, x64/x86 checks, provenance/evidence, target-scoped commit and push. |
| Stop Conditions | Stop before any non-fixture import if it would discard an NXVM canonical capability, import an uncommitted SoftPC change, alter a public ABI, or require a product change. Record it as a ledger item instead. |
| Exit Criteria | The negative fixture is corrected and covered; all six components have complete committed-baseline and current-worktree comparison ledgers; each difference is classified with no silent overwrite; manifests, full dual-architecture units and documentation governance pass; provenance/evidence is complete; each target-specific P is pushed and the worktree is clean. |
| Original Owner Request | "追加s任务 引入softpc的正确修复 并核对两个地方的六组件是否一致"; clarified: "nxvm才是基准 只是要引入softpc的修复再对比所有diff". |
| Similar-Issue Sweep | Search all negative verification fixtures for permitted facade calls asserted as forbidden (`lib_`/`common_`/`x86_`); each must use the raw forbidden spelling. Search all six component inventories for unclassified file-set or byte mismatches and record their owner/disposition. |
## M6 T41 Progress

| S | Result |
| --- | --- |
| S6 | Accepted Shared fixed-width enforcement in `948627378` with closure `9341c723d`. |
| S7 | Accepted: Shared `1d914db89` adapts the Common C-string callback and CRT test boundaries while retaining `lib_u8` event storage. x64/x86 focused Shared regressions and the unblocked MyNES 53/53 suites pass. [Evidence](../etc/evidence/m6-t41-s7-shared-hotkey-text-adapter.md) |
| S8 | Accepted NXVM root-facade retirement in `bee033f59`: `type.h`, `type.c`, and `type-facade` are deleted; x64/x86 complete suites pass 336/336. [Evidence](../etc/evidence/m6-t41-s8-root-types-facade-retirement.md) |
| S9 | Completed pending S9 P: all six Shared source/test trees use the enforced Lib vocabulary; the x86 DEBUG facade is deleted, MyNES and all four NXVM profile x64/x86 artifacts are rebuilt, and the retained native test boundaries are individually recorded. [Evidence](../etc/evidence/m6-t41-s9-shared-six-component-type-convergence.md) |
| S10 | Completed pending commit: versioned artifacts moved to ssets/nxvm and ssets/mynes; live CMake, delivery, documentation and ignore authorities changed with a dedicated root gate. Existing owner worktree changes are included. [Evidence](../etc/evidence/m6-t41-s10-product-artifact-root-rename.md) |
| S12 | Delivered in Shared `4faf19eb7` and pending NXVM evidence P2: canonical SoftPC `df9d1cf8` WASAPI audio is imported with unchanged public ABI; x64/x86 audio checks, manifests, static ownership sweeps and complete repository-only units pass. [Evidence](../etc/evidence/m6-t41-s12-canonical-audio-refresh.md) |
| S13 | Delivered in Shared `b94ea4ffe`: the Raw-CRT x86 negative probe now tests raw `memcpy`, the Lib audio ownership description is synchronized, and the six-component reconciliation is complete pending NXVM evidence P2. [Evidence](../etc/evidence/m6-t41-s13-softpc-raw-crt-reconciliation.md) |
## M6 T41 S4 Shared Closure

Shared S4 closed at `9d7f6ba6c`: Lib now has the approved neutral Types
foundation; all `NOT_CURRENT` dispositions are implemented, and the shared
x64/x86 non-desktop suites and static gates pass. The evidence records the
separate existing KVM Window smoke failure rather than attributing it to S4.

## Retained M6 T41 S2 Progress

**Completed: MyNES M6 T41 S2 existing Types adoption.**

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the ordered M6 T41 sequence on 2026-09-23: Shared/MyNES first, NXVM adoption using existing Lib Types second, then an owner-reviewed facade-retirement design before any final migration. MyNES S1 closed in `a2bfc76e7`. |
| Objective | Replace every NXVM use of an exact existing Lib Types vocabulary item, remove the demonstrated unused standard include, and record the complete legacy-facade remainder without changing Lib. |
| Non-goals | No `src/lib` or `test/lib` change; no `type.h`/`type.c` deletion; no replacement for aliases whose status, pointer-width, bit-width, atomic, formatting, time, trace, or I/O contract lacks an exact existing Lib counterpart; no firmware/media behavior change. |
| Reference Baseline | 439 NXVM production/test files include `type.h`; all still require at least one facade token. Direct non-facade hits are `<limits.h>` in `product/command.c` with no use and `NULL` in `machine/lifecycle.c`. Shared already supplies `LIB_NULL`, `lib_u8/u16/u32/u64`, `lib_i8/i16/i32/i64`, `lib_size`, `lib_bool`, `LIB_TRUE/FALSE`, allocation, memory and selected text vocabulary. |
| Candidate Proposal | [M6 T41 shared type vocabulary convergence](../../mynes/proposals/m6-t41-type-vocabulary-convergence.md) |
| Files And ABI Surface | `src/app-nxvm`, `test/app-nxvm`, NXVM CMake only if direct Lib dependency declaration is needed, and NXVM task evidence/status. |
| Applicable Rules | NXVM product code uses Lib vocabulary rather than duplicate aliases; Lib remains unchanged; every modified caller directly includes its declared owner; test-only native Win32 probes remain external-boundary tests. |
| Verification | x64/x86 NXVM repository-only unit suites pass; relevant current product x64/x86 targets rebuild; static sweep accounts for every `type.h` include and every migrated or remaining facade family; documentation gate passes. |
| Expected Markers | Direct `NULL`/unused C header are gone; all exact existing Lib mappings are migrated; the remaining ledger is sufficient to design S3 without rediscovering source scope. |
| Asset Needs | None; owner-local firmware, media and INI content remain untouched. |
| Reporting Requirements | Report NXVM source/test add/remove/net counts, mapping counts by vocabulary family, each retained facade family and reason, x64/x86 checks, product links, commit/push and the S3 design handoff. |
| Stop Conditions | A candidate replacement changes width, status/error semantics, ownership, variadic formatting, atomic ordering, native-platform contract, or virtual-address meaning; retain it in the ledger for S3 rather than inventing a Lib capability. |
| Exit Criteria | Every exact existing Lib Types equivalent is adopted, no unclassified `type.h` caller or direct C/platform hit remains in NXVM scope, dual-architecture checks/evidence pass, and the S3 design input ledger is committed. |
| Original Owner Request | Clean MyNES first; then clean NXVM with existing Lib Types only; then present a Lib Types expansion and complete root-type retirement design for owner review before implementation. |
| Similar-Issue Sweep | Scan all NXVM production, unit and integration C/H sources for root-facade includes, direct standard/platform headers, raw fixed-width types and facade symbols; classify each as migrated in S2 or retained for S3 with its missing contract. |

## M6 T41 S3 Design Closure

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the ordered M6 T41 sequence. S2 closed in `e85bab86d`; the owner expressly requires review of the facade-retirement design before its implementation. |
| Objective | Produce the minimal shared Lib Types contract, caller-migration batches, test plan and deletion proof needed to retire NXVM's root `type.h`, `type.c` and `type-facade` target. |
| Non-goals | No `src/lib`/`test/lib`, NXVM source, CMake target, binary or API implementation changes; no deletion; no use of `lib_bool` where legacy byte layout must remain; no external asset change. |
| Reference Baseline | S2 recorded 439 remaining `type.h` callers. The legacy facade mixes C aliases/runtime forwarding, byte booleans, host-address integers, outcome statuses, bit operations, trace macros and test output. Existing Lib already owns fixed-width scalars, `lib_status`, C stream formatting aliases and atomics. |
| Candidate Proposal | [M6 T41 S3 Lib Types facade-retirement design](../proposals/m6-t41-s3-lib-types-facade-retirement.md) |
| Files And ABI Surface | `docs/nxvm` only. The proposal defines later `src/lib/types`, `test/lib`, NXVM caller and CMake changes but does not make them. |
| Applicable Rules | Lib gains only neutral cross-product vocabulary; byte-layout, pointer conversion, status and atomic behavior need explicit contract tests; product output policy remains outside Types; one P changes one target. |
| Verification | Complete token inventory and current Lib comparison; proposal has an exact preservation/deletion matrix; documentation governance and actual-diff review pass. |
| Expected Markers | Every root-facade symbol has one receiver: existing Lib, proposed Lib, local product helper, x86-specific helper, or deletion as dead; the later implementation can prove zero `type.h`/`type.c`/`type-facade` references. |
| Asset Needs | None. Owner-local INIs remain untouched. |
| Reporting Requirements | Report retained semantic decisions, proposed public Lib surface, migration order, ABI/behavior tests, deletion gates and the review boundary. |
| Stop Conditions | Any proposed Lib item carries x86 machine semantics, console/output policy, product error behavior, mutable trace control, or a representation change without an ABI test; retain it outside Lib and record its receiver. |
| Exit Criteria | Owner-reviewed design is committed and names every implementation batch and zero-reference proof. Implementation starts only after owner approval. |
| Similar-Issue Sweep | Include all root aliases/macros/functions, all `type-facade` link edges, C standard/platform includes after S2, and MyNES/Shared compatibility impact. |

## Recent Delivery: MyNES M6 T41 S2

| Field | Required record |
| --- | --- |
| Objective | Adopt every exact existing Lib Types equivalent in NXVM without changing Lib, then leave a complete facade-retirement design input. |
| Result | 469 NXVM source/test callers directly consume Lib Types. Exact scalars, null, booleans, allocation, memory and selected text operations migrated; the one `STD_CALLOC` fault-injection seam is deliberately retained. Three x86 test fixtures now use static storage rather than overflowing the 32-bit default stack. `type.h` remains because all 439 callers retain at least one non-equivalent contract. |
| Verification | Static completed-mapping sweep is zero; x64 and x86 optimized `vm-0-5-0535` products build; repository-only x64 and x86 suites each pass 337/337. Documentation governance passes. |
| Evidence | [S2 existing Types adoption](../etc/evidence/m6-t41-s2-existing-types-adoption.md). |
| Next receiver | M6 T41 S3 designs the bounded Lib Types additions and facade-retirement order for owner review before any `type.h`/`type.c` deletion. |

## Prior Delivery: M5 T537 S7

| Field | Required record |
| --- | --- |
| Objective | Repeat the three-view repository audit after the owner withdrew the earlier closure, and establish per-P target-scoped commits. |
| Result | Shared governance now permits explicitly admitted multi-target tasks while preserving exactly one target per P/commit. Fresh NXVM 336/336 unit and 20/20 integration, MyNES 53/53, Shared static/corpus checks, documentation governance and actual-diff review pass. The only repair is a Shared Console test's nine-cell cursor expectation and manifest entry; no production Shared code changed. |
| Evidence | [S7 second three-target audit](../etc/evidence/t537-s7-second-three-target-audit.md). |

## Prior Delivery: M5 T537 S6

| Field | Required record |
| --- | --- |
| Objective | Prove that the six shared components, shared configuration and shared governance have one neutral owner and that App boundaries remain exclusive. |
| Result | Delivered by Shared M5 T537 S6 P1. NXVM presets now explicitly disable MyNES; MyNES presets already explicitly disable NXVM. Lib/Common/x86 manifests and corpus checks pass, and product branding is absent from shared source/test code. The exclusive-scope conclusion is superseded by active S7; the technical evidence remains valid. |
| Evidence | [S6 repository-boundary audit](../etc/evidence/t537-s6-shared-repository-boundary-audit.md). |

## Current Technical Baseline

- `vm-0-5-0535` is the current target. T535 produced eight stripped, optimized
  x64/x86 artifacts for its four fixed products; each product has only one
  executable location, `assets/nxvm/<profile>/`, with its adjacent generated
  INI. The preset-selected default pair is `nxvm_default_0_5_0535_x64.exe` and
  `nxvm_default_0_5_0535_x86.exe`. Paths, architectures, hashes and gate results are in the
  [T535 artifact record](../etc/evidence/t535-s4-artifact-and-integration-verification.md).
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, and `ui`. The separately selected `src/x86` corpus owns
  `xasm32` and the x86 Debug CLI; Common has no x86 dependency.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T537 S6 | Closed in the next Shared-scoped P: root presets now select exactly one App graph; a stale Lib manifest row and product-branded shared-test names are repaired. Lib/Common/x86 manifests/corpus checks and 56 shared labelled unit tests pass; both product documentation gates and independent x64 configuration proof pass. [Evidence](../etc/evidence/t537-s6-shared-repository-boundary-audit.md). |
| T537 S5 | Closed in the next NXVM-scoped P: NXVM authorities now use the live App roots and versioned artifact rule; a standalone NXVM tool guide documents the BYOB x64/x86 routes. Fresh default x64/x86 builds pass 336/336 unit and 20/20 external integration each. The root preset's implicit MyNES graph is explicitly transferred to Shared S6. [Evidence](../etc/evidence/t537-s5-nxvm-developer-audit.md). |
| T537 S4 | Closed in the next Shared-scoped P: a MyNES-only audit removed a misleading NXVM CMake reference, made x86 compiler/test routes self-contained, passed 53/53 product tests on x64 and x86, and verified both PE artifacts. [Evidence](../etc/evidence/t537-s4-mynes-developer-audit.md). |
| T536 | Closed on 2026-09-22: the owner-provided product namespace is the sole live layout: `src/{lib,common,x86,app-nxvm}`, `test/{lib,common,x86,app-nxvm}`, and `assets/nxvm/<profile>/`. CMake, INIs, tools and current documentation use renamed external archives `profiles-nxvm` and `media-nxvm`; historical records remain unchanged. Repository-only unit is 336/336. The 20 external integration rows register but skip because their owner-provided assets are unavailable here. [Evidence](../etc/evidence/t536-s1-product-namespaced-layout.md). |
| T535 | Closed at `e774efb5`; corrective S5 replaces NXVM's duplicate shared-test executable list with three suite-owned aggregate targets, under the owner's explicit shared-corpus exception for later SoftPC/MyNES import. All three suites remain standalone; 336/336 repository-only unit and documentation governance pass. No runtime artifact changes. [History](../history/M5-T535-canonical-softpc-six-component-refresh.md) and [S5 evidence](../etc/evidence/t535-s5-shared-test-aggregation.md). |
| T534 | Closed at `f5170a5d`: 53 bounded repairs exhaust the App/Core convergence ledger without a second production owner or unclassified finding. Current 0534 x64/x86 artifacts for default, 5170, XT and Model 40 are verified in their sole profile directories. 333/333 repository-only unit, default parallel 20/20 integration, 5170 3/3, XT 1/1 and Model 40 3/3 integration pass. [History](../history/M5-T534-app-core-code-quality-remediation.md), [closure ledger](../etc/evidence/t534-app-core-code-quality-ledger.md) and [artifact record](../etc/evidence/t534-s50-artifact-verification.md). |
| T533 | Closed by `99de6d11` and deployment correction `6d5a4828`: fixed XT, 5170, Model 40 and default PC/AT products each use one CMake-selected Profile, one external BYOB asset route and one adjacent NXVM.ini route. Common owns composed host-input ingress; HDD-only validation mutates the production media owner then resets. 335/335 repository-only unit and 20/20 optimized integration pass both serially and in parallel; dual stripped x64/x86 artifacts are verified only in `assets/nxvm/<profile>/`. [History](../history/M5-T533-fixed-machine-products.md) and [evidence](../etc/evidence/t533-s5-product-convergence-ledger.md). |
| T532 | Closed at `0b8918ee`: S1 froze the full map; S2 implementation `66bba6f3` relocated App/Core and repository-only test owners, and retired VDM; S3 proves no former root remains, 336/336 unit and 42/42 integration pass, and stripped 0532 x64/x86 artifacts are byte-identical in both required locations. [History](../history/M5-T532-nxvm-single-product-layout.md) and [evidence](../etc/evidence/t532-s3-single-product-layout-closure.md). |
| T531 | Closed at owner direction on 2026-09-20 after S28 implementation `20c1e71b`: the final canonical SoftPC `2b17749a` six-tree Lib/Common/x86 source-and-test refresh is exact; 338/338 unit, specialized/documentation gates and stripped 0531 x64/x86 artifacts pass. The full external integration remains 41/42: `integration.vm-dos-keyboard-smoke` stays explicitly transferred to [TODO(High)](TODO.md), not relabelled green. [History](../history/M5-T531-shared-common-product-convergence.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |

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
