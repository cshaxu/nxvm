# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Active | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S6 restores retained Debug/xasm32 behavior and replaces the temporary file callback with the sole neutral storage route. |

## M5 T527 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner accepted S5 migration and approved S6 on 2026-09-11 after the line-by-line audit of `aasm32`, `dasm32` and `command`. The owner requires retained xasm32 classifications, removal of dead/direct stdout paths, one direct `lib/storage` route for `N/L/W`, and retained `XSREG`/`XCREG` information and formatting. |
| Objective | Preserve original xasm32 and DOS Debug command behavior while retaining the accepted common/session/common-machine architecture: retain clear existing assembler classifications; delete dead trace/token/stdout remnants; make `N/L/W` use the sole direct neutral storage route; and regain original segment/control diagnostic output from one bounded copied paused-debug snapshot. |
| Non-goals | Do not create common UI; change CPU/device/firmware/media semantics; alter ordinary NXVM command grammar, YAML/profile policy, native UI behavior or host APIs; expose Core/VM/native/file handles; retain injected Debug file callbacks, a Debug-specific queue, nested raw-console reader, compatibility forwarder, polling loop, artifact build or external asset use. |
| Reference Baseline | Accepted S5 implementation `57c1d6d7`, original xasm32 source at `f187c310`, and original Debug command source at `4dbd98ec`; [S5 migration ledger](../etc/evidence/t527-s5-debug-migration-ledger.md) and the owner-approved audit disposition are binding. |
| Candidate Proposal | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md), including its VM-machine extraction table. |
| Files And ABI Surface | May modify `src/common/xasm32/**`, `src/common/debug/**`, `src/common/machine/**`, the VM common-machine driver mapping, their owner-local tests and CMake registrations; update proposal/status/evidence documents. Owner approved two limited Lib changes: make the existing storage writer binary-byte capable for Debug `W`; and extend public `lib/types` with neutral C-runtime/atomic vocabulary so Common has no old `type.h` or direct ISO-C dependency. Core CPU/device/firmware/media behavior, profile semantics and native presenters are excluded. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): one complete P, source/test accounting, actual-diff review and similar-route sweep. [Architecture](../rules/ARCHITECTURE.md): one owner per parser, snapshot and file path; copied values; no Core pointer in common and downward-only dependency. [Coding](../rules/CODING.md): no forwarding compatibility, duplicate command path, speculative callback framework or hidden host I/O. [Documentation](../rules/DOCUMENT.md): packet/evidence/index consistency. |
| Verification | Add classification xasm32 regressions, direct-storage `N/L/W` regression, and exact `XSREG`/`XCREG` output regression; build all affected targets; run complete repository-only unit suite, common dependency/source-layout gates, relevant Debug/provider/route gates, documentation governance and `git diff --check`. Integration and dual artifacts remain S9 work. |
| Expected Markers | `common/xasm32` retains its established `INVALID_ARGUMENT`/`UNSUPPORTED`/`LIMIT_EXCEEDED` classifications and no executable/direct stdout or dead trace/token route. `common/debug` has the only Debug grammar and direct sole `lib/storage` file route, with no file callback ABI. One common-machine copied paused snapshot supplies all original segment/control output; no Core direct print route returns. No legacy parser, target table, raw reader, second Debug FIFO or injected Debug file service survives. |
| Asset Needs | None. No external source, firmware, media, YAML asset, font, trace or binary is read, written or packaged. |
| Reporting Requirements | Record source/test accounting for every restored status, stdout/trace removal, file route and snapshot field. Deliver one complete pushed P after full unit/gates and actual-diff self-review; coordinator independently audits every exit condition before accepting S6. |
| Stop Conditions | Stop and report before implementation if preserving original output requires a Core/VM/session/native pointer in common, a second Debug queue/reader/path, an unbounded target operation, a host handle, a product/platform meaning in Lib, compatibility forwarding, or an unapproved grammar change. |
| Exit Criteria | xasm32 classifications remain explicit and stable; Common has no direct ISO-C or old `type.h` dependency; `common/xasm32` has no direct stdout/dead trace/token route; `N/L/W` use only direct `lib/storage`; `XSREG`/`XCREG` emit retained original information/format via copied bounded snapshot; focused regressions, full unit and all named gates pass; evidence accounts for actual diff and working tree is clean after pushed P. |
| Original Owner Request | Owner requires original behavior and source style over status-code cleanup: eliminate accidental semantic changes, remove useless direct-output residue, use one direct neutral storage owner, and restore all lost Debug diagnostic detail. |
| Similar-Issue Sweep | Audit all xasm32 status returns, trace macros and host-output calls; all Debug file callback/direct-file paths; and every former Core direct Debug print route. Each hit must be restored, deleted or mapped to the sole snapshot/storage owner; no duplicate or injected file route survives. |

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
