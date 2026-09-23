# M5 T454: Parallel Console Profile-Smoke Isolation

T454 isolates the current-gate Console profile-smoke fixtures so parallel CTest
execution cannot share a catalog or profile workspace.

## Active Subtask

- S1 inventories every affected fixed write and discovery caller, gives the
  lifecycle, Model-40, and memory-roundtrip same-mechanism smokes owned
  workspaces through existing test setup, and proves serial plus repeated
  parallel execution. Its evidence will record the exact path ownership and
  residual-file sweep.

## Closure

The three registered CTest cases now each own one build-tree workspace; no
product catalog route, Console selection, or standard-input path changed. The
[S1 evidence](../etc/evidence/t454-s1-parallel-console-profile-smoke-isolation.md)
records the complete sweep, serial control, three parallel executions and
source-root residue check. The independent 8086 decoder-ledger verifier failure
transfers once to `TODO.md`; it is neither hidden nor attributed to this
fixture repair. The consumed [proposal](M5-T454-parallel-console-profile-smoke-isolation-proposal.md)
is retained beside this record.
