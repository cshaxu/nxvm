# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S4 establishes the one `common/machine` executor bridge and paused-Debug boundary. |

## M5 T527 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T527 S4 on 2026-09-11 after S3 implementation commit `b8adc2b6`. The owner requires one correct executor boundary, not a compatibility route: `common/machine` owns the ordered copied request FIFO, NXVM `vm/machine` is its sole Core driver, and the old VM executor transport is removed in the same complete P. |
| Objective | Establish `common/machine` as the sole bounded safe-point request FIFO and opaque product-driver contract. Cut NXVM over through `vm/machine`, move its Core assembly and paused Debug endpoint to that driver, delete `vm/machine/executor_{queue,fifo}` and their production/test routes, and make paused Debug access use bounded copied operations whose validity ends before resume, reset or stop. |
| Non-goals | Do not migrate the Debug CLI/parser or provider registration (S5); create `common/ui`; change Core CPU/device/firmware/media/YAML behavior; alter native UI/host APIs; expose Core/VM/session/native pointers; add a forwarding facade, second executor queue, polling loop, artifact build or external asset use. |
| Reference Baseline | T527 S3 implementation at `b8adc2b6`; [route ledger](../etc/evidence/t527-s1-common-runtime-route-ledger.md), [S2 Debug contract inventory](../etc/evidence/t527-s2-debug-interaction-inventory.md), and [S3 cutover evidence](../etc/evidence/t527-s3-common-session-cutover.md) are binding. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), revised S3 cutover batch. |
| Files And ABI Surface | May add `src/common/machine/**`, owner-local tests and CMake registration; replace/delete `src/vm/machine/executor_{queue,fifo}.*`, their tests and CMake targets; modify `src/vm/machine/runtime/{machine,lifecycle,control,debug_target}.*`, `src/vm/machine/runtime/machine_private.h`, public VM machine interfaces and the minimal `vm/app` binding/callers; adjust architecture/source-layout/proposal/status/evidence documents. `src/lib`, Core behavior, profile semantics and native presenters are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, source/test accounting, actual-diff review and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per mutable queue, copied public values, opaque driver context and composition-only binding. [Coding](../rules/CODING.md): no forwarding compatibility, speculative framework, duplicate state or polling. [Documentation](../rules/DOCUMENT.md): packet/evidence/index consistency. |
| Verification | Add owner-local common-machine FIFO/order/overflow/close/wakeup/stale-run and paused-debug lease regressions; migrate applicable executor and Debug-route tests; build all affected targets; run the complete repository-only unit suite and relevant owner/source-layout/route gates, documentation governance and `git diff --check`. Integration and dual artifacts remain T527 S8 work because S4 does not alter profile/media/UX semantics. |
| Expected Markers | `common/machine` is the only executor FIFO and publishes only copied results; its driver owns an opaque context and safe-point consumption. NXVM has one `vm/machine` Core driver; no production or test `vm_machine_executor_*` route remains. Every paused Debug operation is rejected while not paused; the target owner invalidates its lease before resume/reset/stop. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Before P, record the old-to-new executor and Debug-route source/test accounting. Deliver one complete pushed P after direct cutover, full unit/gates and actual-diff self-review. Coordinator independently reviews before accepting S4; do not admit S5 without owner approval. |
| Stop Conditions | Stop and report before implementation if direct cutover needs a Core/VM/session/native pointer in common, a second executor route, copied machine-state mirror, a polling loop, an unbounded Debug operation, a native API, or changes Debug grammar/product behavior. |
| Exit Criteria | `common/machine` is independently buildable against lib public headers and the only NXVM executor FIFO; `vm/machine` is its sole opaque Core driver; old executor sources/targets/tests are deleted; paused Debug uses only bounded copied operations and invalid lease attempts fail; all required tests/gates pass and evidence accounts for the actual diff. |
| Original Owner Request | Owner approved the S4 bridge and requires a complete replacement, not a shim: the common queue has one owner, VM is only the Core adapter, and Debug never receives a raw Core/VM pointer. |
| Similar-Issue Sweep | Audit every `vm_machine_executor_*`, `vm_machine_request`, command-boundary callback, Core guest-input ingress, lifecycle enqueue, result sink, old `core_debug_target` consumer, CMake target and test. Each must become the named common-machine/VM-driver route or be deleted; no former executor transport survives. |

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
