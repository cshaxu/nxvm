# M6 T37 S3 Committed-Corpus Reconciliation

## Authority

The only import source is SoftPC commit
`6251f896b731e28440c614c217b2ef395ac48614`. Its dirty worktree was neither
read as source nor imported. The commit contains `37562abb`, which establishes
the shared Common rule that an absent Window is created only while the machine
is `RUNNING`.

## Hash Audit

For every path tracked beneath each root, MyNes' blob hash was compared with
`HEAD:path` in the committed SoftPC tree:

| Root | SoftPC paths | MyNes paths | Different paths |
| --- | ---: | ---: | ---: |
| `src/lib` | 97 | 97 | 0 |
| `src/common` | 23 | 23 | 0 |
| `test/lib` | 49 | 49 | 0 |
| `test/common` | 20 | 20 | 0 |

The only Common source receiver was
`session/control_state.c`, with its matrix smoke and manifests. It now carries
the generic `runtime_actual == COMMON_SESSION_MACHINE_RUNNING` gate, so no
stopped, initialized, reset-completed, paused or error session can create a
Window merely because its desired display is Window.

## Product Receivers

SoftPC's committed Lib has no atomic replacement API. MyNes therefore uses the
same direct writer contract: App snapshot save and Core battery save open a
truncating writer, write their bytes, then close it while preserving the first
failure. The obsolete atomic test stubs were removed from Core tests. This is
file policy at the App/Core boundary; it adds no shared API.

## Disposition

The four roots are an exact transferable corpus again. The prior S2 evidence
is retained only as a record of the rejected mutable-worktree comparison.

## Verification

- `cmake --build --preset mynes-x64` and `ctest --preset mynes-x64` passed:
  118 tests.
- `cmake --build --preset mynes-x86` and `ctest --preset mynes-x86` passed:
  118 tests.
- `Verify-DocumentationGovernance.ps1` and `git diff --check` passed.
