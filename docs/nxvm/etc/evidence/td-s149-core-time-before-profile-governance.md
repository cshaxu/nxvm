# Td S149: Core Time Before Profile Governance

`M5:Td:S149:CORE-TIME-BEFORE-PROFILE:OK`

## Decision

The owner identified that a machine profile cannot correctly claim L3 timing
while the machine-neutral Core time axis and Core-to-VM boundary are incomplete.
The pre-execution T473 admission is therefore withdrawn without implementation;
its identifier and original proposal are retained in history and not reused.

## Resulting Order

1. The new queued Core unified guest-time-axis candidate establishes the sole
   Core timing/observation mechanism and immutable neutral plan.
2. PC/AT 5170 root normalization consumes that established contract, selecting
   IBM Model-339 source facts and explicit L2 inputs without adding Core logic.
3. DeskPro, YAML and later profile tasks remain downstream consumers.

T469--T472 remain historical baseline evidence. Their existing API is reused
or simplified by the predecessor; neither it nor any profile may create a
second time owner or host-generated guest-time route.
