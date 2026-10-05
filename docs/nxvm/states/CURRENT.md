# Project Status

## Current Work

M5 T542 is reopened for residual composition extraction. S11's package
relocation remains accepted; it did not finish every construction/factory
member of the original inventory. The four-App split remains queued and
unadmitted until the corrective ledger is exhausted.

| Work | Progress |
| --- | --- |
| T542 S12 | Corrective design reconciliation; S13-S19 planned, not active. |

## Active S12 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective; latest closed numeric T542, next unused S12; S1-S11 remain immutable. |
| Admission And Approval | Owner request on 2026-10-04 to reopen this T, plan the remaining gaps and include DeskPro in the shared AT mechanism. NXVM documentation target only for S12. |
| Objective | Reconcile all remaining pre-split extraction members and plan bounded S13-S19 with receivers, retained differences and exits. |
| Non-goals | No source/build/test/artifact edits, App split, Lib/Common/MyNES change, INI/external asset change or hardware/timing qualification. |
| Reference Baseline | a6f81ad8f; accepted S11 Shared 51a6d209e and NXVM 8c24462b3; source observations indexed in the corrective proposal. |
| Candidate Proposal | [Composition completion](../proposals/m5-pc-composition-completion.md), finite ledger and S12 design batch. |
| Files And ABI Surface | docs/nxvm Current, Queue, Roadmap, successor proposal, corrective proposal and T542 history; no ABI changes. |
| Applicable Rules | NXVM guide, shared Execution/Document, CONTRIBUTING; original T542 proposal and four-App proposal. Existing architecture/coding boundaries constrain the planned owners. |
| Verification | NXVM documentation governance; git diff --check; manual actual-diff, link, allocation and six-ledger-member review. No executable inputs change, so no fresh units/integration or EXE rebuild is required for this design-only S. |
| Expected Markers | T542 open; all three AT consumers named; six residual classes mapped to S13-S18; S19 whole-task gate; successor blocked on completion. |
| Asset Needs | None; accepted 0542 artifacts and owner INIs remain untouched. |
| Reporting Requirements | Explain three-consumer AT boundary, corrective rather than new scope, planned linear S exits and documentation delivery. |
| Stop Conditions | Required code change or new construction/hardware authority during S12; revise/admit the appropriate implementation packet first. |
| Exit Criteria | Complete reviewed ledger, truthful task/queue/dependency status, documentation gate pass, pushed complete P and coordinator acceptance. |
| Original Owner Request | Shared AT assembly should serve 5170/default/DeskPro; other audit findings accepted; reopen this T and plan new S tasks to fix gaps. |
| Similar-Issue Sweep | Reconcile four constructors and their common contracts, factories, finishing, ROM preparation and build/test consumers; this S records the class, implementation proof follows S13-S19. |

## Retained Runnable Evidence

Shared delivery is `51a6d209e`; NXVM callers, gates and eight current artifacts
are `8c24462b3`. Complete units pass 499/499 per width, all 58 once-only optimized
integration contexts pass, and independent package, manifest, specialized and
documentation checks pass. The [S11 evidence](../history/M5-T542-shared-pc-machine-adapter.md)
records actual-diff review, counts, intermediate failures and current hashes.
Lib/Common, MyNES, INIs and external masters remain unchanged. The Common
wake-failure contract and Shared vocabulary follow-up remain in [TODO](TODO.md),
not authorized or claimed repaired by this relocation.

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. `ibmpc/board-common`, `ibmpc/board-at` and
  `ibmpc/board-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `ibmpc/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and genuine Profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- Eight optimized stripped 0542 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common remain unchanged. S2 moves
  common floppy, ROM validation and contract checks into `ibmpc/board-common`;
  S3 moves media providers to ibmpc/machine/media with opaque App handles;
  S4 owns pure input/frame conversion there and removes redundant frame carriers;
  S5 isolates Model40 observations at the prepared Profile context;
  S6 owns the complete execution/debug adapter and copied construction at
  ibmpc/machine; S7 binds fixed constructors without the App plan union/runtime dispatch.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2-S11 units, helper/media/conversion/Profile proofs and current 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
