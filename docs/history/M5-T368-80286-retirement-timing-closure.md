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

### S3: LMSW context reconciliation

S3 shows that the protected-LMSW `#GP` originated in a reused fixture: its
prior real-mode direct form had set CR0.PE, while the subsequent bootstrap only
reset segments and EIP.  The bootstrap now establishes CR0 as real-address
state before loading the GDT and entering protected CPL0.  The existing
Appendix-B register/memory `3/6` rows then retire; the executor's CPL check was
not changed.  S3 is accepted at `e6b78987`; prefix/default successful routes
remain the next T368 CPU receiver, and no 5170 L3 claim is made.

### S4: Prefix and default reachability

S4 reduces the remaining one-tick possibilities to three finite receivers:
successful residual 80286 system forms, legal prefixes that earlier timing
owners do not own, and x87/WAIT/default or negative boundaries.  The static
T366 S10 publisher inventory still passes.  S4 is accepted at `7dc3a9d9`;
S5--S7 must resolve these receivers before T368 may close, and no 5170 L3
claim is made.

### S5: System-form timing reconciliation

S5 reconciles Intel 210498-005 Appendix-B rows with successful executions for
the remaining unprefixed system forms: `LLDT/LTR` register/memory `17/19`,
`SGDT/LGDT` memory `11`, `SIDT/LIDT` memory `12`, and `CLTS` `2`. The
[S5 evidence](../etc/evidence/t368-s5-80286-system-timing-reconciliation.md)
records protected CPL0 selector fixtures, table/control post-state proof, the
sole-publisher gate, and the focused semantic regressions. Fault, delivery,
prefix, bus, device and physical-clock paths remain unallocated. S5 is
accepted at `0e095fb2`; S6/S7 remain necessary before T368 can close, and
there is no 5170 L3 claim.

### S6: Legal-prefix reachability

S6 confirms that defined string/REP and already-admitted segment forms retain
their source owners, while other successfully retired prefix contexts have no
independent Intel 80286 CPU scalar. Its
[prefix evidence](../etc/evidence/t368-s6-80286-prefix-reachability.md)
retains them as one explicit bus/phase transfer rather than treating a prefix
as a one-tick or physical-clock surcharge. S6 is accepted at `1366fcc2`; S7
must still reconcile x87/WAIT/default and negative paths before T368 can
close, and there is no 5170 L3 claim.
