# M5 T354: Physical L3 Bus-Timing Convergence

## Task Record

T354 follows T346's deterministic selected PC/AT event ordering and T347
through T353's controller/topology closures. It distinguishes existing
deterministic order from selected transaction availability and from unselected
physical fidelity. It must not use Windows readiness or guessed delays as a
timing oracle.

## Accepted Progress

S1 is accepted at `e9a259db`. Its
[transaction and timing ledger](../etc/evidence/t354-s1-l3-transaction-timing-ledger.md)
maps every selected CPU, DMA, controller, session and trace boundary. It
identifies one repeated construction: executing CPU and DMA paths directly
call memory/port owners without a shared availability-to-commit boundary,
while the existing `core_machine_bus_*` API is stopped/paused-only and is not
that owner. S2 receives the common mechanism decision, S3 receives admitted
DMA/CPU/service competition, and S4 receives reset/trace/consumer closure.
