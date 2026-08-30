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
| S9 | Accepted: the one VADP Attribute output-enable predicate now gates both recognized planar frames and EGA text fallback; no VM/renderer state path was added. Focused proof and unit 312/312 pass. |
| S10 | Accepted: ATA, WD1003, Compaq/WD and Xebec retain distinct HDC personality routes and one unsourced-service L1 boundary; no guessed duration or second controller was introduced. Focused HDC 6/6 and unit 312/312 pass. |
| S11 | Accepted: Model-40 D4 refresh-hold remains a single board request and a single Core transaction before DMA, with its no-duration L1 boundary explicit. Focused 5/5 and unit 312/312 pass. |
| S12 | Accepted: an eligible L1 owner receives the bounded Core compatibility quantum before any unrelated qualified deadline; Standard and Turbo consume that same Core route and differ only in host waiting. Focused 4/4 and unit 312/312 pass. |
| S13 | Accepted: focused scheduler/speed 4/4, unit 312/312 and external-asset integration 20/20 pass. Stripped Release `nxvm_0_5_0507.exe` retains the runtime debugger. |

## Closure

The finite C1--C13/V1--V4 matrix is exhausted: every controller relation is
an immediate owner action, a qualified deadline, or a named L1/L2 receiver.
The surviving L1 cases are HDC service, Model-40 D4 refresh-hold and any
unqualified DMA/board relation; all use the one bounded Core compatibility
quantum and none is hidden behind an unrelated deadline.  No host-to-Core tick
route, device-local queue, profile scheduler, guessed duration or parallel
fast path exists.

Actual-diff review of `65ac274d^..a3574918` found 64 added and 40 removed
production-source lines, plus 117 added and 59 removed test lines; the net
production change is 24 lines.  The retained production paths are the existing
Core scheduler, VADP, FDC, RTC, default-PC/AT composition and VM waiting
consumer.  The positive test delta is two direct owner regressions (EGA output
gating and L1-before-deadline), not a second fixture or runner.

The final product evidence is
[T507 S13](../etc/evidence/t507-s13-product-performance-closure.md):
`nxvm_0_5_0507.exe` is `-O3`/`-DNDEBUG`, has no compiler debug section, retains
the runtime debugger, and has SHA-256
`5975E348032644A29C3876EA072B7C3DB5D7A9ED438842DBA4797CE6FC5CEFDF`.
T507 is closed.  The next candidate is the independent HDC personality
service-deadline closure; it consumes the explicit HDC L1 receiver rather than
reopening this convergence matrix.
