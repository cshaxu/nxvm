# M5 T392: DeskPro 386 C1 Semantic Endpoint Investigation

## Task Record

T392 is the narrow successor to T391. It investigates the complete finite set
of existing-state C1 semantic candidates or one earliest shared functional
defect; it does not reopen T391 or bypass the C1 physical-qualification block.

## Active Subtask

S1 is active: inventory every existing copied-state candidate and determine
whether any can become a finite semantic checkpoint without an interface
expansion or prohibited retained data.
### S1: Existing-State C1 Candidate Inventory

S1 exhausts the seven copied-state C1 candidate classes. It finds neither an
admissible finite endpoint nor a reproducible earliest-owner defect; device and
session milestones need a separate observability design. [Evidence](../etc/evidence/t392-s1-c1-existing-state-candidate-inventory.md).
## Coordinator Acceptance

After actual-change review of P1 `5b760ef1`, the coordinator accepts S1. The
complete seven-candidate inventory preserves the existing copied-state and
Core/VM boundary, identifies no endpoint or shared defect, and gives every
candidate a receiver. The focused C0/C0A/C1 capture regression and
documentation governance pass. T392 remains open; the next receiver is a
bounded device/session observability design decision.

### S2: VM And Session Observability Decision

S2 exhausts existing VM/session state, rejects mutable media/topology/display/
fault facts as C1 success, and transfers a future immutable VM-owned operation
snapshot design without changing Core or VM ABI. [Evidence](../etc/evidence/t392-s2-vm-session-observability-decision.md).
## Coordinator Acceptance

After actual-change review of P1 `3319d1b4`, the coordinator accepts S2. The
matrix disposes every existing VM/session state without exposing mutable layout
or changing Core/VM ABI. Focused C0/C0A/C1 capture regression and documentation
governance pass. T392 remains open; any next step requires a separately
admitted selected device-operation contract.
