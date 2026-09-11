# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S5 migrates the shared Debug CLI into `common/debug` and directly cuts NXVM over to its provider route. |

## M5 T527 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T527 S5 on 2026-09-11 after reviewing S4 implementation `4dbd98ec`. The owner requires the actual shared Debug migration: `common/debug` becomes the sole Debug parser/continuation owner and common-session provider; NXVM removes the old Core Debug CLI and VM callback-table route rather than adding a wrapper. |
| Objective | Create `common/debug` as the bounded shared x86 Debug CLI provider. Directly cut NXVM monitor Debug through common session provider registration and common-machine paused operations; migrate the existing DOS Debug command implementation, all eight continuation families, trace/step and breakpoint policy; only after that complete behavior-preserving move, delete the legacy Core parser address, target callback table and their production/test routes. Retain NXVM raw instruction recording only as an injected NXVM product CLI capability: `vm/product` owns command/file policy, `vm/machine` transports copied observation and calls the existing Core Machine Debug API, and Core Machine Debug API remains unchanged. |
| Non-goals | Do not create `common/ui` (S6); change Core CPU/device/firmware/media semantics; alter ordinary NXVM command grammar, YAML/profile policy, native UI behavior or host APIs; expose Core/VM/native/file handles through common; retain a Debug-specific queue, nested raw-console reader, compatibility header/forwarder, polling loop or artifact build/external asset use. |
| Reference Baseline | S4 implementation `4dbd98ec`; [route ledger](../etc/evidence/t527-s1-common-runtime-route-ledger.md), [S2 Debug contract inventory](../etc/evidence/t527-s2-debug-interaction-inventory.md), [S3 cutover evidence](../etc/evidence/t527-s3-common-session-cutover.md), and [S4 cutover evidence](../etc/evidence/t527-s4-common-machine-cutover.md) are binding. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), including its VM-machine extraction table. |
| Files And ABI Surface | May add `src/common/debug/**`, owner-local tests and CMake registration; extend `common/session` only with neutral provider registration/line-result contracts; delete `src/core/debug/{debug,debug_access,debug_target}.*` and the old VM Debug target route; modify VM machine only for bounded common-machine target mapping, copied diagnostic-observation transport and Core execution-plan enforcement; move NXVM-only recorder command/file policy to `vm/product`; modify app/product console composition and direct callers; update CMake, architecture/source-layout/proposal/status/evidence documents. `src/lib`, Core CPU/device/firmware/media behavior, profile semantics and native presenters are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, source/test accounting, actual-diff review and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per parser/continuation/queue, copied values, no Core pointer in common and downward-only common dependency. [Coding](../rules/CODING.md): no forwarding compatibility, duplicate command path, nested input reader, speculative framework or hidden host I/O. [Documentation](../rules/DOCUMENT.md): packet/evidence/index consistency. |
| Verification | Add owner-local Debug grammar/continuation/lifecycle-result regressions and neutral paused-target conformance double; migrate every current Debug smoke; build all affected targets; run complete repository-only unit suite, common forbidden-vocabulary/dependency gates, relevant Debug/provider/route/source-layout gates, documentation governance and `git diff --check`. Integration and dual artifacts remain S8 work. |
| Expected Markers | `common/debug` has the only Debug grammar, continuation and shared breakpoint/trace policy, preserving the existing DOS Debug command surface; it is registered through common session and synchronously uses only common-machine bounded leases. `common/session` remains the only monitor line ingress. NXVM `vm/product` owns optional recorder command/file policy; `vm/machine` retains only copied-observation transport, Core operation/execution-plan mapping and paused Debug API mapping. No legacy Core Debug address, `core_debug_target`, `vm_machine_debug_target`, raw Debug Console reader or second Debug FIFO remains after the migrated command source is verified. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Before P, record parser/continuation/target/recorder/provider old-to-new source and test accounting. Deliver one complete pushed implementation P after direct cutover, full unit/gates and actual-diff self-review. Coordinator independently reviews before accepting S5; do not admit S6 without owner approval. |
| Stop Conditions | Stop and report before implementation if complete migration needs a Core/VM/session/native pointer in common, a second Debug queue/reader/path, an unbounded target operation, host file I/O in Debug, compatibility forwarding, or a user-visible grammar/behavior change not covered by the inventory. |
| Exit Criteria | `common/debug` is independently buildable from only lib/types, common/xasm32 and common/machine public contracts; it owns all eight continuation families and Debug lifecycle results; it reaches NXVM only through registered common-session provider and common-machine bounded operations. Legacy Core Debug parser/target and VM target routes are deleted, recorder is explicitly product-owned, all required tests/gates pass, and evidence accounts for the actual diff. |
| Original Owner Request | Owner approved S5 after requiring a shared product Debug capability: session owns ordered monitor input/output, common Debug owns syntax and continuation, common machine owns bounded paused access, and VM maps only Core operations. |
| Similar-Issue Sweep | Audit every `core_debug*`, `core_debugger*`, `core_debug_target*`, `vm_machine_debug*`, raw Console Debug reader, breakpoint/trace owner, `N/L/W` file path, recorder hook, monitor line path, CMake target and Debug test. Each must become the named common/product/VM receiver or be deleted; no legacy parser/callback/queue route survives. |

## Current Technical Baseline

- `vm-0-5-0526` remains the current target. Its stripped Release developer
  artifacts are `nxvm_0_5_0526_x64.exe` and `nxvm_0_5_0526_x86.exe` until a
  later implementation S creates a replacement task target.
- `src/lib` remains byte-identical to the canonical SoftPC corpus. Accepted
  T527 S2 did not modify it.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T527 S1 | Route ledger accepted at `f187c310`; `common/runtime` is prohibited and every current route has a later migration or deletion receiver. [Evidence](../etc/evidence/t527-s1-common-runtime-route-ledger.md) |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 T527 S1 P2:** coordinator review accepts `5c800dc3`. The route ledger
  records one owner and receiver for every named Core/VM/presentation route,
  fixes the three transport boundaries, and leaves `src/lib` unchanged.

- **M5 T527 S2 P2:** coordinator actual-diff review accepts `b3684432`.
  `common/xasm32` is the sole `lib/types`-only assembler/disassembler owner;
  old Core Debug routes are deleted. Serial unit 299/299 and all required
  source-layout/governance gates pass. [Evidence](../etc/evidence/t527-s2-debug-interaction-inventory.md)
