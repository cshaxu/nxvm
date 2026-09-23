# M5 T470 Core Physical Time Axis And Standard Pacing

T470 closes the qualification boundary for host-paced guest execution without
inventing a physical rate. S1--S4 consolidate the one Core-owned physical-axis
contract and its eligible writers; S5 proves default PC/AT, Model-339 and
Model-40 each remain unqualified. S6 makes the product distinction honest:
Standard retains the existing non-advancing L2 HLT host-load backoff, while
Turbo omits only that wait.

The task does not convert host elapsed time into guest ticks, select a nominal
CPU/PIT rate, create a second scheduler, or claim exact wall-clock pacing.
The next receiver is the explicit [profile physical-timebase closure debt](../states/TODO.md): it must close the CPU, transaction, controller and
oscillator/divider evidence chain before Standard may use an ahead-only host
wait against copied Core progress.

The stripped Release `build/output/nxvm_0_5_0470.exe` is built by target
`vm-0-5-0470`, reports version `0.5.0470`, and has SHA-256
`8DAC84FE2F05CAF7718E2B8E3F6DAC6248CF758FCA4F9AB956204FE16656E56C` from
source commit `f5f68aed`. The focused pacing smokes, full current gate,
295/295 direct CTest suite and documentation governance pass at closure.
Detailed reconciliation is retained
in the [S6 closure audit](../etc/evidence/t470-s6-integration-closure-audit.md).
