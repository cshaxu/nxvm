# Execution Workflow

This is the authority for turning a user request into a closed task. It works
with the [Roadmap](planning/roadmap.md), [Project Status](planning/status.md),
[Debt Tracking](debt-tracking.md), and [Execution Policy](planning/execution-policy.md);
it does not replace their architecture, source, test, or artifact rules.

## Task Packet

Before implementation, `status.md` contains one active task packet. A packet
preserves every original user requirement, even when it is split across tasks.
It has a milestone, task identifier, objective, non-goals, baseline, affected
boundaries, and these two tables:

| Requirement | Original request | Owner task/subtask | State | Evidence or decision |
| --- | --- | --- | --- | --- |
| R1 | Exact user-visible or technical outcome | One accountable owner | Planned | Pending |

| Dependency or risk | Decision / trigger | Resolution owner | State |
| --- | --- | --- | --- |
| D1 | What can block or change the work | One accountable owner | Open |

The requirement table is the acceptance ledger. A later task may implement an
item, but may not remove, silently rewrite, or inherit it without keeping its
`R` identifier and recorded decision.

## State Machine

```text
Intake -> Planned -> Active -> Implemented -> Verified -> Closed
                         |             |              |
                         +-> Blocked   +-> Deferred <-+
```

- **Intake:** record the request verbatim enough to preserve its meaning.
- **Planned:** map every `R` to one task/subtask, non-goal, and acceptance
  method before source changes.
- **Active:** exactly one subtask is active. `status.md` names it and its
  packet; other approved work is queued, not implicitly active.
- **Implemented:** the scoped change exists, but no completion claim follows.
- **Verified:** every implemented `R` has focused evidence: test output,
  artifact, review result, or a design decision where runtime proof is not
  applicable.
- **Deferred:** the owner explicitly approves it, gives a reason, target
  milestone, and re-admission trigger, then records it in `TODO.md`.
- **Blocked:** record the external condition and what can continue safely.
- **Closed:** the closure audit confirms every `R` is verified or approved
  deferred. `Implemented` is never a synonym for `Closed`.

## Closure Audit

The final subtask is always a closure audit performed against the original
requirement table, not the implementation narrative. It must answer:

1. Does every `R` have `Verified` evidence or an owner-approved deferral?
2. Did any requirement change meaning during decomposition? If so, where is
   the owner decision?
3. Did the work preserve required behavior, architecture boundaries, and
   artifact policy?
4. Are all current documentation links, debt states, and task identifiers
   consistent?
5. Is a remaining item truly debt, rather than unfinished work hidden in a
   completed task?

Only a passing closure audit may change the packet to `Closed`, update a
milestone closure checklist, or summarize the task in history.

## Multiple Requests And History

When one user message contains several outcomes, keep them in one packet until
each original requirement has a terminal state. Split implementation for
ordering and risk, never for convenience of forgetting requirements.

Completed task detail may move to history only after closure. The compact
history summary retains the task objective, `R` verdicts, deferrals, artifact
identity when applicable, and source commit. Git preserves the detailed work.

## Enforcement

Before a completion commit, review the task packet and reject closure if any
requirement is `Planned`, `Active`, or merely `Implemented`. Link validation,
the applicable build/smoke gates, and the acceptance ledger are all required
evidence. Standalone documentation work uses `Td` and has the same closure
audit, but no task artifact.
