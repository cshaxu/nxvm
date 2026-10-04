# Project Status

## Current Work

M5 T541 remains open. S1's completed Product inventory and design are accepted
at 360dfd776. The owner approved the enumerated App/test/build/manifest
connections on 2026-10-04; S2 is active. Shared implementation remains
limited to the new x86/product; Lib/Common and existing x86 implementations
are excluded. The independent four-App split remains the queue-head candidate.

| Task | Progress |
| --- | --- |
| T540 | Closed through accepted S97 at 9240a3041; complete shared Core/board extraction. |
| T541 S2 | Active INI/request/startup extraction; S1 accepted, T open. |

## Active S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T541 S2 follows accepted S1 P2 bba227a8b. |
| Admission And Approval | Owner's 2026-10-04 approval authorizes the S1-enumerated App removal/binding, test, CMake and manifest connections. Allowed targets Shared and NXVM, one target per P. MyNES has no dependency on the new Product target and is not a change target. |
| Objective | Consume the S1 INI/request/startup ledger batch: relocate the sole parser, copied runtime request and executable-adjacent path construction into x86/product; keep fixed hardware projection in App. |
| Non-goals | No four-App split, Lib/Common or existing x86 implementation edits, new grammar/UX, hardware/timing changes, owner INI or external asset changes, MyNES edits or artifact rebuild. |
| Reference Baseline | Clean bba227a8b; T540 runtime/artifacts at 9240a3041 and S1 inventory in T541 history. |
| Candidate Proposal | [Shared PC Product](../proposals/m5-shared-pc-product.md); [T541 ledger](../history/M5-T541-independent-pc-apps.md). |
| Files And ABI Surface | Shared: five new Product files, matching INI test, x86 source/test CMake and manifests, corpus component registration. NXVM: direct includes/source/link/strict-owner entries, NXVM-only preset target revision, retained fixed-config test, task records and eight 0541 artifacts. Preserve vm_app_ini and request value semantics; startup public header becomes startup_interface.h. |
| Applicable Rules | Product guide, Execution, Document, Architecture and Coding rules, NXVM Architecture/Coding/UI, Contributing and source policy. One parser/request owner, public headers only, code-owned unit inputs, scoped commits, immutable firmware/master inputs; no exceptions beyond approved embedded-EXE publication. |
| Verification | In warm t540-s8-unit-x64/x86 caches: cmake --build build/<cache> --target run-unit-tests; independent test/x86 tools-on/off builds and ctest -L unit; cmake -DX86_ROOT=src/x86 -P src/x86/verify_corpus.cmake; manifest gates; PowerShell documentation gate for nxvm; git diff --check. Transient selection: shared INI and retained App config tests. Build eight release product targets in existing t535-s4 profile/width caches with revision 0541 and verify PE architecture/stripping/hash. |
| Expected Markers | Complete unit suites pass both widths; independent x86 tools-on/off suites pass; parser 0/1, mode/path, rejected-output and startup assertions retained; no old parser source registration or App include in receiver; all affected deployed EXEs current. |
| Asset Needs | Reuse existing owner-authorized firmware build inputs and unchanged deployed INIs. No acquisition, movement or mutation of external ROM/media masters. |
| Reporting Requirements | Confirm boundary before edit; report migrated owner and verification progress; deliver scoped pushed commits, source/test line counts and artifact hashes indexed by history. |
| Stop Conditions | Stop before a Lib/Common/existing-chip edit, unsupported ABI/grammar change, other-App mutation, lost assertion, unavailable toolchain/input, failed required gate or push. Diagnose safely within admitted boundary first. |
| Exit Criteria | Entire S2 batch has sole Shared receiver, old path deleted and all callers connected; fixed projection remains App-owned; required checks/full units/artifacts and actual-diff review pass; scope-separated commits pushed. T remains open for S3-S5. |
| Original Owner Request | Extract all four PC products' shared Product into src/x86/product, automatically progress bounded S tasks, preserve style and one production path; independent App split is queued separately. Latest approval permits necessary connections. |
| Similar-Issue Sweep | Search rg app-nxvm/product/(ini\|startup\|request) and vm_app_ini over src/test/cmake/tools; map every live include, source and owner entry to new receiver. Preserve all existing parser assertions; corpus gate prevents Shared-to-App dependencies. Exclude historical evidence paths from rewriting. |

Shared implementation P1 `4f2511b13` and NXVM implementation P2 `c83d3a252`
are pushed to origin/master. Coordinator review accepts the actual committed
tree: all original structural ledger members have their sole receiving owner,
required coverage/independent linkage remains intact, and other-App inputs
are unchanged. This closure changes no hardware timing grade.

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine.
- Flat `x86/ibmpc-common`, `ibmpc-at` and `ibmpc-xt` own shared/family board
  mechanisms. Genuine D4 remains Model40-owned.
- NXVM retains four fixed implemented products: XT, AT, Model40 and default.
  PC110 is not runnable; Product INI/request/startup now has a sole shared
  receiver awaiting S2 acceptance; command/composition/entry migration remains. The
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
