# M5 T462: Core Controller Board-Timing Contract

T462 establishes the one immutable board-timing contract through which a
profile may select validated PIC 8259A, DMA 8237A and PIT 8254 controller
timing/topology values. Core copies and validates those values before machine
publication; controller state and guest-time progression remain at their
existing owners.

The task consumes only the finite deferred batches from closed T456, T460 and
T461. An exact rational value derived from an L2 board assumption remains L2
board provenance even though it yields deterministic controller transitions.
Only a qualified source-backed value may support a board-L3 claim. L4
electrical recreation is excluded.

## Planned Subtasks

1. S1 freezes the controller exception ledger, field vocabulary, validation
   matrix and replacement/migration decision without changing runtime code.
2. S2 implements one copied construction-plan boundary and removes superseded
   input plumbing.
3. S3 binds the accepted PIT and DMA fields through their current owners with
   focused lifecycle proof.
4. S4 binds the accepted PIC field or preserves its explicit fallback, then
   audits provenance, controller order, artifact and complete ledger closure.

## Accepted Progress

### S1

Accepted at `e88124db`. The finite 11-row ledger establishes the existing
clock/transaction/route owners and rejects placeholder controller fields.

### S2

Accepted at `df6d28a7`. One construction-only copied rule value qualifies
existing PIT/DMA clocks and service phases, rejects invalid combinations and
retains PIC L2.

### S3

Accepted at `3c2d5c3d`. Model 339 selects its existing source-qualified PIT
and DMA values through the descriptor-to-copied-Core-plan path; default and
PIC remain L2. Focused profile/reset/disposition regressions and documentation
governance pass. [Evidence](../etc/evidence/t462-s3-controller-profile-selection.md).
