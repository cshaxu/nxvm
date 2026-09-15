# Project Status

## M5 T531 S21 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T531 S21 corrective package-acceptance subtask. |
| Admission And Approval | Owner reported repeated monitor prompts and incorrect text cursor/viewport behavior on 2026-09-14; this packet treats that report as approval for the bounded owner-level correction. Permanent owner approval covers normal commits and pushes. |
| Objective | Restore one correct presentation path: one pending cooked-monitor line, a visible 80x25 Console text surface, and VADP CRTC cursor scan-lines mapped into the one 8x16 glyph geometry. |
| Non-goals | No product-specific UI workaround, no new queue, no presentation-policy change, no SoftPC sibling write, no new external assets, and no whole-T531 closure until SoftPC adopts the shared corrective corpus. |
| Reference Baseline | T531 S20 accepted at `84d57cd0`; NXVM and SoftPC `src/lib`/`src/common` are byte-identical at investigation start. |
| Candidate Proposal | [M5 shared Common product convergence](../proposals/m5-shared-common-product-convergence.md), revised S21. |
| Files And ABI Surface | Shared `src/common/session`, `src/lib/host/win32/console`, their direct tests/manifests; NXVM Core display snapshot/guest frame and VM display-event/frame adapter with direct owner tests. Copied frame gains only the existing owner fact: current text-cell height. |
| Applicable Rules | `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/design/UI.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, and `docs/rules/EXECUTION.md`: one owner/path; public copied values only; canonical shared corpus; user-visible before/after evidence. |
| Verification | Direct Common prompt scheduling, native Console viewport, Core VADP text snapshot, and VM frame conversion tests; complete x64 unit suite; required external integration suite; x64/x86 optimized stripped artifact build and hash/PE verification; actual diff, canonical-corpus, boundary, and documentation gates. |
| Expected Markers | A monitor can have only one pending input line; text output has a 80x25 visible viewport; an 8-scan-line CRTC cursor interval maps to the bottom of the stable 16-scan-line glyph cell without changing Window geometry. |
| Asset Needs | None. |
| Reporting Requirements | Report owner findings before implementation, one P commit with direct test/build evidence, and explicitly state whether SoftPC has adopted the matching shared change. Do not claim T531 whole-task closure while corpus identity is pending. |
| Stop Conditions | Stop for a contrary canonical SoftPC contract, an incompatible native Console restriction, a required unapproved public capability beyond copied cell height, any prior presentation regression, or a failed integration suite without a bounded owner diagnosis. |
| Exit Criteria | All three owner tests pass; full specified verification and artifacts pass; shared correction is recorded for SoftPC adoption; no NXVM-only shared-code branch or duplicate presentation path remains. |
| Original Owner Request | “还是输出了一堆提示符，而且，window模式下光标始终在每行的中间，并不在行的底部。” |
| Similar-Issue Sweep | Audit every completion kind for prompt arming, every Windows Console text-surface call, and every Core→VM→KVM text-frame conversion for a substituted character height. |

| Task | Compact result |
| --- | --- |
| T531 S13 | Canonical SoftPC `54845ac` Lib/Common/test-corpus refresh accepted. P1 `4e7b4a26` imports the exact four trees and direct NXVM adaptations; P2 records equality, gates and dual-architecture acceptance. |
| T531 S14 | Accepted raw-Console prompt correction at P1 `9e7b4eb2`; Common ownership now suppresses prompt publication during guest input. [Evidence](../etc/evidence/t531-s14-raw-console-monitor-ownership.md). |
| T531 S15 | Accepted P1 `5c648c35`: FDD/HDD Core-media adaptation now resides at `src/vm/machine/media` with the one retained `vm-media` target; direct include/CMake paths and obsolete DAG exceptions are removed. [Evidence](../etc/evidence/t531-s15-vm-machine-media-layout.md). |
| T531 S16 | Accepted P1 `59ccb57c`: SoftPC commit `00d4461` now exactly supplies `src/lib`, `src/common`, `test/lib` and `test/common`; NXVM's former `vm/product` root and target are retired in favor of one `vm/app` owner. [Evidence](../etc/evidence/t531-s16-app-consolidation.md). |
| T531 S17 | App keyboard policy is now isolated in `app/keyboard`; `command` retains grammar and the sole callback-context adapter. No Common/Core/VM-machine contract changed. [Evidence](../etc/evidence/t531-s17-app-keyboard-convergence.md). |
| T531 S18 | App composition is the sole NXVM Common-machine owner: VM describes/binds the driver only; UI, session, Common, then VM teardown is explicit. 325/325 unit and x64/x86 artifact verification pass. [Evidence](../etc/evidence/t531-s18-common-machine-composition-ownership.md). |
| T531 S19 | Accepted at P2: SoftPC `433f57e` Common/test corpus is byte-identical; Common shutdown joins callbacks before the sole App cleanup path releases UI/session state. 325/325 unit and x64/x86 artifact verification pass. [Evidence](../etc/evidence/t531-s19-common-shutdown-refresh.md). |
| T531 S20 | Accepted at P2: retired the `vm/machine/runtime` naming shell; one `vm-machine` target directly owns the Core/Common adapter and retains `media/` as its sole FDD/HDD storage-provider child. 325/325 x64 unit, five boundary gates, documentation governance and stripped x64/x86 artifacts pass. [Evidence](../etc/evidence/t531-s20-vm-machine-responsibility-layout.md). |

## Current Technical Baseline

- `vm-0-5-0531` is the current target. Its stripped Release artifacts are
  `nxvm_0_5_0531_x64.exe` and `nxvm_0_5_0531_x86.exe` in both
  `build/output` and `assets/sessions`.
- `src/lib` is the canonical shared host-services corpus. `src/common` is an
  independently buildable, Lib-public-contract-only product-capability corpus:
  `machine`, `session`, `ui`, `xasm32`, and `debug`.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T531 S11 | Accepted at `3a275380`: imported the exact 94-file SoftPC `987d82e` KVM Lib corpus, deleted all retired UI component paths, and directly cut Common, VM, CMake and tests to the canonical contracts. Lib boundaries, x64/x86 optimized 0531 builds, 299/299 unit and documentation governance pass. [Closure evidence](../etc/evidence/t531-s11-canonical-kvm-lib-refresh.md). |
| T530 | Closed at owner direction on 2026-09-13 after implementation bc9a1247; pinned canonical Lib import and Common adaptation, recorded 299/299 unit and 42/42 integration, both 0530 artifacts verified by hash. No new manual test is claimed. Later source revisions and Common UX convergence belong to the next task. [History](../history/M5-T530-canonical-softpc-lib-refresh.md). |
| T529 | Closed at `cc5fd4a7`: S1 exhaustively classified the 29-file VM-machine corpus and S2 removed the only two proven dead controls. No Common migration was invented: Common remains the sole copied protocol owner; Core and CCPU executors remain distinct product adapters. 299/299 unit, focused regression, specialized/documentation gates and stripped 0528 x64/x86 artifacts pass. The unchanged 39/42 external-integration exception remains transferred to the Model-40/5170 boot-chain TODO. [History](../history/M5-T529-common-machine-residual-audit.md) and [evidence](../etc/evidence/t529-s2-dead-control-cleanup.md). |
| T528 | Closed under the owner's 2026-09-12 exception after `9d892446`: all VM-machine duplicate owners are removed, 299 unit cases, specialized gates, documentation governance and stripped x64/x86 artifacts pass. The known 39/42 external-integration result is transferred unchanged to the external-ROM Model-40/IBM-5170 boot-chain TODO; it is not represented as green. [History](../history/M5-T528-vm-machine-owner-cleanup.md) and [evidence](../etc/evidence/t528-s7-task-closure-attempt.md). |
| T527 | Common product-runtime convergence closed at `2c596f2c`: one Common corpus owns xasm32, Debug, session, machine and UI; NXVM retains one VM Core adapter, product policy owner and App composition root. Independent Common verification, 299/299 unit, 42/42 external integration, specialized gates and stripped x64/x86 artifacts pass. [History](../history/M5-T527-common-product-runtime-convergence.md) and [evidence](../etc/evidence/t527-s9-reusable-common-closure.md). |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 Td S166 P1:** queued two owner-approved, unnumbered candidates in
  dependency order: VM-machine owner cleanup, then Common-machine executor
  completion. Both use the audited NXVM/SoftPC two-consumer criterion; neither
  allocates or admits a numeric implementation task.

- **M5 T527 S9 P2:** records independent corpus verification, complete test
  closure, artifact identity, proposal retention, and task closure after
  implementation `2c596f2c`.
