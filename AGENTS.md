# Agent Instructions

These rules apply to the complete repository. Read the following before making
changes:

1. `docs/planning/status.md`
2. `docs/planning/breakdown.md`
3. `docs/planning/execution-policy.md`
4. `CONTRIBUTING.md`
5. The active task's related architecture, evidence, reference, and
   verification records.

## Boundaries

- This is the canonical `ntvdm64` product repository. Sibling repositories,
  including `../nxvm`, `../pc110-js`, and local NTVDMx64/WineVDM checkouts, are
  read-only references.
- All repository artifacts are English. Conversations with the owner may be
  Chinese.
- Use relative paths in committed artifacts. Do not commit machine-specific
  paths, registry exports, protected media, or user application binaries.
- Preserve unrelated changes. Do not use destructive Git operations.
- Implement one active subtask at a time. The active subtask must be listed in
  `docs/planning/status.md` and have acceptance criteria before code changes.

## Product Direction

- C is the implementation language. Prefer documented Win32 APIs and a
  project-owned VM/host boundary.
- The initial launch contract is an explicit `ntvdm64 run` command. Shell
  integration is opt-in and later.
- Do not add undocumented Windows hooks, loader patches, AppInit/AppCert
  mechanisms, symbol-server dependencies, or global DLL injection.
- Do not substitute application-specific shortcuts for DOS, BIOS, interrupt,
  or device behavior. A synthetic direct-load DOS environment is allowed when
  its ABI state and interrupt behavior are documented and tested.
- WineVDM stays out-of-process unless the owner explicitly changes the
  licensing and packaging decision.

## Evidence And Completion

- Follow `docs/governance/evidence-policy.md` and
  `docs/governance/source-and-license-boundary.md`.
- Record a source baseline before studying or deriving a nontrivial subsystem.
- Preserve each established compatibility baseline. Add a compact verification
  record for every completed subtask.
- Commit subjects use `M<milestone> T<task> S<subtask> P<part>: description`.
