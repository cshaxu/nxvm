# Project Status

## Current Work

M5 T541 remains open. S1's completed Product inventory and design are accepted
at 360dfd776. S2 is accepted and closed at Shared P1 d0a7499a9 and NXVM P2
4d6750b1a. S3 is accepted and closed at 52b96585c; S4 is active for the shared process entry. Shared implementation remains
limited to the new x86/product; Lib/Common and existing x86 implementations
are excluded. The independent four-App split remains the queue-head candidate.

| Task | Progress |
| --- | --- |
| T540 | Closed through accepted S97 at 9240a3041; complete shared Core/board extraction. |
| T541 S3 | Accepted sole command/hotkey/Common composition receiver and eight 0541 pairs; T open for entry/banner and final integration audit. |


## Active S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T541 S4 after accepted S3 at 52b96585c. |
| Admission And Approval | Owner automatic sequential admission and approved Shared/NXVM supporting edits, 2026-10-04; single-session dual-role review. |
| Objective | Exhaust S1 entry/banner ledger: share the complete process startup/cleanup body and banner formatting; App supplies only immutable identity and its frozen factory. |
| Non-goals | No new App/profile/deployment identity, Lib/Common/existing x86 implementation change, MyNES or owner INI/ROM/media edit. |
| Reference Baseline | 52b96585c, accepted S3 source and eight 0541 EXEs. |
| Candidate Proposal | [Shared PC Product](../proposals/m5-shared-pc-product.md), [T541 ledger](../history/M5-T541-independent-pc-apps.md). |
| Files And ABI Surface | Shared Product entry C/public definition header, matching deterministic entry tests, x86 target/registration/manifests/README. NXVM main/version binding, old banner deletion, build/source ownership references, docs and eight 0541 EXEs. |
| Applicable Rules | Task Reading Set, Execution/Architecture/Coding/Document, NXVM Architecture/Coding/UI and source policy. One startup/cleanup body, immutable identity values, no host/native API, no second console loop or factory. |
| Verification | Full repository units x64/x86; standalone shared entry/unit and tools-off/corpus/manifests; specialized/documentation gates; startup failure/cleanup ordering regressions; eight optimized stripped Release pairs. Integration completion remains S5. |
| Expected Markers | App main supplies values and calls one shared entry; no old banner macro/body; output and process exit semantics preserved; no App imports in Product. |
| Asset Needs | Existing immutable embedded firmware build inputs only; EXE deployment preserves adjacent INIs and all excluded artifacts. |
| Reporting Requirements | Boundary confirmation, progress, actual-diff and counted code review, verification/artifact identities, separate Shared/NXVM P delivery. |
| Stop Conditions | New runtime/native/public capability outside Product, identity or hardware behavior change, lost coverage or failed startup cleanup. |
| Exit Criteria | Entire entry/banner batch has sole receiver; required tests/gates/eight artifacts pass; no obsolete production path; complete source/ABI/diff reviewed and pushed. |
| Original Owner Request | Extract all four PC Apps' identical Product implementation; keep Lib/Common unchanged and automatically execute bounded S tasks to T completion. |
| Similar-Issue Sweep | Inspect all banner/entry consumers and build macros, old header includes, direct CRT and startup/cleanup copies; retire the whole shared entry class without changing firmware/Machine lifetimes. |

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine.
- Flat `x86/ibmpc-common`, `ibmpc-at` and `ibmpc-xt` own shared/family board
  mechanisms. Genuine D4 remains Model40-owned.
- NXVM retains four fixed implemented products: XT, AT, Model40 and default.
  PC110 is not runnable; Product INI/request/startup now has a sole shared
  receiver accepted in S2 and command/composition in S3; process entry is S4. The
  independent four-App split is queued, not implemented.
- Eight optimized stripped 0541 EXEs now reside under their `assets/nxvm/<profile>`
  directories, with the runtime debugger and unchanged owner INIs. MyNES
  retains its unchanged 0043 pair and does not link the extracted x86 targets.

## Acceptance Evidence

[S94](../etc/evidence/t540-s94-source-review.md) records 626-path source/coverage
review and full 492/492 units per width. [S95](../etc/evidence/t540-s95-independent-verification.md)
records independent tools-on 298/298 and tools-off 292/292 per width.
[S96](../etc/evidence/t540-s96-artifacts-and-performance.md) records eight
artifact identities and once-only boot checkpoints.
[S97](../etc/evidence/t540-s97-delivery-review.md) reconciles all 58 integration
rows, final/pushed-tree dual-width gates, six manifests, exact artifact hashes,
scope/code-size review and build/test cost improvements. No owner INI, MyNES,
root README or shared-rule change is included.

## Historical Context

[T540 history](../history/M5-T540-shared-ibmpc-integration.md) records delivery
and acceptance. The [receiver work record](../etc/evidence/t540-s93-whole-board-receiver-work.md#legacy-current-snapshot)
preserves the former accumulated status as historical evidence, not another
status authority. [Queue](QUEUE.md) owns remaining candidates.
