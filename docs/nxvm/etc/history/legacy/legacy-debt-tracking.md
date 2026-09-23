# Legacy Debt Tracking

> Superseded by [docs/states/TODO.md](../../../states/TODO.md). Retained only as a historical
> description of an earlier governance layout; it is not current policy.

[`../TODO.md`](../../../states/TODO.md) is the sole live technical-debt ledger. It records
known compatibility gaps, architecture risks, platform limitations, and
recurring review failures. `STATUS.md` remains the sole active-work
authority; the ledger never starts a task or changes product behavior.

Add debt only when it has a concrete risk, missing capability, or repeated
failure pattern. Each item must state its priority (`High`, `Medium`, or
`Low`), owner/boundary, evidence or trigger, and the milestone or design gate
that may admit it. On completion, remove it in the closure commit; its
verification reference belongs in milestone history and Git.

Use the [M5 closure checklist](../../operations/policy/m5-closure-checklist.md) for M5
closure evidence. Use [Roadmap And Milestones](../../../design/ROADMAP.md) to decide
when an admitted debt item may become work.
