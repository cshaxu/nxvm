# M5 T346 S2: Deterministic Timeline Foundation

## Owner And Contract

`core_machine_timeline` is the sole core-owned due-event mechanism. It is a
fixed-capacity, allocation-free queue held by `core_machine`; it has no host
clock, thread, renderer, device-global state, or VM-facing mutable pointer.
An event is ordered by `(due_tick, sequence)`. Sequence is allocated once at
scheduling, so equal ticks are stable. Delivery removes an event before its
callback, allowing a callback to schedule another event at the current tick
without re-delivery. Tokens identify one slot/sequence pair; cancellation and
reset remove pending callbacks.

The machine advances the timeline exactly to its already-authoritative
`elapsed_ticks` after every completed CPU retirement and before the retained
L2 controller scheduler. Existing controller `advance()` behavior therefore
remains unchanged in S2; S3/S4 are the only receivers allowed to migrate a
controller onto due events.

## Checkpoint Boundary

`core_machine_trace_event` now includes copied `elapsed_ticks`. The trace owner
records the following deterministic checkpoints:

| Checkpoint | Existing or S2 owner | Meaning |
| --- | --- | --- |
| CPU retirement | `machine.c` | A completed instruction's linear PC and charged tick duration. |
| DMA advance | `machine.c` | The retained DMA-domain advance amount at the same machine tick. |
| Port read/write | `port_interface.c` | Existing port transaction result, now timestamped. |
| External memory read/write | `memory_interface.c` | Checked public memory transaction result and byte count, timestamped. |

This is a bounded observation/ordering surface, not a claim that all CPU
internal memory operands already have PC/AT wait-state timing. S3 owns DMA and
storage transaction migration; S4 owns input/display; device-specific service
duration remains unclaimed until those packets cite hardware evidence.

## Proof And Compatibility

`core_machine_timeline_s2_smoke` proves nondecreasing time, earlier due tick,
equal-tick sequence stability, callback reentrancy, cancellation, reset
removal, rejected past scheduling, copied observation, and one real NOP
retirement followed by the retained DMA checkpoint and run boundary. Its marker
is `M5:T346:S2:TIMELINE:OK`.

The established trace smoke now separates a machine fault trace from the
independent port-provider fault path: a faulted machine correctly rejects
ordinary bus operations until reset. That repair preserves the original port
trace assertions and removes a lifecycle-invalid test assumption rather than
changing production behavior.

The CMake T345 owner-test cohort count is 119 pure and 3 mixed targets after
the new owner smoke. The target receives the existing target-local strict GCC
options through that verified cohort; no global flag or linked-library claim is
introduced.

## Transfers

* S3 must bind PIC, PIT, RTC, DMA, FDC, and ATA readiness/IRQ/abort decisions
  to this timeline and transaction boundary.
* S4 must bind KBC/AUX and VADP/presentation visibility to it while preserving
  the host boundary.
* S5 must prove the selected machine corpus or transfer non-selected hardware.

Promotion/retirement: retain through T346 S5, then merge final timing evidence
into T346 history.
