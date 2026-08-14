# M5 T365: PC/AT NMI Source Ownership

## Task Record

T365 follows the T364 selected-PC/AT completeness audit. It may select and
implement one documented board NMI source only after its entire lifecycle and
delivery contract is shown; otherwise it retains a precise non-admission.

## Active Progress

### S1: Source-candidate and existing-path audit

S1 audits parity and I/O-channel-check candidates, CPU delivery, CMOS masking,
profile routes, reset and deterministic visibility before allocating a source.

S1 is accepted at `6b922ba1`. Its
[source decision](../etc/evidence/t365-s1-pcat-nmi-source-decision.md)
records that no candidate can be selected without fabricating a board contract.
S2 may only close the task's non-admission and must retain the receiver.

### S2: Non-admission closure audit

S2 is accepted at `febc9352` and closes T365. Its
[closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md)
confirms the sole T365/TODO receiver and prohibits using CMOS, RTC, PIC or CPU
delivery state as a synthetic board source. The selected profile must explicitly
admit a board producer before physical/cycle-exact L3 can close.
