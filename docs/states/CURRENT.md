# Project Status

## Current Work

| Task | Status | Scope |
| --- | --- | --- |
| T527 | Open | [M5 Common Product-Runtime Convergence](../proposals/m5-common-product-runtime-convergence.md): S1 route ledger accepted; later implementation S requires owner admission. |

## Current Technical Baseline

- `vm-0-5-0526` remains the current target. Its stripped Release developer
  artifacts are `nxvm_0_5_0526_x64.exe` and `nxvm_0_5_0526_x86.exe` until a
  later implementation S creates a replacement task target.
- `src/lib` remains byte-identical to the canonical SoftPC corpus at the
  reference baseline. T527 S1 did not modify it.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T526 | Canonical-library integration repair closed at `98ac51a4`: 299/299 unit, 67/67 specialized gates, 42/42 external integration, actual-diff review and stripped x64/x86 artifacts accepted. |

## Recent Governance

- **M5 T527 S1 P2:** coordinator review accepts `5c800dc3`. The route ledger
  records one owner and receiver for every named Core/VM/presentation route,
  fixes the three transport boundaries, and leaves `src/lib` unchanged.
