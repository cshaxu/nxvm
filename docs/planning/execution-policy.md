# Execution Policy

One subtask is active at a time. Before implementation, create a subtask record
with objective, non-goals, reference baseline, files/ABI surface, exact test
commands, expected markers, asset needs, and stop conditions.

A subtask completes only when scoped behavior works, focused tests pass,
established corpus entries remain valid, provenance/evidence records are
updated where needed, `docs/verification/` contains a compact result, and the
task tracking entry is updated in the same commit.

For a legacy coupled system, first establish and record a runnable full-source
baseline before subtractive refactoring. A baseline import may be isolated from
the final module layout, but it cannot gain new product behavior or weaken
source, asset, licensing, or test rules.

Differential debugging is a bounded verification experiment. Its record names
both implementations, inputs, event schema, checkpoints, comparison masks,
instruction/time/no-progress budgets, and cleanup owner. It cannot become a
runtime dependency or replace focused project-owned tests.

Do not advance a milestone merely because individual code exists. Each
milestone closes on the observable completion condition in `breakdown.md`.
When a release cadence is established, implementation milestones M1, M3, M5,
M7, and M8 create immutable snapshot branches from verified `main` commits
before the next milestone begins. M2, M4, and M6 are design milestones; M9
through M11 are research milestones. Design and research milestones have no
default snapshot obligation, though the owner may require one for a specific
approved deliverable.

Escalate to the owner if a license choice, protected asset, incompatible source
license, undocumented Windows mechanism, or change to the direct-launch product
boundary becomes necessary.
