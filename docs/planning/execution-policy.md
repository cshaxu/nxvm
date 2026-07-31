# Execution Policy

One subtask is active at a time. Before implementation, create a subtask record
with objective, non-goals, reference baseline, files/ABI surface, exact test
commands, expected markers, asset needs, and stop conditions.

A design milestone is not complete until it has produced the bounded Task and
subtask breakdown for its immediate implementation milestone. The breakdown
maps each task to an approved decision, contract, regression set, acceptance
result, and stop condition. Do not pre-create or activate implementation tasks
from an earlier milestone: their shape is an output of the preceding design
gate, not an M0 prediction. M8 instead approves one such breakdown for each
admitted corpus increment before implementation.

A subtask completes only when scoped behavior works, focused tests pass,
established corpus entries remain valid, provenance/evidence records are
updated where needed, `docs/verification/` contains a compact result, and the
task tracking entry is updated in the same commit.

Each completed implementation task that changes a runnable path must compile,
verify, and copy one usable task-level local build output to the ignored
`build/output/` directory. Use `nxvm-m<M>_t<T>.exe` for the bootable VM product or
`ntvdm64-m<M>_t<T>.exe` for the DOS app-runner product. Its runtime identity
uses the aggregate suffix `m<M>t<T>`. Baseline artifacts may keep the
historical `ntvdm64-*` name already recorded by M1. Record its SHA-256, source
commit, and whether it is a baseline/developer artifact or a product artifact
in the verification record. Smoke-test executables remain build-tree
verification tools and are never copied as developer artifacts. Design-only
tasks do not manufacture executables. M3 is a recorded historical exception:
only its final T5 artifact is retained. Local artifacts are never release
evidence, must not bundle protected media or Microsoft binaries, and may be
replaced only by a newly verified build of the same named task.

For a runnable artifact, the verification record also states the emitted runtime
identity/banner and version. It must follow the pre-cutover or post-cutover
rules in `docs/architecture.md`; changing identity, version, or cutover state
without an approved subtask and regression evidence is prohibited.

For a legacy coupled system, first establish and record a runnable full-source
baseline before subtractive refactoring. A baseline import may be isolated from
the final module layout, but it cannot gain new product behavior or weaken
source, asset, licensing, or test rules.

Differential debugging is a bounded verification experiment. Its record names
both implementations, inputs, event schema, checkpoints, comparison masks,
instruction/time/no-progress budgets, and cleanup owner. It cannot become a
runtime dependency or replace focused project-owned tests.

Do not advance a milestone merely because individual code exists. The roadmap
owns milestone goal, scope, and exit conditions; its active subtask owns exact
commands, expected markers, budgets, and stop conditions. `breakdown.md` is an
index only and cannot close a milestone.
When a release cadence is established, implementation milestones M1, M3, M5,
M7, M9, and M10 create immutable snapshot branches from verified `main` commits
before the next milestone begins. M2, M4, M6, and M8 are design milestones; M11
is a research milestone with one active task at a time. Design and research
work has no default snapshot obligation, though the owner may require one for a
specific approved deliverable.

Escalate to the owner if a license choice, protected asset, incompatible source
license, undocumented Windows mechanism, or change to the direct-launch product
boundary becomes necessary.
