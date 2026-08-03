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
`build/output/` directory. Beginning with M5 T48, the task number is the patch
version: `T48` is `0.5.0048`, `T49` is `0.5.0049`, and so on. Use
`nxvm_0_5_NNNN.exe` for the bootable VM product or `ntvdm64_0_5_NNNN.exe` for
the DOS app-runner product, where `NNNN` is the four-digit decimal task
number. Earlier historical artifacts retain their recorded names and banners.
Record the artifact SHA-256, source commit, runtime identity/banner, and
whether it is a baseline/developer artifact or a product artifact in the
verification record. Smoke-test executables remain build-tree verification
tools and are never copied as developer artifacts. Design-only tasks do not
manufacture executables. M3 is a recorded historical exception: only its final
T5 artifact is retained. Local artifacts are never release evidence, must not
bundle protected media or Microsoft binaries, and may be replaced only by a
newly verified build of the same named task.

## Build Tree Hygiene

After every build, test, smoke run, sanitizer run, or failed verification,
remove owned temporary build products as soon as they are no longer required
by the active task or its immediate next task. `build/output/` is the sole
preserved local artifact directory and retains verified task executables. All
other build configuration trees, object files, generated test executables,
logs, traces, sanitizer trees, and stale CMake/Ninja state are disposable
unless an active subtask records why they are needed. Before recursive cleanup,
verify the resolved target is below `build/`, exclude `build/output/`, and
confirm no owned process still uses it.

For a runnable artifact, the verification record also states the emitted runtime
identity/banner and version. It must follow the task-version rules in
`docs/architecture/overview.md`; changing identity, version, or cutover state
without an approved subtask and regression evidence is prohibited.

For a legacy coupled system, first establish and record a runnable full-source
baseline before subtractive refactoring. A baseline import may be isolated from
the final module layout, but it cannot gain new product behavior or weaken
source, asset, licensing, or test rules.

Differential debugging is a bounded verification experiment. Its record names
both implementations, inputs, event schema, checkpoints, comparison masks,
instruction/time/no-progress budgets, and cleanup owner. It cannot become a
runtime dependency or replace focused project-owned tests.

## Recorder Trace Containment

Raw instruction recording is an ignored, potentially unbounded diagnostic
artifact. A recorder run is prohibited unless its subtask record declares a
unique ignored output path, wall-clock, no-progress, and maximum-byte budget,
the process-tree cleanup owner, and the checkpoint data to retain after the raw
trace is deleted. The byte budget is a hard limit: a cooperative recorder must
stop before it; otherwise the host harness monitors file growth and terminates
the entire launched process tree at the first exceeded limit. A timeout alone
is not sufficient.

Use a fresh run-specific name under ignored `build/` or `artifacts/`; never
reuse a trace path from an earlier run. Before launch, reserve at least twice
the declared byte budget as free workspace space. After every completion,
timeout, failure, or cancellation, the harness must wait for process exit,
verify that the trace handle is closed, record the final size/checkpoint in the
compact verification record, and delete the raw trace unless the approved
subtask explicitly retains it for immediate diagnosis. The next recorder run is
blocked while a prior owned process or trace remains.

Legacy recorders without an in-process byte limit are diagnostic-only and must
run through this monitored harness. Their raw output is never a fixture,
baseline, release artifact, or committed evidence.

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
