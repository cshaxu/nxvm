# Project Status

## Current Work

M5 T541 remains open. S1's completed Product inventory and design are accepted
at 360dfd776. S2 is accepted and closed at Shared P1 d0a7499a9 and NXVM P2
4d6750b1a. S3 is now admitted for the complete command/composition binding batch. Shared implementation remains
limited to the new x86/product; Lib/Common and existing x86 implementations
are excluded. The independent four-App split remains the queue-head candidate.

| Task | Progress |
| --- | --- |
| T540 | Closed through accepted S97 at 9240a3041; complete shared Core/board extraction. |
| T541 S2 | Accepted sole INI/request/startup receiver and eight 0541 artifacts; S2 closed, T open for remaining Product batches. |


Shared implementation P1 `4f2511b13` and NXVM implementation P2 `c83d3a252`
are pushed to origin/master. Coordinator review accepts the actual committed
tree: all original structural ledger members have their sole receiving owner,
required coverage/independent linkage remains intact, and other-App inputs
are unchanged. This closure changes no hardware timing grade.

## Current Technical Baseline

## Active S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, M5 T541 S3 after accepted S2 at 9432c70b1. |
| Admission And Approval | Owner approval and automatic sequential S admission on 2026-10-04; supporting Shared/NXVM connection edits approved. Single-session coordinator/executor review. |
| Objective | Move command, hotkey and Common composition ownership to x86/product through one frozen App factory and copied INFO/speed binding. Consume the S1 command/composition ledger batch. |
| Non-goals | No four-App split, Lib/Common change, existing x86 implementation change, MyNES change, owner INI change, hardware/timing change or new worker/queue. |
| Reference Baseline | 9432c70b1, accepted S2 source and eight 0541 artifacts. |
| Candidate Proposal | [Shared PC Product](../proposals/m5-shared-pc-product.md), [T541 ledger](../history/M5-T541-independent-pc-apps.md). |
| Files And ABI Surface | Shared: new Product command/composition/keyboard and typed factory, matching test/x86/product, x86 target/corpus registrations and manifests. NXVM: old Product deletion, config/main binding, App tests/build references, docs and eight 0541 EXEs. App machine implementation remains unchanged unless a proven obsolete wrapper is removed. |
| Applicable Rules | Task Reading Set; shared Execution/Architecture/Coding/Document; NXVM Architecture/Coding/UI and source policy. Sole Product owner, declared acyclic dependencies, opaque construction handle, atomic publication/rollback, unchanged Common reducer and copied inputs. |
| Verification | Complete repository units x64/x86; standalone shared tools-on/off tests and corpus/manifests; specialized/documentation gates; composition rollback, INFO state, command/Debug and CAD/AltEnter regressions; eight optimized stripped 0541 builds. T-level 58 integrations retained for S5. |
| Expected Markers | Full suites pass; no App include in Product; old command/composition/keyboard production paths absent; one Common Machine/Session/UI composition; AltEnter complete make/break sequence. |
| Asset Needs | Existing immutable embedded firmware build inputs only; no external master or INI writes; MyNES artifacts unchanged. |
| Reporting Requirements | Initial boundary confirmation, material progress, complete verification/evidence, counted code delta and separate Shared/NXVM P delivery. |
| Stop Conditions | Missing public capability requiring Lib/Common edits, changed hardware behavior, unapproved consumer or failed rollback/coverage proof. |
| Exit Criteria | Entire ledger batch has sole receiver, all original assertions preserved, no reverse dependency/second runner, required tests/artifacts/gates verified and actual-diff reviewed, ordered target-specific commits pushed. |
| Original Owner Request | Extract all four Apps' shared Product logic into src/x86/product; do not modify Lib/Common; automatically admit each bounded S until T closure. |
| Similar-Issue Sweep | Search all Product/App callers, build lists and tests for private App dependencies, duplicated media/input routes, raw CRT and old paths; repair the complete affected batch, retain genuine machine adapter paths with caller proof. |


- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine.
- Flat `x86/ibmpc-common`, `ibmpc-at` and `ibmpc-xt` own shared/family board
  mechanisms. Genuine D4 remains Model40-owned.
- NXVM retains four fixed implemented products: XT, AT, Model40 and default.
  PC110 is not runnable; Product INI/request/startup now has a sole shared
  receiver accepted in S2; command/composition/entry migration remains. The
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
