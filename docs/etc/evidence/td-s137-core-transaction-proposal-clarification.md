# Td S137: Core Transaction Proposal Clarification

## Scope

Owner-approved documentation governance clarifies the queued Core CPU-to-board
transaction and arbitration candidate.  It changes no runtime source, ABI,
Queue order or task admission.

## Recorded Design Constraints

- CPU execution requests typed transactions but does not decide board-specific
  availability, completion, arbitration or device phases.
- One Core transaction owner owns every guest fetch/memory/port transaction,
  including DMA variants, from classification through commit, cancellation and
  reset.
- One Core arbitration owner owns CPU/DMA/refresh eligibility, priority,
  grant, release and cancellation; a competing request cannot leave a partial
  transaction committed.
- Controller/device owners publish or consume declared routes and retain only
  their chip state.  They neither mutate CPU state nor create a second access,
  timing or observation path.
- VM chooses validated immutable data; it supplies neither Core callbacks nor
  machine-name branches.  Missing selected-board facts remain explicit L2
  fallback or rejection.
- The proposal's S1--S6 sequence freezes routes, plan selection, shared
  transaction lifecycle, availability/prefetch, arbitration/memory classes,
  then sole retirement/trace integration and closure audit.

## Review And Verification

The actual proposal diff was checked against the owner request, the queued
candidate scope, the architecture one-owner/one-path rules, and the next
controller/device candidate.  It adds no second architecture authority: the
proposal remains an unnumbered candidate detail, while Current retains only
this Td's concise closure status.

`verify-documentation-governance` and `git diff --check` passed at closure.

`M5:Td:S137:CORE-TRANSACTION-PROPOSAL-CLARIFICATION:OK`
