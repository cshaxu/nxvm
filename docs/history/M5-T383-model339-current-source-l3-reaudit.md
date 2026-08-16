# M5 T383: IBM PC/AT 5170 Model-339 Current-Source L3 Re-Audit

## Task Record

T383 independently replays the frozen IBM PC/AT 5170 Model 339/Type 3
deterministic-L3 contract against the current runnable source graph. It is the
required successor to the historical T379 decision because T380 changed the
strict selected-profile start path. It is an audit-only task: any failed or
missing selected row transfers to its earliest owning implementation task.
The retained [candidate proposal](M5-T383-model339-current-source-l3-reaudit-proposal.md)
defines its original bounded admission context.

## Active Progress

### S1: Current-source reconciliation

S1 is admitted to compare the T379 acceptance baseline with the current graph,
replay its selected CPU, memory, firmware, FDC, keyboard, CGA, NMI and
deterministic-order evidence, and make one fresh ready/not-ready decision.
The independent result is [T383 S1](../etc/evidence/t383-s1-model339-current-source-reaudit.md):
the selected baseline remains ready under the same bounded deterministic L3
contract.

## Closure Audit

Coordinator review inspected P1 `6da7b9ca` and its actual packet, evidence,
history and supporting-index changes against the approved audit-only scope. It
contains no runtime source, ABI, asset, ROM or guest-media modification. The
current-source matrix covers every selected Model-339 row and each post-T379
change; the clean current gate passed 251/251, the focused replay passed 10/10,
and the specialized verification suite passed. T383 therefore closes with the
current ready decision retained in its S1 evidence. The next Queue candidate is
the independent DeskPro 386 Model 40 profile and capability-gap audit.
