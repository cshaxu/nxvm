# Project Status

## Current Work

M5 T542 S3 is admitted as the next bounded media-adapter batch. The four-App
split remains queued and depends on complete T542 acceptance.

| Task | Progress |
| --- | --- |
| T542 S3 | Active: extract opaque FDD/HDD providers and their media-resource lifetime. S1/S2 accepted; T remains open. |

### Active Subtask Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T542 S3; next sequential S after accepted S2 at 2c4962430. |
| Admission And Approval | Owner approved T542 public-logic extraction and automatic sequential S admission. Coordinator admits this S on 2026-10-04 within that scope: Shared x86 source/tests/build and NXVM direct consumers only. |
| Objective | Consume the media batch of the T542 ledger: one shared FDD/HDD provider and resource owner under x86/product/machine/media, with opaque production handles. |
| Non-goals | No Lib/Common/MyNES/owner INI change, new backend, controller algorithm or timing-grade change, App split, new worker/FIFO, or generic media framework. |
| Reference Baseline | Clean accepted S2 at 2c4962430; original media behavior and all existing assertions are the preservation baseline. |
| Candidate Proposal | [Shared adapter proposal](../proposals/m5-shared-pc-machine-adapter.md); [finite ledger](../history/M5-T542-shared-pc-machine-adapter.md), media-provider/resource batch. |
| Files And ABI Surface | Move FDD/HDD source, public interfaces and private layouts plus their two owner-local tests; repair App handles, construction/cleanup, tests and build consumers. Add only opaque object allocation/destruction needed to retire App embedding of Shared-private layouts. Existing media IDs/provider contracts and algorithms remain. |
| Applicable Rules | Guide reading set; shared Execution, Architecture, Coding and Document rules; NXVM Architecture/Source Layout/source policy. Unique media-state/lease ownership; public opaque boundary; no reverse App dependency; separate Shared/NXVM P targets. |
| Verification | Full repository unit tests x64/x86; independent media tests and x86 corpus/negative/manifest gates; six manifest checks; documentation gate and diff check; eight optimized stripped 0542 products with PE width/hash evidence. All 58 integration contexts remain mandatory at S8. |
| Expected Markers | Full units pass on both widths; old App media paths removed; no App private media access; standalone Shared provider tests pass; eight 0.5.0542 EXEs without compiler debug sections. |
| Asset Needs | Existing approved build-time firmware inputs only. No master/media/INI mutation; unit inputs remain code-owned or generated locally. |
| Reporting Requirements | Confirm boundary, report migration/build proof and actual added/removed/net source/test lines, then complete scoped P delivery and coordinator actual-diff acceptance. |
| Stop Conditions | Required Lib/Common or MyNES changes, new machine behavior, lost assertion/capability, broader public construction authority, or timing downgrade/new L1; report rather than silently narrowing. |
| Exit Criteria | One source/build owner; no production Shared-private layout dependency; all geometry/protection/address-mark/generation and replacement/eject/failure semantics preserved; verification/artifacts complete; scoped commits pushed and actual changes reviewed. |
| Original Owner Request | Extract the four PC builds' remaining common logic before App split; keep genuine board/profile differences local and avoid duplicate paths or forwarding frameworks. |
| Similar-Issue Sweep | Audit both FDD/HDD constructors, partial initialization, provider-context stability, replacement/eject cleanup, private test access and every source/test/build consumer together. |


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
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2 units, independent helper/gate proofs and 0542 artifact hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
