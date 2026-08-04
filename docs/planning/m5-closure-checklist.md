# M5 Closure Checklist

## Purpose

This is the single operational index for closing M5. It does not create new
architecture or execution rules: `roadmap.md`, `RULES.md`, and
`execution-policy.md` remain authoritative. A checked item requires a compact
evidence record with commands, result markers, source commit, and any approved
exception. Historical task evidence is not enough when the current source graph
has changed.

M5 remains open until every applicable item is evidenced or an owner-approved
deferral is recorded in `TODO.md` with its target milestone. M6 must not use an
unfinished M5 item as an implicit waiver.

## 1. Current Build Graph

- [ ] The configured GCC preset, CMake target names, task version, and artifact
  name describe the current source graph; stale task targets and fixture-only
  targets are not presented as product evidence.
- [ ] The full applicable build and focused smoke gates pass from a clean owned
  build tree. A test proves exactly the runnable path or contract claimed.
- [ ] Every runnable behavior-changing M5 task has one verified task-level EXE
  in `build/output/`, with SHA-256, source commit, banner/version, and runtime
  classification recorded in its verification record.

## 2. Workspace And Asset Hygiene

- [ ] `build/output/` is the only retained local artifact directory. Other
  owned build trees, logs, traces, test executables, and stale CMake/Ninja
  state are removed unless the active subtask records a short-lived need.
- [ ] No protected guest media, Microsoft binary, machine-local path, raw trace,
  generated binary, or unreviewed third-party material is tracked or presented
  as repository evidence.
- [ ] Any differential-debug or recorder experiment has its declared time,
  no-progress, size, process-cleanup, and raw-output deletion evidence.

## 3. Single Owner And Single Runtime Path

- [ ] A current source audit covers every `core/{machine,platform,product}`,
  `vm/{machine,platform,product,profile}`, `vdm/{machine,platform,product,profile}`,
  composition root, target, test, and main entry.
- [ ] Each mutable runtime datum has one explicit owner. There is one production
  core executor, one NXVM session/execution path, and no duplicate machine,
  session, reset, or VM-side instruction loop.
- [ ] Production code has no process-global, `_Thread_local`, singleton, or
  implicit current-object session/machine selector. Process-exclusive host
  resources use explicit leases.
- [ ] `core` has no VM/VDM dependency; core peer modules remain independent;
  only product-root composition integrates product peers. Platform code does
  not mutate guest state.
- [ ] Every adapter/wrapper adds a documented boundary responsibility. No
  forwarding-only facade, permanent same-object alias, or test-only historical
  path remains in a formal product route.
- [ ] The session-owned platform run handle is the sole owner of worker create,
  stop, join, backend destruction, and host-resource release. Workers only
  report state or completion. Windows failure paths are verified; Linux native
  validation is either evidenced or recorded as the explicit portability
  follow-up.

## 4. Retained NXVM Behavior

- [ ] The retained NXVM Console, debugger, startup sequence, and direct command
  lifecycle are exercised without an unapproved user-visible change.
- [ ] Applicable FDD/HDD boot, DOS prompt, keyboard/display, pause/debug, and
  multi-session isolation baselines pass. A changed path has before/after
  acceptance evidence rather than a directory or compile-only claim.
- [ ] Core lifecycle, configuration freeze, provider order, memory mapping, and
  cold RAM reconfiguration contracts retain their documented boundary tests.

## 5. Documentation And Deferred Work

- [ ] `status.md`, `roadmap.md`, architecture contracts/layout, CMake presets,
  task tracking, and verification records agree on the current implementation,
  active subtask, artifact identity, and M5 scope.
- [ ] Every remaining compatibility, hardware, platform, or product gap is
  classified in `TODO.md` as a bounded deferred item. VDM remains a deliberately
  non-runnable skeleton until M6 approves its DOS contracts.
- [ ] The final M5 verification record links this checklist item-by-item to
  commands and evidence, states every approved deferral, and explicitly says
  whether M5 may close. Only then may `TODO.md`, `status.md`, and `roadmap.md`
  record M5 closure.

## Evidence Sources

- Milestone scope and exit: [Roadmap](roadmap.md)
- Task, artifact, build-tree, and trace policy: [Execution Policy](execution-policy.md)
- Dependency, ownership, lifecycle, and wrapper rules: [Repository Rules](../../RULES.md)
- Open work and approved deferrals: [TODO Ledger](../../TODO.md)
