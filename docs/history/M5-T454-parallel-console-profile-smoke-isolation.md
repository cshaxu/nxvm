# M5 T454: Parallel Console Profile-Smoke Isolation

T454 isolates the current-gate Console profile-smoke fixtures so parallel CTest
execution cannot share a catalog or profile workspace.

## Active Subtask

- S1 inventories every affected fixed write and discovery caller, gives the
  lifecycle, Model-40, and memory-roundtrip same-mechanism smokes owned
  workspaces through existing test setup, and proves serial plus repeated
  parallel execution. Its evidence will record the exact path ownership and
  residual-file sweep.
