# M5 T48 S1: Naming Convergence Plan

T48 S1 is complete as the design subtask for the first naming-convergence
implementation task. It added the M5 naming convergence plan, froze T48
through T54 task scope, recorded the `0.5.NNNN` task-version rule, and updated
artifact naming to `build/output/nxvm_0_5_NNNN.exe` for runnable NXVM tasks.

No source files, CMake target paths, or runtime behavior changed. No
executable artifact is produced for S1; T48 produces
`build/output/nxvm_0_5_0048.exe` only after S2 completes the runnable rename.

## Checks

- Exactly one active subtask remains after closure: M5 T48 S2.
- M6 T1 is deferred and not active.
- The stale `device.h` current-boundary claim in module layout is removed.
- The active implementation subtask has explicit gates and stop conditions.
