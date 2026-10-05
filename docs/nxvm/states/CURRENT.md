# Project Status

## Current Work

M5 T542 S10 is accepted. Owner-approved corrective S11 is active: finish the
shared PC extraction boundary by separating IBM PC code from neutral x86.
The four-App split remains queued and is not admitted.

| Task | Progress |
| --- | --- |
| T542 | S11 active: relocate the accepted PC implementation without changing behavior. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective M5 T542 S11; consolidate the remaining PC package boundary of the latest closed extraction task, not the queued four-App split. |
| Admission And Approval | Owner explicitly approves S10 closure and S11 on 2026-10-04; targets Shared and NXVM, each commit scoped separately. No Lib/Common or MyNES changes authorized. |
| Objective | One top-level ibmpc package owning board-common, board-xt, board-at, machine and product, with matching tests and build/verification entry points. |
| Non-goals | No chip/Core algorithms, timing, public C ABI, INIs, firmware/media, Lib/Common, MyNES or four-App implementation changes. |
| Reference Baseline | Accepted S10 ee440d463996f2d8761b0697b34a2074df7266eb; clean worktree; 499 units per width and 58 integration contexts. |
| Candidate Proposal | T542 history, S11 five-owner relocation ledger; queued four-App proposal consumes the new package without being admitted. |
| Files And ABI Surface | src/ibmpc and test/ibmpc replace x86 PC directories; Shared CMake/verifiers/manifests and NXVM callers/tools/current design. Include/target names change; function/type behavior stays unchanged. |
| Applicable Rules | Execution lifecycle and target-scoped P; architecture sole owner and downward dependencies; coding mechanical migration and no wrappers; documentation single authority; source policy unchanged BYOB inputs. Architecture/coding governance skills apply. |
| Verification | Complete unit per width, unchanged optimized integration once per context, standalone x86 and ibmpc builds/gates, manifests, NXVM specialized/documentation checks, normalized source comparison and diff review. |
| Expected Markers | No old production PC paths, no x86-to-ibmpc dependency, all original tests retained and passing; eight optimized stripped 0542 artifacts. |
| Asset Needs | Existing qualified BYOB inputs only; no INI edits, master writes or protected raw files added. |
| Reporting Requirements | Report dependency confirmation, migration/build progress, reviewed commits, full results, line counts and any residual gap. |
| Stop Conditions | Unexpected semantic changes, reverse production dependency, Lib/Common or MyNES changes, missing assets or newly discovered nonmechanical defect. |
| Exit Criteria | Five owners migrated once, no forwarding layer, matching complete tests, independent package verification, all required checks and artifacts, target-scoped pushed commits and clean tree. |
| Original Owner Request | Add ibmpc outside x86 containing board-common, board-xt, board-at, machine, product; move the previous x86/ibmpc-* and x86/product there after S10. |
| Similar-Issue Sweep | All source includes, target dependencies, test fixtures/registration, verifier paths, active architecture/proposals and consumers; exclude historical evidence from path rewriting. |

S10 repairs replacement protection/geometry, checked capacity, status
propagation, dead media state/comments and ordinary boolean vocabulary.
Complete units pass 499/499 per width and all 58 final optimized integration
contexts pass. The eight rebuilt 0542 EXEs and their current hashes are recorded
in the [S10 evidence](../history/M5-T542-shared-pc-machine-adapter.md).
Shared delivery is `42a2c2178`; NXVM callers/artifacts/evidence are `8b9111c66`.
Lib/Common and MyNES remain unchanged. The discovered Common wake-failure
contract gap is reported in [TODO](TODO.md), not authorized or claimed repaired.

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
Fresh S2-S10 units, helper/media/conversion/Profile proofs and current 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
