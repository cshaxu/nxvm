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

### S4: C0A-Preconditioned READ DATA Diagnostic

S4 adds a test-capture-only, opt-in observation. C0A is the immediate
post-return `IN AL, 61h` retirement; its copied FDC-terminal sequence becomes
the lower bound. Only a later successful drive-0 `READ DATA` copied result is
reported. The one-retirement budget makes ambiguous same-boundary ordering a
non-success. Synthetic success/failure/reset coverage and focused FDC/capture
smokes prove the bounded diagnostic without changing a Core or VM production
interface.
[Evidence](../etc/evidence/t393-s4-fdc-read-data-c0a-diagnostic.md).

## Coordinator Acceptance

After actual-change review of P1 `b01cf455`, the coordinator accepts S4. The
capture reads only the existing Model-40 private copied result and adds no
Core/VM production interface. It accepts only a successful drive-0 `READ DATA`
record whose sequence advances after the C0A `IN AL, 61h` baseline; baseline,
wrong-drive, failed and reset-invalid records are rejected. Synthetic and
focused FDC/capture regressions pass. One owner-managed BYOB run reaches its
2,000,000-retirement containment limit with no unallocated timing but no
later successful `READ DATA`; this is bounded non-success, not C1 or physical
qualification. T393 remains open only for a separately admitted decision on
whether any further finite observation receiver exists.

### S5: Task Closure Audit

S5 reconciles every frozen FDC publisher disposition, the copied Core/VM
contract and the bounded C0A consumer result. The BYOB non-success supplies no
C1 endpoint or physical conclusion; it completes the selected finite consumer
disposition. The task may close only after P1 actual-change review archives the
proposal and removes its Queue entry.
[Evidence](../etc/evidence/t393-s5-task-closure-audit.md).
## Task Closure Acceptance

After actual-change review of P1 `f15164f1`, the coordinator accepts S5 and
closes T393. The P1 changed only the active packet, durable evidence, history
and supporting index; its complete publisher/contract/consumer reconciliation
matches the task completion standard. P2 archives the retained proposal beside
this history record and removes its Queue entry. Focused FDC/capture
regressions, direct reference sweep, diff hygiene and documentation governance
pass. The next ordered receiver remains shared 80386 physical-retirement
qualification; no C1, CPU physical-time, DeskPro board, firmware or L3 gate
opens from this closure.