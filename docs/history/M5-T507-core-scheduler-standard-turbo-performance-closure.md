# M5 T507: Core Scheduler Standard/Turbo Performance Closure

T507 consolidates admitted controller deadline contracts into the sole Core
scheduler. It is a performance and correctness convergence task: Core remains
the only guest-time writer; Standard is solely a host limiter on completed
Core progress; Turbo removes that wait and nothing else.

## Accepted Subtasks

| Subtask | Accepted result |
| --- | --- |
| S1 | Pending coordinator acceptance: freezes C1--C13 and V1--V4, including same-tick order, cancellation and explicit L1/L2 receivers. |

The next implementation batch may consume only this finite matrix. It must
not create a VM/profile scheduler, device-local queue, host-to-Core tick
route, guessed duration or parallel fast path.
