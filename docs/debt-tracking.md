# Debt Tracking

[`../TODO.md`](../TODO.md) is the sole live technical-debt ledger. It records
known compatibility gaps, architecture risks, platform limitations, and
recurring review failures. `planning/status.md` remains the sole active-work
authority; the ledger never starts a task or changes product behavior.

Add debt only when it has a concrete risk, missing capability, or repeated
failure pattern. Each item must state its priority (`High`, `Medium`, or
`Low`), owner/boundary, evidence or trigger, and the milestone or design gate
that may admit it. Mark it complete only with a current verification reference.

Use the [M5 closure checklist](planning/m5-closure-checklist.md) for M5
closure evidence. Use [Roadmap And Milestones](planning/roadmap.md) to decide
when an admitted debt item may become work.
