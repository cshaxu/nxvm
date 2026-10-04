# Project Status

## Current Work

M5 T541 S1 is active: inventory and design shared PC Product extraction into x86/product.
The owner narrowed T541 on 2026-10-04: keep all four current builds and extract
their common Product first. The independent App split is a separate queue-head
candidate. No source change or artifact cutover is authorized by design S1.

| Task | Progress |
| --- | --- |
| T540 | Closed through accepted S97 at 9240a3041; complete shared Core/board extraction. |
| T541 S1 | Product inventory, typed boundary and S2-S5 plan delivered for coordinator review; runtime connection permission pending, no implementation delivered. |

Shared implementation P1 `4f2511b13` and NXVM implementation P2 `c83d3a252`
are pushed to origin/master. Coordinator review accepts the actual committed
tree: all original structural ledger members have their sole receiving owner,
required coverage/independent linkage remains intact, and other-App inputs
are unchanged. This closure changes no hardware timing grade.

## S1 Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T541 S1, following closed T540 at 9240a3041; no later NXVM identifier exists. |
| Admission And Approval | Owner's 2026-10-04 request: admit the next task. Allowed change target in S1 is NXVM documents only; Shared/MyNES and prospective App targets are read-only. Owner excludes Lib/Common and existing x86 mechanisms; this planning S revises no code. |
| Objective | Inspect all four current build consumers and freeze the shared Product capability/dependency receiver map and bounded batches; defer independent App creation to the queue-head successor. |
| Non-goals | No source/test/build/runtime change, shared-rule edit, new App directory, EXE rebuild/move, INI write, external asset change, MyNES change, PC110 stub or hardware/timing upgrade. |
| Reference Baseline | 9240a3041, accepted T540, current eight verified 0540 product hashes, unchanged owner INIs, full 492/492 units per width and 58 integration rows. |
| Candidate Proposal | [Shared Product proposal](../proposals/m5-shared-pc-product.md), [queued App split](../proposals/m5-independent-pc-apps.md) and [T541 original-request/design record](../history/M5-T541-independent-pc-apps.md). |
| Files And ABI Surface | NXVM proposal/history/current/queue and directly affected design/evidence links. Read src/app-nxvm, test/app-nxvm, cmake/nxvm, tools/nxvm, assets/nxvm and actual shared contracts/registrations. Design x86/product and test/x86/product, separating fixed App bindings from shared Console/API/INI/UX and their actual Machine-adapter dependency. No ABI or runtime edit in S1. |
| Applicable Rules | Shared Execution and Document; architecture/coding invariants and local architecture/layout/source policy. Apply local architecture/documentation governance skills. Preserve sole state/parser/runner owners, direct composition, one scope per commit, BYOB boundaries and historical provenance. |
| Verification | Enumerate every tracked migration path and generated registration; reconcile four rows and all 58 integration contexts to unchanged predicates. Inspect dependencies and real consumers, validate full coverage/owner dispositions, changed-document links, packet fields, documentation governance and diff checks. Design-only scope needs no new runtime execution or EXE. |
| Expected Markers | No unmapped NXVM path or integration row; no peer-App/private import or unexplained copied runtime in target design; concrete Product and necessary connection owners; no new-App/deployment cutover. |
| Asset Needs | Read existing manifests/INI identities only. Existing owner-managed external ROM/media masters and all deployed EXEs remain unchanged. |
| Reporting Requirements | Confirm T541/S1 scope; report actual dependency/duplication findings and necessary decisions; deliver complete design/map and proposed bounded S sequence, not a claim of completed migration. |
| Stop Conditions | No Lib/Common or existing x86 edits. Enumerate and obtain approval for any necessary App/test/build/manifest connection edits before implementation; no new App targets, owner INI mutation, external inputs or incompatible UX. |
| Exit Criteria | Entire finite shared Product capability/dependency inventory has justified receivers and coverage mapping; no cloning or Shared-to-App imports; required connection permissions are explicit; independent App creation is transferred to the queue-head proposal; actual-change/document checks pass, complete design commit is pushed and coordinator acceptance recorded. |
| Original Owner Request | Original: four independent Apps. Owner revision: separate that split into a queue-head task; T541 now extracts all common Product logic into x86/product, with Lib/Common excluded. |
| Similar-Issue Sweep | Inventory all NXVM product source, tests, CMake/tool/config/document/artifact consumers, model-selection and direct private includes; classify every shared or board-local hit. MyNES runtime is excluded because no change is admitted; shared paths are read-only design inputs. |

## Current Technical Baseline

- `src/x86/chips` owns independent chips; `src/x86/core` owns the sole neutral
  execution/time/memory engine.
- Flat `x86/ibmpc-common`, `ibmpc-at` and `ibmpc-xt` own shared/family board
  mechanisms. Genuine D4 remains Model40-owned.
- NXVM retains four fixed implemented products: XT, AT, Model40 and default.
  PC110 is not runnable; shared Product extraction is active design work and the
  independent four-App split is queued, not implemented.
- Eight optimized stripped 0540 EXEs remain under their `assets/nxvm/<profile>`
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
