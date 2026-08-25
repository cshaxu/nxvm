# M5 Closure Checklist

## Purpose

This is the single operational index for closing M5. It does not create new
architecture or execution rules: `design/ROADMAP.md`, `design/CODING.md`,
`rules/CODING.md`, and `rules/EXECUTION.md` remain authoritative. A checked
item requires a compact
evidence record with commands, result markers, source commit, and any approved
exception. Historical task evidence is not enough when the current source graph
has changed.

An M5-close decision requires every applicable item to be evidenced or an
owner-approved deferral to be recorded in `TODO.md` with its target milestone.
M6 mantle work must not use an unfinished M5 boundary or corpus item as an
implicit waiver. Current milestone state remains in `states/CURRENT.md`.

## 1. Current Build Graph

- [x] The configured GCC preset, CMake target names, task version, and artifact
  name describe the current source graph; stale task targets and fixture-only
  targets are not presented as product evidence.
- [x] The full applicable build and focused smoke gates pass from a clean owned
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

- [ ] A current source audit covers `core/utils`, every
  `core/{machine,platform,product}`,
  `vm/{machine,platform,product,profile}`, and every implemented
  `mantle/{machine,platform,product}`, `dos/{machine,platform,product,profile}`,
  and `vdm/{machine,platform,product,profile}` root, plus each composition
  root, target, test, and main entry. A planned component is recorded as not
  applicable until its admission task creates it.
- [ ] Each mutable runtime datum has one explicit owner. There is one production
  core executor, one NXVM session/execution path, and no duplicate machine,
  session, reset, or VM-side instruction loop.
- [ ] Product-visible core contracts export no raw CPU, RAM, port, controller,
  or executor pointer. VM composition binds typed frozen configuration and
  providers; the core-only second-consumer fixture proves that boundary before
  M6 may construct mantle.
- [ ] Production code has no process-global, `_Thread_local`, singleton, or
  implicit current-object session/machine selector. Process-exclusive host
  resources use explicit leases.
- [ ] `core` has no VM, mantle, DOS, or VDM dependency; core peer modules
  remain independent; VM and mantle composition are the only integration
  owners. Platform code does not mutate guest state.
- [x] `core/utils` depends only on `type-facade` and carries only neutral,
  explicitly named utility or callback contracts. No VM, mantle, DOS, or VDM
  peer imports a non-matching `core/*` peer, and the CMake target graph has the
  same boundary.
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
- [ ] The selected Windows 3.x startup corpus either reaches its declared
  checkpoint or records every unmet CPU/device/profile prerequisite as a
  bounded deferral. A boot attempt alone is not compatibility proof.
- [ ] Core lifecycle, configuration freeze, provider order, memory mapping, and
  cold RAM reconfiguration contracts retain their documented boundary tests.

## 5. Documentation And Deferred Work

- [ ] `states/CURRENT.md`, `design/ROADMAP.md`, architecture/layout detail, CMake presets,
  task tracking, and verification records agree on the current implementation,
  active subtask, artifact identity, and M5 scope.
- [ ] Every remaining compatibility, hardware, platform, or product gap is
  classified in `TODO.md` as a bounded deferred item. Mantle, DOS, and VDM
  remain non-runnable until their respective M6/M8 admission work begins.
- [ ] The final M5 verification record links this checklist item-by-item to
  commands and evidence, states every approved deferral, and explicitly says
  whether M5 may close. Only then may `TODO.md`, `states/CURRENT.md`, and `design/ROADMAP.md`
  record M5 closure.

## Evidence Sources

- Milestone scope and exit: [Roadmap](../../../design/ROADMAP.md)
- Task, artifact, build-tree, and trace policy: [Execution Policy](../../../rules/EXECUTION.md)
- Dependency and ownership rules: [Module Layout](../../../design/CODING.md)
- Lifecycle and public-contract detail: [Machine Contract Details](../../architecture/machine-contract-details.md)
- Coding and execution rules: [Coding Standard](../../../rules/CODING.md) and
  [Execution Policy](../../../rules/EXECUTION.md)
- Open work and approved deferrals: [TODO Ledger](../../../states/TODO.md)
