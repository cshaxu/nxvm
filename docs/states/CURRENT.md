# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S2 extracts `common/xasm32` and freezes the shared Debug route before common Debug migration. |

## M5 T527 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the revised shared `common/xasm32` and `common/debug` architecture and T527 S2 execution on 2026-09-11. It supersedes the earlier unimplemented direct `core/debug -> core/machine` plan. S1 is accepted at `f187c310`. This S extracts `common/xasm32` and freezes the concrete shared Debug route before the later session, machine and Debug migrations. |
| Objective | Move NXVM's x86 assembly/disassembly logic to `common/xasm32`, with `lib/types` as its only dependency and no behavior change. Record the exact shared Debug contracts: session-registered CLI-provider behavior; synchronous paused Debug operations owned by `common/machine`; NXVM's `core/machine -> vm/machine -> common/machine -> common/debug` path; ordinary session lifecycle requests; bounded `N/L/W` product file-service exchange; mandatory DOS-style trace/step; and optional NXVM recorder disposition. No Debug parser or executor path moves in this S. |
| Non-goals | Do not create `common/debug`, `common/session`, `common/machine` or `common/ui`; modify `src/lib`; change CPU/device/debug command meaning; expose a Core-machine, CPU, RAM, VM, session, host or native handle; add a polling loop, thread or platform dependency; migrate product Console grammar or generic VM queues; or build/deploy task artifacts. |
| Reference Baseline | T527 S1 accepted at `f187c310`; [route ledger](../etc/evidence/t527-s1-common-runtime-route-ledger.md) is the complete migration/deletion inventory. The production reference is `98ac51a4` plus T527 S1 documentation only. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), S2 batch: xasm32 extraction and shared Debug-contract freeze. |
| Files And ABI Surface | May change only the existing assembly/disassembly owner and direct callers, new `src/common/xasm32/**`, `src/lib/types/**` only when an existing public scalar/status definition is required by xasm, the corresponding canonical-library manifest/CMake/test registration, direct CMake/test registrations, [System Architecture](../design/ARCHITECTURE.md), [Source Layout](../design/CODING.md), S2 evidence/status records and this proposal. `common/debug`, `common/session`, `common/machine`, `common/ui`, VM runtime behavior, CPU/device semantics, assets and integration fixtures are excluded. Old xasm source is deleted in the same P after callers move. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, actual-diff review, evidence, source/test line accounting and similar-issue sweep. [Architecture](../rules/ARCHITECTURE.md): one owner/route, copied opaque boundary values and composition-only binding; the approved future Debug route is `core/machine -> vm/machine -> common/machine -> common/debug`, with no direct Debug-to-Core edge. [Coding](../rules/CODING.md): no forwarding compatibility API, no speculative framework, bounded validated data and deterministic cleanup. [Documentation](../rules/DOCUMENT.md): concise task evidence/status. |
| Verification | Add focused xasm input/encoding/disassembly/error regressions and update all direct callers. Build affected targets, run full repository unit suite, relevant static source-layout gates, documentation governance and `git diff --check`. Integration and dual artifacts are deferred to T527 S8 because S2 does not change profile/media/UX behavior. |
| Expected Markers | `common/xasm32` has one public x86 assembly/disassembly contract and depends only on `lib/types`; its former owner has no retained source or caller route. The S2 evidence maps the exact common Debug API and command classes without introducing an implementation: registered session CLI provider; synchronous common-machine paused operation; NXVM adapter path; lifecycle return; `N/L/W` file-service exchange; trace/step; optional recorder. |
| Asset Needs | None. No source import, firmware, guest media, YAML, font, trace or external binary is read, written or packaged. |
| Reporting Requirements | Report the existing xasm owner, every caller, and exact old-to-new source/test line accounting before the P. Deliver one complete pushed P only after all direct callers and tests move, full unit and stated gates pass, line accounting and actual-diff self-review complete. Coordinator independently reviews the P before accepting S2; do not admit S3 without owner approval. |
| Stop Conditions | Stop and report before source changes if the current assembler/disassembler cannot move without an exposed Core/VM pointer, a host/native dependency, duplicated types, a compatibility route or Debug semantic change. Stop if a similar xasm entry point exists outside the S1 ledger and cannot receive the same owner treatment. |
| Exit Criteria | `common/xasm32` is the sole assembly/disassembly production path, depends only on `lib/types`, has no Core/VM/product/platform dependency, and all direct callers/tests use it. The old source has no retained production route or compatibility wrapper. The shared Debug-contract evidence is complete and agrees with the revised proposal; stated regressions and full unit/gates pass; evidence records source/test net change. |
| Original Owner Request | Owner requires one shared `common/xasm32`, one shared `common/debug`, session-owned monitor ordering, and the paused Debug operation route `core/machine -> vm/machine -> common/machine -> common/debug`; no second Debug queue or direct Debug-to-Core route may survive. |
| Similar-Issue Sweep | Audit every assembly/disassembly source, public header, caller, CMake/test registration and duplicate type declaration. Each must move to `common/xasm32`, be a private helper under that owner, or be deleted; no second xasm production path may survive. |

## Current Technical Baseline

- `vm-0-5-0526` remains the current target. Its stripped Release developer
  artifacts are `nxvm_0_5_0526_x64.exe` and `nxvm_0_5_0526_x86.exe` until a
  later implementation S creates a replacement task target.
- `src/lib` remains byte-identical to the canonical SoftPC corpus. T527 S2
  must not modify it.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T527 S1 | Route ledger accepted at `f187c310`; `common/runtime` is prohibited and every current route has a later migration or deletion receiver. [Evidence](../etc/evidence/t527-s1-common-runtime-route-ledger.md) |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 T527 S1 P2:** coordinator review accepts `5c800dc3`. The route ledger
  records one owner and receiver for every named Core/VM/presentation route,
  fixes the three transport boundaries, and leaves `src/lib` unchanged.
