# M5 T428: Generic-AT Prefetch Reservation

## Outcome

T428 S1 adds a Model-40-selected, Core-owned generic-AT prefetch reservation.
It records a bounded sequential identity after successful real-mode instruction
execution and releases it only at shared arbitration after refresh, DMA, HOLD
and transaction ownership have priority. IBM 5170 Model 339 remains disabled.
The accepted implementation is `6fa58c3d`.

## Evidence And Verification

The retained [S1 evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md)
records the generic-at tier, lifecycle, cancellation paths, focused regression,
serial current-gate, documentation governance and diff hygiene result.

## Closure And Transfer

The reservation deliberately does not fetch memory, open a second CPU
transaction, emit an external-cycle phase or overlap declaration, deliver an
exception, or publish timing. It therefore closes only the safe state and
arbitration prerequisite. A source-backed asynchronous producer, D4 page-hit
qualification, BWAIT/ISA availability binding, PAL phase model, calibrated
clock duration and DeskPro physical/L3 acceptance remain in the physical-cycle
proposal and Queue.