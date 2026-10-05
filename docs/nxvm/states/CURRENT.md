# Project Status

## Current Work

M5 T542 S10 is admitted as a narrow corrective S after accepted S1-S9.
The four-App split remains queued and is not admitted.

| Task | Progress |
| --- | --- |
| T542 S10 | Delivery verified: five repairs, full units and 58 integration contexts pass; coordinator acceptance remains. |

## Active S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective: latest closed numeric task M5 T542, next unused S10; no successor has been admitted. |
| Admission And Approval | Owner approves all five audit repairs on 2026-10-04, explicitly without Lib/Common changes. Targets Shared x86 and NXVM; consumers are the four fixed PC builds on x64/x86. MyNES does not consume the changed adapter and must not be edited or rebuilt. |
| Objective | Repair media replacement protection, checked HDD creation capacity, status propagation, dead media fields/comments and boolean vocabulary as one cohesive owner-local cleanup. |
| Non-goals | No Lib/Common source/test change, App split, firmware/timing change, new framework/API family, owner INI change, external-master write or MyNES change/rebuild. |
| Reference Baseline | Accepted closure 881063639 and eight deployed 0542 artifacts. Read product guide, execution lifecycle, CONTRIBUTING, product architecture/coding, shared architecture/coding/document rules and source policy. Apply architecture then coding governance skills. |
| Candidate Proposal | [T542 design](../history/M5-T542-shared-pc-machine-adapter-proposal.md); [S10 corrective ledger](../history/M5-T542-shared-pc-machine-adapter.md). |
| Files And ABI Surface | x86/product/machine media, creation, lifecycle/input/display/debug adapter vocabulary and their direct callers/tests; affected x86 manifests; NXVM evidence and eight 0542 EXEs. Preserve existing names and migrate existing error-return APIs to lib_status rather than adding wrappers. |
| Applicable Rules | One install owner publishes all medium metadata; failed candidates preserve old lease/state. No arithmetic truncation, error swallowing, dead state or misleading thread ownership. Ordinary booleans use lib_bool/LIB_TRUE/LIB_FALSE; atomic flags retain atomic types. |
| Verification | Full repository-only units on x64/x86, existing media/failure regressions plus replacement/overflow/status cases, six manifests, standalone x86 and current specialized/document gates. Build eight Release products and run all original 58 integration contexts once each with unchanged predicates. Verify PE width, stripped sections, deployed hashes and excluded surfaces. |
| Expected Markers | Readonly-to-overlay replacements writable without remove; oversized x86 capacity rejected without publication; original storage/allocation errors preserved; no dead media cursor fields or old device-thread comments; boolean vocabulary consistent in the changed owner. |
| Asset Needs | Existing embedded BYOB inputs and unchanged adjacent owner INIs. Code-owned unit fixtures only. Owned t542-s10 build trees may remain until final verification, then are removed after terminal handles. |
| Reporting Requirements | Confirmation, implementation/verification updates, per-finding proof, actual diff and added/removed/net code counts, artifact hashes, separate target P commits with immediate pushes followed by coordinator acceptance. |
| Stop Conditions | Required Lib/Common or MyNES change, new public behavior outside these fixes, firmware/source assumptions, lost capability, timing downgrade/new L1 or unsupported broad API redesign. |
| Exit Criteria | All five finite audit batches repaired and swept; full units/integration/manifests/gates pass; eight current 0542 artifacts delivered; excluded surfaces unchanged; pushed reviewed delivery and clean worktree before reclosure. |
| Original Owner Request | Admit a new S to solve all five reported quality findings without changing Lib/Common. |
| Similar-Issue Sweep | Search all changed-owner media installation, capacity arithmetic, error-folding callers, ordinary boolean declarations/assignments and obsolete cursor/thread references; classify every hit and repair direct NXVM/test consumers in the same batch. |

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine. Flat `x86/ibmpc-common`, `ibmpc-at` and
  `ibmpc-xt` own common/family board mechanisms; genuine D4 remains Model40-owned.
- `x86/product` owns the one PC INI/request/startup, command/hotkey/Debug
  binding, Common composition and process entry/banner. App retains immutable
  identity, fixed config/factory and genuine Profile/firmware composition.
- NXVM retains four fixed implemented products under app-nxvm: XT, AT, Model40
  and default. PC110 is not runnable. The independent App split is queued.
- Eight optimized stripped 0542 EXEs reside only in `assets/nxvm/<profile>/`,
  with the runtime debugger and unchanged owner INIs. MyNES retains its unchanged
  0043 pair and does not link Product. Lib/Common remain unchanged. S2 moves
  common floppy, ROM validation and contract checks into `x86/ibmpc-common`;
  S3 moves media providers to x86/product/machine/media with opaque App handles;
  S4 owns pure input/frame conversion there and removes redundant frame carriers;
  S5 isolates Model40 observations at the prepared Profile context;
  S6 owns the complete execution/debug adapter and copied construction at
  x86/product/machine; S7 binds fixed constructors without the App plan union/runtime dispatch.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2-S7 units, helper/media/conversion/Profile proofs and 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
