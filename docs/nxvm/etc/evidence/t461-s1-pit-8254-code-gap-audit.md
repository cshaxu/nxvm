# T461 S1: 8254 Current-Code Gap Audit (List 2)

| Ledger rows | Current owner and observed disposition | Next receiver |
| --- | --- | --- |
| P1--P4 | `src/core/machine/pit.c` is one three-counter owner with ports, RW sequencing, binary/BCD conversion and zero reload expansion. Focused read-back smoke covers only a subset. | S2 unified chip phase contract. |
| P5--P7 | `pit.c` has one count latch and one status latch per counter, but its interleaved latch/read ordering needs manual-form tests. | S2 unified chip phase contract. |
| P8--P13 | `pit.c` has six mode-specific tick functions and one gate entry. Its present load is immediate on completed count write, while the manual requires mode-specific next-CLK load/defer semantics. No complete mode/gate/rewrite corpus exists. | S2 unified chip phase contract; these coupled rules cannot be partitioned into invalid intermediate mode subsets. |
| P14 | Control-write resets local fields/output in the sole owner, but initial OUT/null-count and partial-write behavior lack complete proof. | S2 unified chip phase contract. |
| P15 | `machine.c` owns the sole channel-0 OUT-to-PIC binding; scheduler owns `DMA -> PIT -> PIC`. No second route found. Exact output-to-IRQ visibility tests are incomplete. | S3 integration and closure. |
| P16--P18 | Current deterministic reset and clock domains are functional L2 policy. No selected physical input frequency, oscillator phase, PPI/speaker or electrical claim exists. | Retain L2; transfer only to the queued Core integration/time-plan task or a separately admitted speaker topology task. |

No code is changed by S1. The implementation must replace incomplete
mode-local behavior in the existing `pit.c` owner, never add a parallel timer,
profile-side state machine or host-time callback.
