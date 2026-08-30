# M5 T507: Core Scheduler Standard/Turbo Performance Closure

T507 consolidates admitted controller deadline contracts into the sole Core
scheduler. It is a performance and correctness convergence task: Core remains
the only guest-time writer; Standard is solely a host limiter on completed
Core progress; Turbo removes that wait and nothing else.

## Accepted Subtasks

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: freezes C1--C13 and V1--V4, including same-tick order, cancellation and explicit L1/L2 receivers. Documentation governance and unit 312/312 pass. |
| S2 | Accepted: idle unconfigured FDC and idle HDC no longer receive scheduler work or trace records; active controller order remains Core-owned. Focused regressions and unit 312/312 pass. |
| S3 | Accepted: complete 8259A source/List 1/List 2 audit removes the obsolete scheduler-side slave-cascade L1 fallback. Focused PIC 6/6, full unit 312/312 and documentation governance pass. |
| S4 | Accepted: complete Intel/IBM/List 1/List 2 re-audit confirms one DMA owner and scheduler observation path for request, arbitration, phase, refresh, FDC DMA2 and Xebec DMA3. No new DMA defect is confirmed; focused 5/5, full unit 312/312 and documentation governance pass. |
| S5 | In review: complete 8253/8254/List 1/List 2 re-audit confirms one PIT owner and direct IRQ0, DMA0-refresh, speaker and D4-consumer route; no new PIT defect is confirmed pending focused and full-unit verification. |

The next implementation batch may consume only this finite matrix. It must
not create a VM/profile scheduler, device-local queue, host-to-Core tick
route, guessed duration or parallel fast path.
