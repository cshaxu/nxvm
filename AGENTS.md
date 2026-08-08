# Agent Instructions

Read these documents before changing the repository:

1. `docs/README.md`
2. `docs/planning/status.md`
3. `docs/execution-workflow.md`
4. `docs/planning/execution-policy.md`
5. `docs/planning/roadmap.md`
6. `docs/architecture/overview.md`
7. `docs/architecture/module-layout.md`
8. `docs/architecture/contracts.md`
9. `docs/coding-standard.md`
10. `docs/source-policy.md`
11. `CONTRIBUTING.md`

## Boundaries

- ntvdm64 is the canonical successor repository. Its current and planned
  source forms are `core`, `vm`, `mantle`, `dos`, and `vdm`: `vm` builds
  `nxvm.exe`; `vdm` will build `nxvdm.exe` over the shared `mantle` and owned
  `dos` components. `core.dll`, `mantle.dll`, and `dos.dll` are medium-term
  targets only. Sibling references,
  including `../nxvm`, are read-only unless a task explicitly authorizes a
  source import into this repository.
- NXVM is the formal machine foundation and continuing bootable-VM product
  surface. Copyright-holder-authorized imports use the root MIT License and
  require exact provenance, preserved copyright notices, and focused tests.
- The owned DOS backend is the default NXVDM path. Trusted external VDM/DOS
  research reports may inform neutral requirements, but never become a shared
  ABI, backend, default dependency, or release input. Microsoft components
  remain research-only, not a backend.
- Use the module boundaries defined in `docs/architecture/module-layout.md`.
- Apply the applicable local architecture, coding, source, and execution
  rules to every task; record them and any owner-approved exception in the
  task and verification evidence.
- Invasive integration, Microsoft component research, and Win16 are research-
  only. Do not add them to default builds, release dependencies, or automatic
  system changes.
- Do not commit protected guest media, Microsoft binaries, machine-local paths,
  or unreviewed third-party code.

## Execution

- Keep exactly one active subtask in `docs/planning/status.md`.
- Allocate numeric tasks and subtask/governance identifiers only in the linear
  order defined by `docs/planning/execution-policy.md`; never skip, reopen, or
  reuse a closed identifier.
- Record evidence, provenance, verification, and task tracking when affected.
- Preserve unrelated changes and avoid destructive Git operations.
- Commit subjects use `M<milestone> T<task> S<subtask> P<part>: description`
  for implementation tasks, or `M<milestone> Td [S<subtask>] P<part>:
  description` for standalone documentation work. `Td` does not consume a
  numeric task identifier.
