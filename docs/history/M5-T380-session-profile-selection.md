# M5 T380: NXVM Session Profile Selection

## Task Record

T380 is the owner-approved product-entry insertion immediately after the
closed 5170 Model-339 L3 audit.  It exposes existing VM profile descriptors in
the retained Console without changing a descriptor, its hardware contract, or
the M5 baseline-machine sequence.

## Active Progress

### S1: Session profile chooser and named selection

S1 owns the Console-facing chooser, explicit profile parse path, selected
profile visibility and their regressions.  It must preserve the generic
default session and reject generic configuration overrides for Model-339.

The implementation and proof are recorded in
[T380 S1 evidence](../etc/evidence/t380-s1-session-profile-selection.md).

## Closure Audit

P1 `f00478fb` implements the VM-owned menu, direct profile parse path and
truthful selected-session output.  Coordinator review found that the Console
smoke showed selection but did not assert it; P2 `987bdd96` adds that
assertion.  The focused console/profile/Model-339 replay and full current gate
pass.  The final review confirms no descriptor, ROM/media, device or timing
contract changed; T380 therefore closes and its proposal is retained beside
this record.
