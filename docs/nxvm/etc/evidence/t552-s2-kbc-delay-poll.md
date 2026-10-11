# T552 S2 KBC Reply Delay And Status-Poll Repair

## Owner And Defect

`core/chips/kbc8042/controller.c` owns both reply delay and the profile-owned
visibility-poll ordering. Before this repair, `advance()` reduced a response
delay only after visibility polls were exhausted, while `read_status()`
exhausted polls only after the delay had reached zero. A nonzero value for both
states was therefore a circular wait.

## Change

The existing pending-response block in `x86_kbc8042_advance()` now always
reduces `response_remaining_ticks`. Reply publication remains gated by
`response_status_polls_remaining == 0`, plus the pre-existing serial/FIFO
conditions. No state, API, queue, board behavior, profile rule or scheduler
path was added.

The code change is three added and three removed source lines. It preserves
the established order:

1. elapsed time reaches reply readiness;
2. the host observes the configured number of status polls;
3. the existing serial/FIFO publication condition makes the reply visible.

## Owner-Local Regression

`controller_contract_smoke.c` adds one 43-line contract case using a two-tick
delay and one visibility poll for each reply origin:

- controller command `20h` returns command byte `43h` without IRQ1;
- keyboard `F2h` returns `FA AB 41` through IRQ1, retaining controller-side
  identification translation;
- AUX `D4h/F2h` returns `FA 00` through IRQ12 and AUX status.

Each case proves that the first tick and status read leave OBF clear, the
second tick makes the delay ready but does not itself expose output, and the
next status read releases the configured poll gate. The previous circular
implementation fails this sequence because neither gate changes.

## Verification

- Core source manifest and corpus boundary gates: pass.
- Focused KBC controller, serial-cadence and board-controller routes: 3/3
  pass on x64 and 3/3 pass on x86.
- All eight PC profile artifacts were rebuilt in their existing profile-specific
  Ninja directories; each artifact build reported the expected architecture.

S2 intentionally does not claim serial/FIFO deadline correctness. That
separate mechanism remains S3.
