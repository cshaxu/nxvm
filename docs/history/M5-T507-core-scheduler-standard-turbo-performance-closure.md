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
| S5 | Accepted: complete 8253/8254/List 1/List 2 re-audit confirms one PIT owner and direct IRQ0, DMA0-refresh, speaker and D4-consumer route. No new PIT defect is confirmed; focused 9/9, full unit 312/312 and documentation governance pass. |
| S6 | Accepted: MC146818A RESET retains the sole calendar/divider phase instead of moving the next update deadline; focused RTC 9/9, full unit 312/312 and documentation governance pass. |
| S7 | Accepted: rendered Intel/IBM and external-reference reconciliation retains distinct 8042 and XT PPI owners, removes the false IBM 5170 AUX/IRQ12 profile route, and retains generic default-AT AUX in the same KBC owner. Focused 9/9, full unit 312/312 and documentation governance pass. |
| S8 | Accepted: rendered Intel 8272A and available-local external reconciliation corrects the sole FDC byte-service formula to its explicit FM/MFM values, retaining one controller, DMA2/IRQ6 path and Core deadline receiver. Focused FDC proof, full unit 312/312 and documentation governance pass. |

The next implementation batch may consume only this finite matrix. It must
not create a VM/profile scheduler, device-local queue, host-to-Core tick
route, guessed duration or parallel fast path.
