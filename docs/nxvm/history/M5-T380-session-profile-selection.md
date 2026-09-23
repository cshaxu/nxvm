# M5 T380: NXVM Session Profile Selection

## Task Record

T380 is the owner-approved product-entry insertion immediately after the
closed 5170 Model-339 L3 audit. S1 exposes existing VM profile descriptors in
the retained Console. Its later S2 corrective startup recovery changes the
strict Model-339 profile/firmware composition at the documented FDC, BDA,
keyboard, and absent-memory boundaries; it neither makes nor preserves an L3
decision for that changed runnable path.

## Active Progress

### S1: Session profile chooser and named selection

S1 owns the Console-facing chooser, explicit profile parse path, selected
profile visibility and their regressions.  It must preserve the generic
default session and reject generic configuration overrides for Model-339.

The implementation and proof are recorded in
[T380 S1 evidence](../etc/evidence/t380-s1-session-profile-selection.md).

### S2: Model-339 strict-start recovery

S2 corrects the strict 512 KB Model-339 startup path: profile-owned FDC bounce
placement and BDA base-memory publication, conventional-media FDC handling,
keyboard translation, and the bounded absent-memory provider reached after
A20. It is a runtime/profile correction, not a replacement L3 audit. The
implementation and proof are recorded in
[T380 S2 evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md).

## Closure Audit

P1 `f00478fb` implements the VM-owned menu, direct profile parse path and
truthful selected-session output.  Coordinator review found that the Console
smoke showed selection but did not assert it; P2 `987bdd96` adds that
assertion. P3 `9c74c34a` accepts the S1 selection work. Corrective S2 P1
`4bd5e2c4` repairs the strict Model-339 startup path described above and
records its focused/full-gate proof. No ROM or media enters the repository;
the later current-source Model-339 L3 re-audit is the only receiver for a
post-S2 readiness decision. T380 therefore closes and its proposal is retained
beside this record.
