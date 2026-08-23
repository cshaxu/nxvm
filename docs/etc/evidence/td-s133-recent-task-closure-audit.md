# M5 Td S133 Recent Task Closure Audit

## Scope

This audit reviews the immediately preceding closure chain T438--T443 only.
It is not a claim about all historical M5 tasks. It inspected each main record,
proposal companion/evidence, implementation commit surface, artifact hash,
focused regression, current source owner, and current gate disposition.

## Closure Matrix

| Task | Actual owner/minimalism conclusion | Current proof and disposition |
| --- | --- | --- |
| T438 | Core firmware context remains the single reset failure owner; no VM reset wrapper or parallel rollback route is present. | Artifact hash matches; firmware failure/retry smoke passes. Its historical full-gate claim transfers to the shared restoration candidate below. |
| T439 | VM lifecycle retains one visible reset outcome and one finish-reset completion path; Core owns reset itself. | Artifact hash matches; session initialization/reset retry smoke passes. Proposal companion was moved to history. Shared-gate transfer applies. |
| T440 | Model-40 policy stays at VM profile/session boundary; Core has no machine-name branch and generic reconfiguration remains separate. | Artifact hash matches; Model-40 rejection and generic reconfiguration smokes pass. Proposal companion was moved to history. Shared-gate transfer applies. |
| T441 | Media staging remains one VM owner; FDD owns only derived-sidecar release on paired-save failure. | Artifact hash matches; media persistence failure smoke passes. Shared-gate transfer applies. |
| T442 | CPU lexical and RAM mapping checks stay at their respective owners; no validation facade or caller copy was added. | Artifact hash matches; both boundary smokes pass. Original implementation/closure commits lacked S/P subjects; `44a8c68d` records the immutable-history correction without rewrite. Shared-gate transfer applies. |
| T443 | Mailbox owns frame/lock state; renderers consume status and VM has one publish/generation commit path. | Artifact hash matches; mailbox, display publication and Win32 renderer smokes pass. Shared-gate transfer applies. |

## Audit Repairs And Unavoidable Historical Fact

T439 and T440 were closed but their proposals still lived in `docs/proposals/`
and were retained in Queue indexes. This audit moves both to their numbered
history companions and repairs their links.

T442's original source commit `f882abe7` and closure commit `6746d400` lack
the required S/P commit subjects. Rewriting already-pushed public history would
be destructive; `44a8c68d` is the durable explicit correction. New admission
was blocked until that record existed, so the identifier gate now prevents the
same omission from silently advancing task numbering.

## Gate Transfer

All nine focused CTest invocations for T438--T443 pass in the current 0443
build. However, the current-fast aggregate reproducibly has 20 CPU-timing,
planar-parity, RTC, DMA/RTC and control-transfer failures, and the full current
gate has the T344 historical fixture mismatch (71 expected direct machine
constructors; 75 found). Those failures mean no audited task may truthfully
claim a currently passing global gate.

The new [current-gate regression restoration](../../proposals/m5-current-gate-regression-restoration.md)
candidate is the required transfer. It first classifies a common owner or
splits the distinct owners; it may not weaken tests or rebaseline counts without
evidence. This makes the prior task behavior closures truthful while treating
their shared global-gate exit condition as open project work rather than a
hidden exception.
