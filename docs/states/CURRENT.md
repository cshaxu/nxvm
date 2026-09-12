# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S7 moves the sole generic Lib UX binding into `common/ui` and deletes `vm/presentation`. |

## M5 T527 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T527 S7 on 2026-09-11 after accepting S6. The approved scope is one generic `common/ui` Lib-presenter binding, an NXVM product policy/Console binding, and complete retirement of `vm/presentation`; no product-specific UI policy moves into Common. |
| Objective | Make `common/ui` the sole owner of generic copied-frame publication, Lib presenter target/title/mouse operations, native input/focus facts and Console/Window lifecycle binding. Replace all NXVM `vm/presentation` production and test paths in the same P, leaving `vm/product` as the owner of raw-VM/monitor/none policy and Console text. |
| Non-goals | Do not change Core display/mailbox semantics, guest rendering, CPU/device/firmware/media/YAML behavior, host/Lib APIs, session lifecycle policy, Debug grammar, multi-session policy, external assets, release artifacts or introduce a UI queue, native handle, Core/VM pointer, product text or platform API into Common. |
| Reference Baseline | Accepted S6 governance commit `75285511`; [M5 proposal](../proposals/m5-common-product-runtime-convergence.md) S7, ownership/data-flow and public-common-contract sections; current `vm/presentation` production/test graph. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), S7 common UI binding. |
| Files And ABI Surface | May add `src/common/ui/**` and owner-local tests/CMake registration; move/replace/delete `src/vm/presentation/**` and its tests; update `src/vm/product/**`, `src/vm/app/**`, minimal `src/vm/machine` display adaptation and build registrations solely to bind the new owner. Update task/evidence documents. `src/lib/**`, Core behavior, profile semantics and native presenter implementation are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): complete P, source/test accounting, actual-diff review and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per UI route, copied public values, composition-only integration and no platform leakage. [Coding](../rules/CODING.md): no forwarding compatibility, mirrored presentation state, duplicate Console broker or speculative framework. [Documentation](../rules/DOCUMENT.md): packet/evidence/index consistency. [Product UX](../design/UI.md): guest interaction and separate monitor Console remain distinct. |
| Verification | Add common-UI owner tests for Console/Window target switch, copied frame publication, title, mouse capture/release, close/focus input and plan application; migrate existing presentation tests; build affected targets; run complete repository-only unit suite, common/source-layout and route gates, documentation governance and `git diff --check`. Integration and dual artifacts remain S9 work. |
| Expected Markers | `common/ui` depends only on Lib public interfaces and owns one generic presenter binding with copied plans/facts. `vm/product` supplies policy and Console text through its product binding; `vm/app` only composes. No production/test `vm_presentation_*` route, duplicate Console broker, direct native UI call or second target/frame/mouse state survives outside the named owner. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Record old-to-new UI route and source/test accounting, state ownership and all retained live paths. Deliver one complete pushed P after focused proof, full unit/gates and actual-diff self-review; coordinator independently audits every exit condition before accepting S7. |
| Stop Conditions | Stop and report before implementation if the direct replacement requires product policy in Common, a Core/VM/native pointer or platform API in a Common public interface, a second Console broker, an added queue/lifecycle owner, a Lib API change, a compatibility forwarder, or a behavior change not covered by the owner-approved UI contract. |
| Exit Criteria | `common/ui` is independently buildable against Lib public headers and the sole generic presenter binding; NXVM policy/text remain in `vm/product`; `vm/presentation` and its tests/targets are deleted; all UI facts/operations are copied values; focused regressions, full unit and named gates pass; evidence accounts for the actual diff and working tree is clean after pushed P. |
| Original Owner Request | Owner requires the stated architecture: `common = session + machine + ui`, with no second NXVM-only generic presentation path. |
| Similar-Issue Sweep | Audit every `vm_presentation_*`, Console-host claim/release, presenter target/title/mouse/frame call, native UI include, UI input sink and presentation CMake/test target. Each must become the named Common UI or product policy route, or be deleted; no forwarding facade survives. |

## Current Technical Baseline

- `vm-0-5-0526` remains the current target. Its stripped Release developer
  artifacts are `nxvm_0_5_0526_x64.exe` and `nxvm_0_5_0526_x86.exe` until a
  later implementation S creates a replacement task target.
- `src/lib` remains the canonical shared corpus. T527 S6 has two
  owner-approved changes: the existing storage writer accepts opaque bytes for
  Debug `W`; and `types` provides the neutral C-runtime/atomic vocabulary used
  by Common. Neither adds a product-specific route or platform API.

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

- **M5 T527 S6 P2:** coordinator actual-diff review accepts `75285511`.
  `common/debug/command.c` remains the sole table-driven DOS Debug grammar;
  `N/L/W` use the sole public `lib/storage` route; and `XSREG`/`XCREG` receive
  one bounded copied paused-debug snapshot through Common and VM adapters.
  The approved Lib extensions are neutral C-runtime vocabulary and opaque-byte
  writer input only. Full x64 unit 297/297, standalone strict Lib build and
  every named boundary/documentation gate passed. [Evidence](../etc/evidence/t527-s6-debug-source-preservation-correction.md)
