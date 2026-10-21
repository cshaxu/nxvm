# T7 S52 COP Command-Window Read Contract

## Decision And Boundary

The existing COP provider claimed the command window for both reads and writes,
but rejected reads after query admission.  The CPU correctly converted that
provider failure into its internal case-error stop.  S52 repairs the
contradiction at its only owner: `myraidenii_core_cop_read` now returns zero
bytes for the admitted command range.

This is a local **L2** deterministic interoperability model.  The inspected
external Raiden II map snapshot, `raiden2.cpp` SHA-256
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`, declares
the range as write-only; it does not establish a physical open-bus value.
Zero is consequently not presented as a hardware measurement.  It prevents an
internally contradictory accepted read from becoming a fatal transport error
without inventing a latch, a second route, or a pending-command side effect.

## Retained Ownership

- COP remains the sole query/read/write provider for its command range.
- Command writes, command classification, pending operation and failure
  observation are unchanged.
- Board fallback, X86 Core CPU fault transport, Shared components, ROM/media
  handling and host presentation are unchanged.

## Verification

The existing Board smoke now proves direct read and inspect values at two
command slots, keeps the failure observation clear, and executes a V30 memory
read followed by `HLT` through the production provider path.  It also retains
the prior unsupported command-write and reset checks.

Focused `myraidenii.core/board-bus-smoke` passed on both stable Ninja widths.
The complete MyRaidenII unit label passed 12/12 on x64 and 12/12 on x86.
The existing bounded selected-set wrapper ran once per width with its existing
Coin/Start frame sequence and 100,000,000-instruction containment budget.
Both widths reached the same neutral `FAULT` category and wrapper staging was
empty after each run.  This proves the former command-window provider failure
is no longer the reported qualification boundary only to the extent that the
existing probe can classify it; it does not establish title, credit acceptance,
gameplay, graphics, audio, desktop/RDP behavior or release readiness.

Documentation governance and whitespace checks passed.  No executable is
deployed: this bounded Core repair is not an admitted product-release boundary,
and `assets/myraidenii` has no prior M1 release pair to replace.
