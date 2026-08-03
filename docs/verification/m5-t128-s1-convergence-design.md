# M5 T128 S1: Convergence Design

The approved design is recorded in
[`m5-t128-t136-convergence.md`](../planning/m5-t128-t136-convergence.md).
It preserves product entry signatures and UX while forbidding ambient
thread-local invocation selection, and it makes the executor/RAM/port lifecycle
a core responsibility before VDM moves to the shared executable core.

T128 is design-only and creates no executable artifact.
