# M5 T346: Core-Machine Device Parity And L3 Timing Convergence

## Task Record

T346 turns the completed CPU-profile program into a hardware composition
program.  It inventories and then closes, or precisely transfers, the
core-machine device, bus, port, memory-map, interrupt, DMA, and deterministic
timing gaps relevant to a PC/AT-class Windows 3.x research machine.  It
excludes numerical x87 execution.

## Active Plan

S1 establishes the source/evidence/reference ledger and L3 dependency graph.
No later device or timing implementation may bypass that ledger or make a
Bochs/PCjs implementation a project dependency.  Subsequent S units are
admitted from its owner/dependency results, not preallocated from convenient
port batches.

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
