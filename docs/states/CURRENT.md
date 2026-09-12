# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 S8 | Accepted | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): product/app ownership, machine-adjacent copied event contracts and one App-owned Common UI lifecycle accepted. |

## M5 T527 S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T527's complete Common-runtime convergence and instructed execution through its proposal exit criteria. S9 is the next approved bounded closure step after accepted S8. |
| Objective | Prove the final reusable Common corpus: independently build and verify it only through public Lib contracts; add neutral multi-adapter conformance proof for the session/machine/UI contracts; then complete T527's full unit, external integration, route/ownership, documentation and dual-artifact closure. |
| Non-goals | Do not change Core semantics, NXVM product UX/CLI/debugger grammar, Common/Lib public contracts, profiles, external assets or firmware/media contents. Do not import SoftPC code or create a second product/runtime path. |
| Reference Baseline | Accepted S8 governance `256efdd1`; [M5 proposal](../proposals/m5-common-product-runtime-convergence.md) S9 and task exit criteria; current Lib manifest/build graph and Common source/test corpus. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), S9 reusable-corpus and closure audit. |
| Files And ABI Surface | May add or tighten repository-only `test/common/**`, Lib/Common standalone build/manifest verification and T527 evidence/current/history records. Product/Core/Common production interfaces are read-only unless a demonstrated conformance defect requires a packet revision. Artifacts may be generated only by the approved configured build/deployment path. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): complete coverage/closure evidence, full unit per S, full integration and dual artifacts for T closure. [Architecture](../rules/ARCHITECTURE.md): Common uses only declared neutral contracts; no product adapter or second owner. [Coding](../rules/CODING.md): conformance doubles are test-only and cannot mirror production state. [Documentation](../rules/DOCUMENT.md): final status/history and evidence consistency. [Source policy](../etc/operations/policy/source-policy.md): external integration assets are read-only, unmodified inputs. |
| Verification | Establish a complete S9 corpus ledger: exact Common tracked files, public Lib include surface, manifest digest and forbidden-vocabulary scan; build the standalone corpus and run two neutral adapter conformance configurations. Build x64 and x86 Release; run all repository-only unit tests, full external integration suite, specialized/ownership/DAG/docs gates and diff review; verify deployed stripped artifacts and hashes in both required locations. |
| Expected Markers | `common/{ui,session,machine}` and `common/{xasm32,debug}` build without NXVM/product/Core/native headers; each neutral runtime contract has two test adapter implementations through its public ABI; no retained VM parallel route exists; one verified x64 and x86 task artifact pair is present in `build/output` and `assets/sessions`. |
| Asset Needs | Existing external ROM/CMOS/media assets are read only for the retained integration suite. No asset is copied, altered, committed or newly sourced. |
| Reporting Requirements | Record the complete Common corpus revision/digest, every public include and forbidden-vocabulary result, both adapter conformance paths, exact unit/integration/gate/artifact results, code/test delta and any residual transfer. Close T527 only if every proposal exit criterion is directly evidenced. |
| Stop Conditions | Stop and report before changing a public Lib/Common API, importing SoftPC/third-party content, changing product-visible behavior, modifying external assets, treating an NXVM-specific test double as neutral, or claiming byte-identical SoftPC adoption without a separately verified SoftPC checkout. |
| Exit Criteria | Standalone Common proof and two neutral adapter conformance paths pass; all T527 architecture/route/manifest assertions hold; full unit and integration suites are green; stripped x64/x86 artifacts are deployed and hashed; evidence/history records map every T527 exit criterion; no active packet, unresolved receiver or dirty worktree remains after governance closure. |
| Original Owner Request | Finish the requested refactor: Core Debug becomes reusable `common/xasm32` plus `common/debug`; Common is exactly session, machine and UI; NXVM has no duplicate product-runtime architecture and can later share the corpus with SoftPC. |
| Similar-Issue Sweep | Audit every Common source/header/CMake/test inclusion, all public Lib-only build entries, every Common runtime adapter implementation, every NXVM/Core/product/native token in Common, each product/VM legacy route, both target architectures and all external integration registrations. Treat every duplicate or forbidden occurrence as one batch with a named disposition. |

## Current Technical Baseline

- `vm-0-5-0527` is the current target. S9 will publish its stripped Release
  developer artifacts as `nxvm_0_5_0527_x64.exe` and `nxvm_0_5_0527_x86.exe`.
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

- **M5 T527 S8 P3:** coordinator audit accepts implementation `a8d17a39`
  and ownership correction `1a6bb158`. `vm/product` is the sole NXVM
  YAML/profile/CLI/Debugger/input-policy owner; `vm/app` composes and owns one
  Common session, machine and UI; the old `vm/events` root is gone. Focused
  product/Model-40 smoke tests, full x64 unit **298/298**, specialized gates,
  DAG/source sweeps, documentation governance and diff hygiene pass.
  [Evidence](../etc/evidence/t527-s8-product-app-cutover.md)

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
