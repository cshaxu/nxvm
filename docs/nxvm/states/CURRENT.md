# Project Status

## Current Work

M5 T542 S1 is accepted and closed at d7b92e35a. S2 is admitted to extract
shared PC construction helpers. The four-App split remains queued and
depends on complete T542 acceptance.

| Task | Progress |
| --- | --- |
| T542 S2 | Implementation verified: shared construction helpers, callers/tests and eight 0542 artifacts; delivery and coordinator review pending. |

## Active S Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T542 S2 after accepted S1 d7b92e35a; next unused numeric S. |
| Admission And Approval | Human owner approved T542 remaining shared-logic extraction and automatic sequential S admission. Scope follows S1 frozen Helpers batch. Targets: Shared source/tests/build/manifests and NXVM caller/build/gate/docs/artifacts, in separate P commits. No Lib/Common or MyNES change. |
| Objective | Establish one x86/ibmpc-common owner for floppy geometry/rate/double-step, Option ROM validation and profile-contract validation, removing original App implementations and repairing every caller. |
| Non-goals | No firmware or topology behavior change, Machine execution move, new API framework, ROM/media/INI edits, new App, timing reclassification or Lib/Common change. |
| Reference Baseline | S1 accepted d7b92e35a; current eight 0541 EXEs and unchanged owner INIs; Helpers ledger and contracts frozen in T542 history. |
| Candidate Proposal | [T542 proposal](../proposals/m5-shared-pc-machine-adapter.md); [S1 ledger](../history/M5-T542-shared-pc-machine-adapter.md), consume complete Helpers batch. |
| Files And ABI Surface | Move six helper C/H files to src/x86/ibmpc-common using public *_interface.h names, preserving existing symbols/values; relocate independent contract assertions to test/x86/ibmpc-common. Repair App/profile/Machine and test includes, both CMake registries, affected static gates and src/test x86 manifests. No native/mutable device layout crosses public boundary. |
| Applicable Rules | Execution full-batch repair, git mv, one target per P, full units, affected dual-width EXEs and cleanup; Architecture sole helper owner/no Shared-to-App edge; Coding Types vocabulary and unchanged coherent implementation; Documentation authority/evidence. Read NXVM guide, contributing, T542 proposal/history, architecture/coding/roadmap, shared execution/document/architecture/coding rules and source policy. |
| Verification | Full repository units x64/x86; standalone x86 helper/corpus/manifest checks and targeted negative/boundary tests; compile all four fixed products for both widths as 0542, preserve INIs and verify PE/stripped identity/hashes; documentation gate, exact include/symbol sweep and git diff --check. Commands/build-tree identities are recorded before launch below. |
| Expected Markers | Unit suites pass with no removed predicate; Shared build needs no App source; no old helper-path reference or duplicate source target; affected manifests valid; eight optimized stripped EXEs and unchanged INIs. |
| Asset Needs | Existing approved BYOB firmware is a build input only; external masters remain unchanged, integration not claimed from unit compilation. |
| Reporting Requirements | Confirm Helpers boundary; report any unrelated coupling; deliver actual code-size/path/caller sweep, tests and artifact evidence; complete target-scoped commits/pushes before coordinator review. |
| Stop Conditions | Helper extraction requires broader machine behavior, Lib/Common change, lost assertion/capability, new asset/license approval or timing downgrade/L1: stop affected scope and report. |
| Exit Criteria | Complete Helpers batch migrated with sole source owner and all callers/build/tests working; required units/gates/artifacts pass; no App dependency in Shared, old helpers removed, scoped actual-diff review and complete delivery pushed. |
| Original Owner Request | Extract remaining IBM-PC public logic before App split; bounded S tasks execute sequentially through T closure. |
| Similar-Issue Sweep | Search old helper paths/symbols across src/test/cmake/tools/docs current authorities; migrate every producer/caller/source-list and preserve all four-board constraints. Historical evidence retains old paths explicitly as history. |

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. Flat `x86/ibmpc-common`, `ibmpc-at` and
  `ibmpc-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `x86/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and its real Machine/profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- Eight optimized stripped 0542 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common remain unchanged. S2 moves
  common floppy, ROM validation and contract checks into `x86/ibmpc-common`;
  remaining Machine/profile adapter extraction is still open.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
current artifact SHA-256 identities; S5 confirms they are unchanged.
Fresh S2 units, independent helper/gate proofs and 0542 artifact hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
