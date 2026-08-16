# M5 T383: IBM PC/AT 5170 Model-339 Current-Source L3 Re-Audit

## Task Record

T383 independently replays the frozen IBM PC/AT 5170 Model 339/Type 3
deterministic-L3 contract against the current runnable source graph. It is the
required successor to the historical T379 decision because T380 changed the
strict selected-profile start path. It is an audit-only task: any failed or
missing selected row transfers to its earliest owning implementation task.

## Active Progress

### S1: Current-source reconciliation

S1 is admitted to compare the T379 acceptance baseline with the current graph,
replay its selected CPU, memory, firmware, FDC, keyboard, CGA, NMI and
deterministic-order evidence, and make one fresh ready/not-ready decision.
The independent result is [T383 S1](../etc/evidence/t383-s1-model339-current-source-reaudit.md):
the selected baseline remains ready under the same bounded deterministic L3
contract. Task-level closure review remains pending.
