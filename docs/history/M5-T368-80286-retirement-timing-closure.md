# M5 T368: 80286 Successful-Retirement Timing Closure

## Task Record

T368 consumes T366's transferred successful-retirement timing work before the
PC/AT bus candidate can allocate availability.  It is a CPU ledger task; it
does not select a physical clock or close IBM 5170 model-L3.

## Active Progress

### S1: Ordered retirement-route inventory

S1 records the one post-refresh successful-retirement publisher and every
80286-reachable classifier layer before it.  Its
[route inventory](../etc/evidence/t368-s1-80286-retirement-route-inventory.md)
separates accepted string/port, primary and control-stack rows from the
remaining prefix, system-context and final-default fallback classes.  S2 must
prove reachability and authority by source/form/context before admitting a
new value; S1 makes no timing or L3 claim.
S1 is accepted at `06b0098c`.

### S2: System-context reachability reconciliation

S2 checks the accepted 80286 system rows against their actual successful
publisher route.  Its
[reconciliation evidence](../etc/evidence/t368-s2-80286-system-context-reconciliation.md)
repairs VERR/VERW's accidental SLDT/STR scalar route and stale LAR/SMSW
fixture assumptions.  It transfers the observed protected LMSW `#GP` before
retirement to S3 and retains successful prefix/default work for later T368
subtasks; no whole-ledger or L3 claim is made.

S2 is accepted at `5cf9f2df`; protected CPL0 LMSW is the immediate S3
semantic-and-retirement receiver.
