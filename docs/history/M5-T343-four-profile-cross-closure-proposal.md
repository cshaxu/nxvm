# Four-Profile Cross-Closure Verification

## Objective

Perform the final evidence-led cross-check of the 8086, 80186, 80286, and
80386DX ledgers after their implementation candidates close. Verify that each
form has exactly one truthful profile/mode disposition and that shared state
transitions compose across the profile boundaries.

## Scope and completion standard

This is a closure verification candidate, not a substitute for missing
implementation. It reconciles the source graph, focused regressions, generated
profile matrices, open TODO transfers, and Intel boundaries. It closes only
when no in-scope row is partial, missing, or unclassified; any discovered gap
returns to a newly ordered implementation candidate before Windows readiness
or M5 closure work proceeds.

## Required reconciliation order

First verify the shared delivery rows, then compare each form's 8086/80186,
80286, and 80386DX disposition, and finally reconcile cross-profile state
composition: prefix and `LOCK` legality, operand/address attributes, frame and
restart rules, privilege, task, paging, debug, and VM86 boundaries. The audit
must distinguish Intel-required layout differences from accidental divergent
construction. It may close evidence only; a missing mechanism is returned to
the earliest owning implementation candidate with a precise admission reason.

Its durable output is the final four-profile disposition ledger: each
applicable form/state transition names its Intel profile boundary, mode and
privilege condition, evidence owner, and any deliberate external transfer.
No Windows, timing, device, or x87 conclusion follows from this ledger.

## Audit Outputs

This final audit is a dependency barrier, not a sixth implementation bucket.
It checks delivery first, then form dispositions, then cross-profile state
composition. Any gap returns to the earliest form or state candidate with a
finite missing matrix; no cleanup is implemented inside the audit.
