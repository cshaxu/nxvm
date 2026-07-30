# Agent Instructions

Read these documents before changing the repository:

1. `docs/planning/status.md`
2. `docs/roadmap.md`
3. `docs/architecture.md`
4. `docs/source-policy.md`
5. `docs/planning/execution-policy.md`
6. `CONTRIBUTING.md`

## Boundaries

- ntvdm64 is the canonical product repository. Sibling references, including
  `../nxvm`, are read-only unless a task explicitly authorizes a source import
  into this repository.
- NXVM is the formal machine foundation. Imports require exact provenance,
  preserved LGPL notices, and focused tests.
- The owned DOS backend is the default path. Microsoft guest mode is optional
  BYOB research. OpenNT and NTVDMx64 are historical research only.
- Use the module boundaries defined in `docs/architecture.md`.
- Invasive integration and Win16 are research-only. Do not add them to default
  builds, release dependencies, or automatic system changes.
- Do not commit protected guest media, Microsoft binaries, machine-local paths,
  or unreviewed third-party code.

## Execution

- Keep exactly one active subtask in `docs/planning/status.md`.
- Record evidence, provenance, verification, and task tracking when affected.
- Preserve unrelated changes and avoid destructive Git operations.
- Commit subjects use `M<milestone> T<task> S<subtask> P<part>: description`.
