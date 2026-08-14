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

S2 was accepted at `406471d3`. It creates a core-owned
synchronous CPU/DMA transaction owner around selected checked physical-memory
and CPU-port paths, with begin/commit/cancel/reset trace semantics. It adds no
duration or instruction continuation: the executor remains atomic. The
detailed [S2 transaction-owner evidence](../etc/evidence/t354-s2-transaction-owner.md)
transfers deterministic competing-owner policy to S3 and session/consumer
closure to S4.

S3 was accepted at `8d29e836`.  It reconciles existing selected
same-guest-tick ordering without inventing physical duration: CPU transaction
commit precedes retirement; arbitration then services DMA, PIT, and PIC; and
the equal-due readiness callback advances/refreshes FDC then HDC.  The focused
[competition evidence](../etc/evidence/t354-s3-competition.md) proves that
ordering and transfers reset, trace-consumer, and final selected-L3 closure to
S4.

S4 is delivered pending acceptance.  Its
[selected-L3 closure audit](../etc/evidence/t354-s4-selected-l3-closure.md)
proves transaction cancellation before reset, trace continuity after reset,
and a fresh post-reset CPU commit/retire sequence.  It reconciles retained
firmware/DOS consumers without treating them as a timing oracle, and records
all unmodeled physical timing and Windows readiness as explicit transfers.
