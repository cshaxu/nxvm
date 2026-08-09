# Contributing

## Engineering Rules

- Repository content is English. Runtime code is C11 plus narrowly scoped
  documented Win32 APIs.
- [System Architecture](docs/design/ARCHITECTURE.md),
  [Source Layout](docs/design/CODING.md),
  [Coding Rules](docs/rules/CODING.md), [Source Rules](docs/rules/SOURCE.md),
  and [Execution Rules](docs/rules/EXECUTION.md) are the local
  implementation checklist. Every task records its applicable rules and proves
  them at exit or records an owner-approved exception.
- Follow the visible module boundaries in `src/`. `core/machine` has no VM/VDM
  policy, concrete product UI, profile, or host OS dependency; `core/platform`
  never mutates guest state. `vm/*`, `mantle/*`, `dos/*`, and `vdm/*` contain
  only their own component behavior. `dos/*` is independent of all other
  components. `core/product` is shared infrastructure; `vm/product` and
  `vdm/product` contain user experience; `mantle/` owns reusable VDM
  composition. The `vm/` and `mantle/` roots own composition.
- One subtask is active at a time. Define its scope, non-goals, source baseline,
  applicable local rules, verification commands, and acceptance evidence before
  changing runtime code.
- Use `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only for deferred work.

## NXVM Imports

NXVM is the approved machine foundation and continuing bootable-VM product
surface, not merely an example. Before copying or substantially deriving code,
add a provenance record containing commit, source and destination paths, the
MIT authorization record, preserved copyright notices, changes, and tests. Keep
imported code inside the appropriate shared-core, firmware, platform, or
product boundary and reduce legacy global-state coupling when making new
interfaces.

## References And Guest Components

Other open-source projects require a license review before copying. Trusted
external VDM/DOS research may inform neutral requirements, but cannot create a
shared ABI, source import, default runtime dependency, or substitute for owned
product evidence. Microsoft binaries are never committed. Microsoft research
belongs under `docs/research/` or approved `tools/research/`; it cannot block
the owned DOS backend.

## Pull Request Record

Every change must state:

- affected module and whether it crosses a module boundary;
- whether it imports or derives NXVM code, or references another project;
- license and redistributability impact;
- tests added and user-observable behavior changed;
- whether the change involves Microsoft research or invasive integration;
- whether research code becomes a default runtime dependency; and
- whether the work directly improves real-program compatibility and has a
  demonstrable test result.

## Commits And Tracking

Use `M<milestone> T<task> S<subtask> P<part>: description` for implementation
tasks and their task-specific design work. Use
`M<milestone> Td [S<subtask>] P<part>: description` for a standalone
documentation/governance task; `Td` does not allocate a numeric `T` identifier
or require an executable artifact. Keep records appropriate to the work,
preserve all established baselines, and follow
`docs/rules/EXECUTION.md` for the authoritative identifier rules.
Identifiers are allocated strictly in queue order; closed identifiers are never
skipped backward, reopened, or reused.

When a completed subtask produces a runnable Windows executable, place its
verified local copy in ignored `build/output/` using the product-specific
artifact name from `docs/rules/EXECUTION.md`. Record its hash and
artifact classification in the verification record; do not treat it as a
release or bundle protected media.

After each build or test run, delete owned temporary build trees, logs, and
generated test binaries no longer needed by the active or immediate next task.
Preserve verified developer artifacts only in `build/output/`.
