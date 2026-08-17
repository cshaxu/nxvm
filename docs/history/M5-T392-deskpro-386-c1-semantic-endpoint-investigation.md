# M5 T392: DeskPro 386 C1 Semantic Endpoint Investigation

## Task Record

T392 is the narrow successor to T391. It investigates the complete finite set
of existing-state C1 semantic candidates or one earliest shared functional
defect; it does not reopen T391 or bypass the C1 physical-qualification block.

## Active Subtask

T392 is closed by S4 after S1-S3 inventory and decision evidence. S1 originally inventoried every existing copied-state candidate and determined
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

### S3: FDC Operation Decision

S3 verifies the generic Core FDC has operation-completion behavior but no safe
published snapshot or selected firmware-consumer contract. It transfers the
whole mechanism rather than exposing private FDC state. [Evidence](../etc/evidence/t392-s3-fdc-operation-decision.md).
## Coordinator Acceptance

After actual-change review of P1 `4c79473c`, the coordinator accepts S3. The
review confirms the FDC decision is based on current project-owned completion
paths, preserves the Core/VM boundary, and does not import reference code or
claim a firmware/CPU/physical result. Focused C1 capture regression and
documentation governance pass. T392 remains open; the generic operation-
snapshot design is its sole next receiver.

### S4: C1 Semantic Endpoint Closure Audit

S4 closes T392 with complete C1/VM/FDC transfer and no endpoint or CPU defect.
The sole future receiver is a separately admitted generic immutable device-
operation snapshot plus selected VM consumer contract. [Evidence](../etc/evidence/t392-s4-c1-semantic-endpoint-closure-audit.md).
## Coordinator Acceptance

After actual-change review of P1 `750de4e8`, the coordinator accepts S4 and
closes T392. The audit exhausts all C1, VM/session and FDC candidates, preserves
Core/VM ownership and transfers only the unimplemented generic immutable device-
operation snapshot plus selected consumer contract. Focused C1 regression and
documentation governance pass.
