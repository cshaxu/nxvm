# M5 T509: Session Deadline-Consumer Convergence

T509 closed the finite session-runner audit after T508 exposed an obsolete
immediate-completion test assumption.

## Accepted Result

The [S1 ledger](../etc/evidence/t509-s1-session-deadline-consumer-ledger.md)
exhausts all 33 direct session-runner test sources and the single production
runner. Seven long-running source files with eight consumer loops now use the
existing `vm_session_waiting_advance()` boundary after HLT. Six sources were
already compliant; four intentionally assert HLT; sixteen are bounded
non-continuation probes. Core-only tests remain direct-Core tests by design.

The retained production path is unchanged: Core publishes and advances its own
deadline; VM reads that observation and asks Core to advance. No Core source,
public ABI, controller state, runtime scheduler, timing setter, or VM tick
writer was added. The T509 tracked source/test delta is 64 added, 0 removed,
net +64 lines; documentation is excluded. The additions are direct calls to
the pre-existing owner boundary in each distinct test runner, rather than a
test-only forwarding layer.

## Verification

Focused repaired cohort: 7/7 passed. Complete repository unit (313/313) and
integration (20/20) gates passed. Documentation governance passed. The
stripped Release `nxvm_0_5_0508.exe` was reproduced without work and remains
SHA-256 `7350EF960A3C611F0A74696E153F23BE73D1824A8E7FED1486A3995206F8DE30`.
