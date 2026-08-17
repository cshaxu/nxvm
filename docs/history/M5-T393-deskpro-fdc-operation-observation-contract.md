# M5 T393: DeskPro FDC Operation Observation Contract

## Task Record

T393 owns the bounded Core FDC operation-result observation prerequisite
transferred by T392. It preserves Core ownership and permits only a copied VM
consumer; it does not establish a C1 endpoint or physical qualification.

## Active Subtask

### S1: Publisher And Consumer Contract Inventory

S1 freezes all current FDC result publishers, reset/failure semantics and
possible Model-40 copied consumers. It selects an implementation contract only
if the exact publisher and consumer boundaries are finite.
[Evidence](../etc/evidence/t393-s1-fdc-publisher-consumer-inventory.md).

## Coordinator Acceptance

After actual-change review of P1 5baf0a8d, the coordinator accepts S1. The complete publisher batch preserves Core ownership, excludes reset/seek/status false positives, and selects only a copied Model-40 candidate. Focused capture regression and documentation governance pass; T393 remains open for bounded implementation.

### S2: Copied Terminal-Result Implementation

S2 publishes one Core-owned copied terminal record for data-transfer completion,
and Model 40 stores only its private copy. Reset invalidates the copy; focused
Core/Model-40 direct smoke proves success, DMA terminal and failure boundaries.
[Evidence](../etc/evidence/t393-s2-fdc-terminal-observation-implementation.md).

## Coordinator Acceptance

After actual-change review of P1 `50ff77aa`, the coordinator accepts S2. The
provider is frozen through generic FDC topology, Core has no VM dependency, and
no mutable FDC/session layout escapes. Direct Core and Model-40 FDC smokes plus
the current C1 capture regression and documentation governance pass. T393
remains open: only bounded candidate observation may decide whether a semantic
checkpoint exists.