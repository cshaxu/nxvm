# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Open | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S1 through S7 accepted; S8 VM/application migration remains queued. |

## M5 T527 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved the full T527 convergence program and instructed continued execution. S8 is the approved next proposal step after accepted S7; it completes NXVM product/app ownership without changing Lib or Core contracts. |
| Objective | Make `vm/product` the sole NXVM YAML/profile/CLI/debugger/presentation-policy owner and `vm/app` the sole composition root; retire `vm/events` and every remaining product/host bridge or duplicate route. |
| Non-goals | Do not alter Core machine/device/timing behavior, Lib APIs or implementation, Common component contracts/queues, native presenter behavior, profiles/assets, debugger grammar, recorder semantics, external integration scenarios or release artifacts. |
| Reference Baseline | Accepted S7 governance commit `aea11c79`; [M5 proposal](../proposals/m5-common-product-runtime-convergence.md) S8 and its ownership/data-flow sections; the current VM source/build graph. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), S8 NXVM product and app completion. |
| Files And ABI Surface | May move or delete `src/vm/events/**` and its owner-local tests, update `src/vm/app/**`, `src/vm/product/**`, `src/vm/machine/**`, CMake/test registrations and task evidence. Public Common and VM composition interfaces may be tightened only when the old bridge is deleted in the same P. `src/lib/**` and `src/core/**` are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, actual-diff review, code-size accounting and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per mutable state/queue, copied values at boundaries and no reversed dependencies. [Coding](../rules/CODING.md): remove forwarding compatibility and unnecessary layers. [Documentation](../rules/DOCUMENT.md): current/evidence consistency. [Product UX](../design/UI.md): preserve NXVM Console/debugger interaction. |
| Verification | Establish a pre-change route ledger for all `vm/events`, app-to-machine/product calls and Common fact/plan bridges. Add or migrate owner-local repository-only tests for product policy, app composition and copied machine facts; build affected targets; run complete repository-only unit suite, relevant static route/ownership gates, documentation governance and `git diff --check`. Integration and dual artifacts remain S9. |
| Expected Markers | `vm/product` alone parses NXVM commands/YAML/profile and chooses presentation policy/text; `vm/app` only composes Common session/machine/UI with NXVM adapters; `vm/machine` retains Core assembly/provider/execution/media/copied-Core-fact adaptation; `vm/events` and obsolete bridge headers/targets are absent. One session FIFO, one machine FIFO and one UI route remain. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Record the complete old-to-new route ledger, each deletion receiver, tracked source/test delta, focused proof and full verification. Deliver one pushed implementation P; coordinator then independently audits each exit condition before acceptance. |
| Stop Conditions | Stop and report before implementation if removal requires a Lib/Core change, moves NXVM product policy into Common, exposes a Core/native handle, introduces a second queue/lifecycle owner, requires compatibility forwarding, or changes visible CLI/debugger semantics beyond the named ownership migration. |
| Exit Criteria | All `vm/events` and duplicate bridge paths are deleted; each retained VM route has its named owner; product/app/machine direction matches the proposal; focused and full unit tests plus route/ownership/docs gates pass; evidence accounts for the actual diff and the worktree is clean after pushed P. |
| Original Owner Request | Complete the requested layout: Common is session, machine and UI; core/debug has become common/xasm32 plus common/debug; NXVM has no parallel product-runtime architecture. |
| Similar-Issue Sweep | Audit every VM event carrier, direct Common/Core include, Common fact/plan conversion, app queue/lifecycle field, product CLI/YAML/title/hotkey path, composition callback and CMake/test route. Delete or move each to its sole named owner; no forwarding facade remains. |

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

- **M5 T527 S7 P2:** coordinator review accepts implementation `bcb8346a`.
  `common/ui` is the sole generic Lib presenter binding; `vm/presentation` is
  retired; Core-display conversion remains NXVM `vm/machine`; NXVM policy,
  hotkeys, title and monitor text remain `vm/product`. Full x64 unit 298/298,
  specialized gates, documentation governance and source-boundary sweeps pass.
  [Evidence](../etc/evidence/t527-s7-common-ui-cutover.md)

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
