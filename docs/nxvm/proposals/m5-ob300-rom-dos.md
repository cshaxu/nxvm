# M5 Candidate: OmniBook 300: real firmware and ROM DOS

Owner queued this bounded T candidate on 2026-11-19. Numeric T/S identifiers
are allocated only at admission. [Queue](../states/QUEUE.md) owns priority;
[family research and shared boundaries](m5-omnibook-emulator.md) apply.

## Objective and scope

Assemble the proven OB300 board as a Machine with reset mapping, original
System ROM card and matching writable storage. Keep card/media lifetimes in
the existing Machine construction and teardown path.

Execute original reset, POST and ROM DOS. Provide the writable C-drive medium
needed by the original software; retain the immutable card master. Validate
card bank/window changes and failure behavior, not merely extracted files.
Measure boot stages, execution throughput and input latency from this first
working guest, retaining raw-work counts separately from wall time.

## Dependencies

Accepted [board wiring](m5-ob300-board.md), hardware contracts and T588 ledger.

## Non-goals

No host-side DOS/FFS/Windows replacement, arbitrary ROM patch or full-family claim.

## Completion and stop conditions

Both host widths reach an interactive ROM DOS checkpoint through the real
firmware, with usable display/input, directory access and writable storage.
Reset/resume and stop/start repeat successfully. The ledger distinguishes
completed boot requirements from Windows/peripheral receivers. Record current
performance against the frozen budgets and transfer any shortfall explicitly
to the next performance task; do not call a static POST screen a DOS boot.

At admission, plan bounded numeric S batches from the convergence ledger rather
than one S per symptom. Reuse existing tests where they cover the contract.
Missing manuals alone are not an indefinite blocker: document L1 assumptions,
L2 reference models or L3 explicit manual formulas/constants honestly. Public
interface changes follow the applicable review rules. New evidence that
invalidates a prerequisite requires a recorded design/scope decision, not a
second execution route or an unreported transfer.

## T588 handoff

Consume the [OB300 design](m5-ob300-architecture.md), including its O01-O12
owner/proof table, external System Card contract and numeric usability gates.
The reset experiment reaches the first FBh write only; no hardware, DOS or
Windows completion is inherited from design closure. Reconcile the complete
receiving batch before selecting fixes, and reuse existing shared tests.
