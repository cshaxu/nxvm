# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 S8 | Closed | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): product/app ownership, machine-adjacent copied event contracts and one App-owned Common UI lifecycle accepted; S9 remains unadmitted. |

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
