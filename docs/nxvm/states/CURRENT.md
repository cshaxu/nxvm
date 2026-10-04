# Project Status

## Current Work

M5 T542 S6 is admitted. Cohesive execution/debug adapter extraction is active;
the four-App split remains queued behind complete T542 acceptance.

| Task | Progress |
| --- | --- |
| T542 S6 | Active: extract the complete execution/debug adapter and its neutral construction boundary. S1-S5 accepted; T remains open. |

## Active S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T542 S6, after accepted S5 at 0d8d47c8b243709b0c245865eee301c451c408e8. No new T or reused S. |
| Admission And Approval | Owner's approved T542 shared extraction and automatic sequential S admission; confirmed continuation on 2026-10-04. Allowed targets: Shared (x86 only) and NXVM. Consumers: default, XT, AT and Model40, both host widths. MyNES does not consume Product and stays unchanged. No rule exception. |
| Objective | Consume the complete Execution/debug ledger batch and its Construction prerequisite: migrate the cohesive Core-to-Common driver, bounded runner, pacing/HLT service, reset/fault, debug budget/completion, media binding, display capture and opaque Machine ownership into x86/product/machine. App prepares copied construction values and genuine Profile context; Shared never includes an App header. |
| Non-goals | No Lib/Common source or test change, MyNES change, owner INI rewrite, external master mutation, new App split, new firmware, hardware/timing upgrade, worker/FIFO/clock, universal executor, profile inheritance or command rewrite. S7 retains fixed-composition union/dispatch retirement; S6 must not leave Shared dependent on that App implementation. |
| Reference Baseline | Clean HEAD/origin 0d8d47c8b; accepted S5 source and eight 0542 artifacts. Read NXVM guide, CONTRIBUTING, rules EXECUTION/ARCHITECTURE/CODING/DOCUMENT, NXVM ARCHITECTURE/CODING, source policy, approved proposal and T542 S1 construction/receiver ledger. Architecture and coding governance skills apply in that order. |
| Candidate Proposal | [Shared PC Machine adapter](../proposals/m5-shared-pc-machine-adapter.md), S6; [convergence and construction record](../history/M5-T542-shared-pc-machine-adapter.md). |
| Files And ABI Surface | src/app-nxvm/machine and its callers/tests; profiles/machine_plan boundary and product/config binding; receiving src/x86/product/machine and test/x86/product/machine; src/test x86 CMake/manifests/verifiers; cmake/nxvm source lists/gates; tools/nxvm DAG; NXVM design/history/status and eight 0542 artifacts. Preserve stable symbols where meaningful. Shared public construction/opaque operations use copied values, Core public contracts and explicitly transferred context lifetime, not App enums/layout. |
| Applicable Rules | Neutral dependencies: standalone adapter target and no App include; unique state/dispatch/lifetime: one Common worker/FIFO, Core clock, media lease and adapter body; bounded ABI: opaque Machine/copied observations; rollback: Core routes revoked before providers/context release; simplicity: delete old implementations, no forwarding shell or new framework; provenance: move owner-authorized project source only; test ownership: independent mechanism tests follow x86, real profile/integration tests remain NXVM. Actual-diff coordinator review covers every invariant. |
| Verification | Build both warm root trees build/t542-s2-unit-x64 and -x86 with cmake --build -j 12; execute ctest --test-dir each -L unit -j 12 --output-on-failure. Build standalone test/x86 adapter/tests without App source; run its registered mechanisms and corpus/negative/manifest gates. Run six manifest verifiers, NXVM DAG/D4/document gates and git diff --check. Rebuild only vm-0-5-0542 in the eight existing Release trees; verify PE width, 0.5.0542 identity and absent compiler-debug sections, record SHA-256. Full 58 external contexts remain mandatory T-level S9 proof, not inferred from units. |
| Expected Markers | Complete units on both widths; independent execution-state/debug/construction/lifetime regressions green; all gates/manifests pass; no app-nxvm include/model identity in Shared Machine; old adapter source/targets removed; exactly eight verified 0542 NXVM products, untouched MyNES/INI/Lib/Common. A local test pass alone does not accept this batch. |
| Asset Needs | Existing owner-approved linked firmware inputs only for product builds. Units/standalone tests use code-owned fixtures, never external ROM/INI/media. Retain warm S2 trees and S5 evidence while S6 needs them; native handles must be terminal before cleanup. |
| Reporting Requirements | Confirm/raise contract objection before edits; report receiving-owner and lifetime decisions, verification progress and any scope change. Complete delivery records actual added/removed/net source/test lines, caller/test dispositions, source/artifact hashes and target-scoped pushed P commits; coordinator reviews actual changes before a pure acceptance P. No partial implementation P. |
| Stop Conditions | Need for Lib/Common or MyNES edits, lost profile/CPU/test capability, product behavior change, new license/asset assumption, timing downgrade/new L1 or expansion beyond frozen construction authority. In-scope dependency repairs remain this S; report an objection before changing the packet materially. |
| Exit Criteria | Entire admitted execution/debug/construction-boundary batch has one working Shared owner with no App dependency or duplicated loop/state; all original assertions/58 integration registrations preserved; reset/failure/cancel/teardown and paused-debug authority proven; both full units, standalone and gates pass; all affected artifacts current; complete Shared/NXVM deliveries separately pushed and actual-diff acceptance recorded. T remains open for S7-S9. |
| Original Owner Request | Extract remaining public IBM-PC logic before four-App split; first-principles, flat/minimal ownership; automatic linear S admission, single-session dual-role review, no Lib/Common change and no MyNES rebuild/diff. |
| Similar-Issue Sweep | Reconcile all rg hits for app-nxvm/machine, vm_profile_machine_plan, retained_config, profile_kind and private Machine access across src/test/cmake/tools. Shared mechanisms migrate; actual topology/ROM/identity/D4 stays App-owned; test-only views use the actual owner. Add/update receiver checks to reject old paths and Shared-to-App dependencies. Record each remaining hit with reason and S7 receiver, never hide it behind an allowlist expansion. |

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
  S3 moves media providers to x86/product/machine/media with opaque App handles;
  S4 owns pure input/frame conversion there and removes redundant frame carriers;
  S5 isolates Model40 observations at the prepared Profile context;
  remaining Machine/profile adapter extraction is still open.

## Acceptance Evidence

[T541 history](../history/M5-T541-independent-pc-apps.md) maps the complete
Product inventory to its sole receiver, original regressions, all 58 once-only
integration contexts, final 495/495 units per width, standalone and specialized
gates, six manifests, actual-diff review and code counts. S4 records the eight
then-current 0541 artifact identities; S5 confirms that historical baseline.
Fresh S2-S5 units, helper/media/conversion/Profile proofs and 0542 hashes are in
the [T542 record](../history/M5-T542-shared-pc-machine-adapter.md); T541's hashes
identify the superseded baseline, not the current products.
The [historical design](../history/m5-shared-pc-product.md) preserves the approved
boundary. No hardware timing or new guest-software qualification is claimed.

## Historical Context

[T539](../history/M5-T539-independent-shared-chips.md) closed independent chips;
[T540](../history/M5-T540-shared-ibmpc-integration.md) closed neutral Core/board
extraction. [Queue](QUEUE.md) owns remaining candidates.
