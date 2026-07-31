# Contributing

## Engineering Rules

- Repository content is English. Runtime code is C11 plus narrowly scoped
  documented Win32 APIs.
- Follow the visible module boundaries in `src/`. `machine` has no DOS or
  Windows dependency; `dos` has no concrete Windows API dependency; `app` has
  no emulation logic; `runtime` is the composition root.
- One subtask is active at a time. Define its scope, non-goals, source baseline,
  verification commands, and acceptance evidence before changing runtime code.
- Use `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` only for deferred work.

## NXVM Imports

NXVM is the approved machine foundation, not merely an example. Before copying
or substantially deriving code, add a provenance record containing commit,
source and destination paths, the MIT authorization record, preserved copyright
notices, changes, and tests.
Keep imported code inside the appropriate `machine` boundary and reduce legacy
global-state coupling when making new interfaces.

## References And Guest Components

Other open-source projects require a license review before copying. OpenNT and
NTVDMx64 are historical research sources only. Microsoft binaries are never
committed. Microsoft NTVDM research belongs under `docs/research/` or approved
`tools/research/`; it cannot create a default runtime dependency or block the
owned DOS backend.

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

Use `M<milestone> T<task> S<subtask> P<part>: description`. Keep task records
under `docs/tracking/`, write a verification record for completed subtasks, and
preserve all established baselines.

When a completed subtask produces a runnable Windows executable, place its
verified local copy in ignored `build/output/` as
`ntvdm64-m<M>_t<T>_s<S>.exe`. Record its hash and artifact classification in
the verification record; do not treat it as a release or bundle protected media.
