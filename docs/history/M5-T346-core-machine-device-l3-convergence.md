# M5 T346: Core-Machine Device Parity And L3 Timing Convergence

## Task Record

T346 turns the completed CPU-profile program into a hardware composition
program.  It inventories and then closes, or precisely transfers, the
core-machine device, bus, port, memory-map, interrupt, DMA, and deterministic
timing gaps relevant to a PC/AT-class Windows 3.x research machine.  It
excludes numerical x87 execution.

## Active Plan

S1 established the source/evidence/reference ledger and L3 dependency graph.
S2 supplied its deterministic time/event foundation without migrating a
controller. No later device or timing implementation may bypass the ledger or
make a Bochs/PCjs implementation a project dependency. Subsequent S units are
admitted from the recorded owner/dependency results, not preallocated from
convenient port batches.

## Completed Subtasks

### S1 - Whole-Machine Capability, Reference, And L3 Audit

Accepted the indexed whole-machine ledger at
`docs/etc/evidence/t346-s1-core-machine-device-l3-audit.md`. It measured the
current deterministic, host-clock-independent L2 scheduler and identified the
required L3 foundation: one due-event/arbitration owner plus bounded memory,
I/O, and DMA transaction visibility. It allocated S2 before all controller
expansion, S3/S4 as dependent device migrations, and S5 as the reconciliation
handoff to the Windows readiness map. It refreshed stale capability wording,
preserved unselected device debt in TODO, and excluded x87 execution and source
import.

### S2 - Deterministic Time, Event, And Bus Foundation

Accepted the implementation and evidence at
`docs/etc/evidence/t346-s2-deterministic-timeline-foundation.md`. The core
machine now owns one allocation-free deterministic due-event timeline ordered
by `(due_tick, sequence)`, with cancellation and cold-reset removal. Copied
trace checkpoints cover CPU retirement, retained DMA advancement, checked
external memory transactions, and existing port transactions at machine time.
S2 deliberately retains the L2 controller scheduler and assigns every
controller migration to S3--S5; it does not claim device-specific L3 service
timing or host-time behavior.
