# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S3 establishes and directly cuts NXVM over to the one `common/session` owner. |

## M5 T527 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T527 S3 execution on 2026-09-11, after accepting S2 at `6796a34a`. The owner expressly requires NXVM to use the new session mechanism now and to remove the duplicate `vm/session` FIFO/reducer rather than retaining it until later components exist. |
| Objective | Establish `common/session` as the sole bounded monitor/machine/UI control FIFO, run-id owner and lifecycle reducer. Directly bind NXVM through a minimal `vm/app` composition leaf, remove the old `vm/session` lifecycle/FIFO/reducer route in the same P, and relocate the product YAML catalog under `vm/product`. Live CLI-provider registration follows in S5 with `common/debug`, avoiding an unused S3 callback API. |
| Non-goals | Do not create `common/machine`, `common/ui` or `common/debug`; move the Debug parser; change Core/CPU/device, firmware, media, YAML grammar, product command meanings, native UI behavior, profile resolution or host APIs; expose a Core/VM/session/native pointer through common; add a forwarding compatibility facade, second queue, polling loop or artifact build. |
| Reference Baseline | T527 S2 accepted at `6796a34a`; [route ledger](../etc/evidence/t527-s1-common-runtime-route-ledger.md) and [S2 Debug contract inventory](../etc/evidence/t527-s2-debug-interaction-inventory.md) remain binding route evidence. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), revised S3 cutover batch. |
| Files And ABI Surface | May add `src/common/contracts.h`, `src/common/session/**`, owner-local tests and direct CMake registration; replace/delete `src/vm/session/{control,session}.*`; relocate its YAML catalog to `src/vm/product/**`; add the minimal `src/vm/app/**` composition binding; and adjust direct NXVM Console/presentation/machine callers, architecture/source-layout/proposal/status/evidence documents. `src/lib`, Core, VM machine semantics, VM profile semantics and native presenters are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, source/test accounting, actual-diff review and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per mutable fact/queue, copied values and composition-only binding. [Coding](../rules/CODING.md): no compatibility forwarding, no speculative framework, bounded data and deterministic teardown. [Documentation](../rules/DOCUMENT.md): packet/evidence/index consistency. |
| Verification | Add owner-local common-session lifecycle/FIFO/run-id/overflow regressions, migrate applicable existing VM session tests, build all affected targets, run complete repository-only unit suite, relevant owner/source-layout gates, documentation governance and `git diff --check`. Integration and dual artifacts remain T527 S8 work because S3 does not alter profile/media/UX semantics. |
| Expected Markers | There is one FIFO/reducer/run-id owner in `common/session`; its public types contain only copied neutral values and lib UI values. NXVM production traffic uses it through a stateless `vm/app` binding. No `vm/session` control or lifecycle source, target or caller remains; YAML catalog is product-owned. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Before P, record every migrated/deleted queue/state owner and the old-to-new source/test accounting. Deliver one complete pushed implementation P after direct cutover, full unit/gates and actual-diff self-review. Coordinator independently reviews before accepting S3; do not admit S4 without owner approval. |
| Stop Conditions | Stop and report before implementation if direct NXVM cutover requires a Core/VM pointer in common, duplicated queue/state, a second lifecycle route, an unbounded product token, native API exposure, or a behavior/grammar change outside the packet. |
| Exit Criteria | `common/session` is an independently buildable lib-only component and the only NXVM session FIFO/reducer/run-id owner; NXVM binds it through stateless ports; old VM session owner code is deleted and catalog is product-owned; all scope tests/gates pass and evidence accounts for the actual diff. |
| Original Owner Request | Owner requires S3 to integrate immediately with NXVM and delete the duplicate old session implementation. Any connecting code may only be a stateless composition binding, never a shim with its own queue, lifecycle or mirrored state. |
| Similar-Issue Sweep | Audit every `vm_session*` queue, lifecycle state, run generation, monitor/presentation ingress, machine result sink, host-input pressed state, CMake target and test. Each must move to `common/session`, move to the named product/composition owner, or be deleted; no old session owner may survive. |

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
