# M5 Candidate: OmniBook 300: execution and interaction performance

Owner queued this bounded T candidate on 2026-11-19. Numeric T/S identifiers
are allocated only at admission. [Queue](../states/QUEUE.md) owns priority;
[family research and shared boundaries](m5-omnibook-emulator.md) apply.

## Objective and scope

Profile the actual OB300 workload, separating CPU execution, timing and
observation, memory routing, device scheduling, video, host pacing and firmware
polling. Diagnose missing-device waits before optimizing instructions.
Use one existing execution/clock path. Prefer reductions in duplicate work,
production code or algorithmic cost without material regressions. Keep neutral
optimizations shared and HP-specific hardware fixes local.

Do not introduce an OmniBook fast path, skip guest events, force timeouts to
expire or silently downgrade timing accuracy. Evaluate complete guest state
and device outcomes, not just an instruction-loop speed number. If the existing
implementation already meets the budgets, deliver that evidence without
manufacturing an optimization. Keep this finite: rank measured hotspots and
review the expected payoff before adding further implementation batches.

## Dependencies

Working real ROM DOS, [product assembly](m5-ob300-product-qualification.md)
and the design-stage measurement/budget contract. T591 S4 transfers
the measured startup and directory latency misses. Start with the
[controlled baseline](../etc/evidence/t591-s4-ob300-dos-baseline.md)
and preserve its comparison method and thresholds.

## Non-goals

No unlimited optimization campaign, JIT prerequisite or performance claim inferred from PC110.

## Completion and stop conditions

Demonstrate both-width cold boot, DOS command latency and sustained input/
redraw against the predeclared budgets, plus regressions for affected shared
consumers. If budgets remain unmet, present the measured limit and require an
explicit scope/budget decision; do not silently declare usability or proceed
to full Windows qualification. Any later Windows hotspot returns to its real
owner using the same measurement rules.

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
