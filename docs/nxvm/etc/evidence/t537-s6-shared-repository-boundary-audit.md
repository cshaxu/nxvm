# M5 T537 S6: Shared Repository-Boundary Audit

## Scope

This is a `Shared` task.  It changes only root shared CMake configuration and
the six-component Shared corpus (`src/lib`, `test/lib`); NXVM and MyNES product
source, tests, tools, configuration, assets and artifacts remain untouched.

## Findings And Repairs

| Finding | Repair |
| --- | --- |
| Every NXVM root configure preset inherited `REPOSITORY_BUILD_MYNES=ON`, silently adding MyNES to an NXVM graph. | Each NXVM preset now explicitly sets `REPOSITORY_BUILD_MYNES=OFF`. Existing MyNES presets already set `REPOSITORY_BUILD_NXVM=OFF`. |
| `src/lib/verify_kvm_naming.cmake` changed without its `MANIFEST.sha256` row changing. | Updated the one stale canonical SHA-256 entry. |
| Shared Lib tests still contained SoftPC-specific names, output text and scratch-file names. | Renamed only test-local identifiers/text to neutral Shared/Monitor terms; behavior and coverage are unchanged. |

The only retained product spelling in the six-component tree is the
owner-approved standalone CMake project name `nxvm_shared_library`.  It is a
formal library name, not a runtime dependency or product behavior.

## Verification

| Check | Result |
| --- | --- |
| NXVM x64 preset configuration | `REPOSITORY_BUILD_NXVM=ON`, `REPOSITORY_BUILD_MYNES=OFF` |
| MyNES x64 preset configuration | `REPOSITORY_BUILD_MYNES=ON`, `REPOSITORY_BUILD_NXVM=OFF` |
| Lib manifest | Passed |
| Common manifest and corpus boundary | Passed |
| x86 manifest and corpus boundary | Passed |
| Shared labelled CTest (`lib`, `common`, `x86`) | 56/56 passed |
| NXVM documentation governance | Passed |
| MyNES documentation governance | Passed |
| Actual diff | Shared configuration/corpus plus T537 status/evidence only; no App implementation change |

## Boundary Result

> **Superseded governance conclusion.** M5 T537 S7 replaced the exclusive
> task-scope model on 2026-09-23. This S6 record remains evidence for its
> configuration and corpus checks; it is not the current authority for task
> targets or commit prefixes.

The repository now has exactly three enforceable work scopes: `NXVM`, `MyNES`,
and `Shared`.  A product task cannot alter a sibling or a shared component;
Shared is the sole receiver for the six components, their tests, shared CMake
configuration and shared governance documents.
