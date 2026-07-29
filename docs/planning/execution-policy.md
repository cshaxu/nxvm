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
When a release cadence is established, snapshot branches `m1` through `m5` are
created from the verified `main` commit before the next milestone begins.

Escalate to the owner if a license choice, protected asset, incompatible source
license, undocumented Windows mechanism, or change to the direct-launch product
boundary becomes necessary.
