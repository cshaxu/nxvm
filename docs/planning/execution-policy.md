# Execution Policy

One subtask is active at a time. Before implementation, create a subtask record
with objective, non-goals, reference baseline, files/ABI surface, exact test
commands, expected markers, asset needs, and stop conditions.

A subtask completes only when scoped behavior works, focused tests pass,
established corpus entries remain valid, provenance/evidence records are
updated where needed, `docs/verification/` contains a compact result, and the
task tracking entry is updated in the same commit.

Do not advance a milestone merely because individual code exists. Each
milestone closes on the observable completion condition in `breakdown.md`.
When a release cadence is established, implementation milestones M1 through M4
create immutable snapshot branches `m1` through `m4` from verified `main`
commits before the next milestone begins. M5 through M7 are research milestones
and have no default snapshot obligation; the owner may require one for a
specific approved research deliverable.

Escalate to the owner if a license choice, protected asset, incompatible source
license, undocumented Windows mechanism, or change to the direct-launch product
boundary becomes necessary.
