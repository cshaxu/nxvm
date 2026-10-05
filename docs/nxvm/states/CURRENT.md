# Project Status

## Current Work

| Work | Progress |
| --- | --- |
| T542 S20 | Active corrective extraction of remaining AT construction mechanisms and shared PC version. |

## Active S Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective: latest closed numeric T542; next unused S20 after S19. |
| Admission And Approval | Owner request on 2026-10-04 approves extraction described in the preceding source audit. Shared ibmpc and NXVM consumers default/5170/DeskPro/XT are authorized; no MyNES, Lib/Common or chip changes. |
| Objective | Share remaining equivalent AT endpoint/topology/controller and ROM registration mechanisms; move the four PC products' version definition into ibmpc/product. |
| Non-goals | No App split, universal Profile, new hardware/timing, runtime model registry, Lib/Common/MyNES/INI/external-master change. |
| Reference Baseline | Accepted T542 S19 commit 398fa5a1a; existing eight 0542 products and preserved boot predicates. |
| Candidate Proposal | [Residual AT composition](../proposals/m5-at-composition-residual.md); original T542 extraction ledger remains historical evidence. |
| Files And ABI Surface | ibmpc board-at endpoint lookup, board-common explicit AT/ROM materialization, product version interface, corresponding test/ibmpc tests/build/manifests; NXVM profile callers/tests/main, observation-test build links, matching construction closure checks and affected artifacts. |
| Applicable Rules | Task Reading Set, Architecture/Coding/Document/Execution, source policy and local corpus boundaries: sole owners, neutral inputs, no reverse imports, failure-before-publication, full test/artifact proof and single-target commits. |
| Verification | Full NXVM repository-only units on x64/x86; relevant shared and all three AT composition regressions; eight selected product builds; once-only existing integration suite across eight profile/width contexts; manifest/corpus/DAG/docs/diff checks. |
| Expected Markers | No private default lookup implementation; three AT callers use shared assembly; ROM aliases and original topology remain unchanged; version has one shared header and no App duplicate. |
| Asset Needs | Existing approved embedded firmware and external integration inputs only; no acquisition or master writes. |
| Reporting Requirements | Confirm design before execution, report meaningful progress, actual-diff review and counted source/test delta; pushed delivery and artifact hashes before closure. |
| Stop Conditions | Required excluded edit, changed hardware value/timing grade, capability loss or failed regression; diagnose at shared owner rather than add profile workaround. |
| Exit Criteria | All proposal ledger members have source/caller/failure/regression proof; full units and original integration pass; eight optimized stripped 0542 artifacts current; actual-change coordinator review and clean pushed tree. |
| Original Owner Request | Extract the audited common logic and other justified common mechanisms in a new S; all four PC models share version. |
| Similar-Issue Sweep | All four constructors/ROM providers, all three AT topology/controller builders and live lookup tests, build version/entry/binding callers. Release verification found a mixed production-Board/observable-Core pair and a Model40 trace-asserting test linked to production Core; align existing observable test libraries and inspect every trace-provider consumer. Production trace remains disabled. |

The earlier S12-S19 composition batch is accepted. [T542 evidence](../history/M5-T542-shared-pc-machine-adapter.md)
maps every corrective member to its actual owner, callers, failure handling and
regressions. The [corrective proposal](../history/M5-T542-pc-composition-completion-proposal.md)
is archived. S19 delivery 6e1de211f is accepted after separate coordinator review.
S19 is historical acceptance; S20 corrects the additional residual identified by the owner-directed source audit.

## Next Work

[Four independent PC Apps](../proposals/m5-independent-pc-apps.md) is the first
[Queue](QUEUE.md) candidate. Its shared-composition prerequisite awaits S20 correction acceptance,
but the App split is not admitted and has no allocated task identifier.
Its separate naming/scope governance prerequisite remains.

## Retained Runnable Evidence

Source-changing S18 deliveries are Shared f1086b3bf and NXVM 844950505.
Eight optimized stripped 0.5.0542 EXEs remain in assets/nxvm/<profile>, with
unchanged owner INIs and runtime Debug. Their current SHA-256 values are in
T542's S18 table and independently confirmed unchanged during S19.

Fresh final acceptance: full units 504/504 per width; strict matrix 520/520,
zero deferred; all 58 external integration contexts pass once across four
profiles and two widths. Standalone ibmpc, canonical manifests, corpus/DAG,
specialized and documentation checks pass. These fresh S19 results supersede
historical S11 passes for this extraction. Owned temporary build/test trees
are cleaned; EXEs and external masters remain intact.

Lib/Common/x86 algorithms, MyNES, root rules/README and owner configurations
have no corrective-batch diff. The Common wake-failure and Shared vocabulary
follow-ups remain in [TODO](TODO.md); they are not claimed repaired here.

## Current Technical Baseline

- x86/chips owns independent chips; x86/core owns the sole neutral
  execution/time/memory engine. No product knows private chip state.
- ibmpc/board-common, board-at and board-xt own matching PC family mechanisms.
  Shared AT grammar/materialization has three independent consumers:
  default, 5170 and DeskPro; each supplies its own hardware values.
- ibmpc/machine owns one preparation/finishing, media/resource lifetime,
  execution/pacing and copied Common input/output/debug adaptation.
  App transfers a prepared construction and genuine Profile context;
  Core/board teardown precedes its one release.
- ibmpc/product owns shared INI/request/factory, command/hotkey/Debug,
  Common composition, process entry/banner and injected embed/deploy recipes.
  Shared Product owns PC identity/version; App supplies a fixed machine binding,
  constructor and firmware inputs.
- App retains actual ROM layout/alias/provider and machine constraints.
  Genuine D4 and copied Model40 observations stay Model40-owned.
  No DeskPro-to-default/5170 private dependency remains.
- Production links only the selected composition. Multi-profile aggregation
  is explicit test-only input. Default/5170 retain cohesive translation units
  with build-only constructor selection; four top-level Apps are not yet built.
- NXVM retains runnable XT, AT, Model40 and default fixed products. PC110 is
  unimplemented. MyNES retains its unchanged 0043 pair and does not consume ibmpc.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closes independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closes neutral Core/board
extraction; [T541](../history/M5-T541-independent-pc-apps.md) closes shared PC
Product. T542 closes adapter and residual composition extraction, not new
hardware/timing or guest-software qualification. Queue retains those later
qualification scopes without reduced coverage.
