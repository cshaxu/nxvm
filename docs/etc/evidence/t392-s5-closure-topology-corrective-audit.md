# T392 S5 Closure-Topology Corrective Audit

## Finding

Accepted T392 S4 closed the task in `CURRENT.md`, but its retained proposal
remained in `docs/proposals/` and Queue still listed it as an unnumbered
candidate. That conflicts with the documentation topology: closed numeric-task
proposals belong beside the numbered history record, and Queue contains only
current unnumbered candidates.

## Corrective Boundary

This is the latest closed numeric task, so T392 S5 uses the narrow
`Corrective` allocation exception. P1 adds this audit plus direct history/index
links. After independent P1 review, P2 removes the corresponding Queue candidate
and moves the retained proposal to `docs/history/`. It does not select or
implement the transferred generic device-operation snapshot, and it changes no
runtime, artifact, Core/VM interface, asset, timing, physical or L3 behavior.

## Complete Reference Sweep

- `CURRENT.md` remains the sole current-status authority and carries the active
  T392 S5 packet while retaining T392 as the latest closed numeric task.
- The active packet and Queue still link the retained proposal while P1 is
  under review; P2 removes the completed candidate atomically with archival.
- The active packet links the retained proposal in `docs/proposals/`; P2 moves
  it beside T392's numbered record only after P1 review and acceptance.
- T392 history, the supporting-document index and this audit form the durable
  retained record. Direct T392 reference search found no other current proposal
  or Queue link requiring change.

## Verification And Transfer

The corrective requires diff hygiene, the retained focused C0/C0A/C1 capture
regression and the complete documentation governance gate. Its final acceptance
must inspect the actual P1 diff. The earlier transfer remains unchanged: a
separately admitted generic immutable Core device-operation snapshot plus a
selected VM consumer contract is required before any new C1 endpoint claim;
physical qualification remains blocked.

## Acceptance Result

Coordinator review of P1 `3b40d2ce` confirmed that its changed surface was
limited to the active packet, numbered history, evidence and supporting index.
P2 atomically moves the proposal beside the T392 history record and removes its
Queue candidate. `git diff --check`, the focused C0/C0A/C1 capture regression,
and the complete documentation governance gate pass. The corrective re-closes
T392 without changing runtime source, artifact identity, Core/VM boundaries,
assets, timing, physical qualification or L3 status.
