# T582 S13 — scheduler cost and board deadlines

At the same fixed PC110 startup checkpoint (10,000,000 retired instructions, 79,964,294 guest ticks, PC 0003dbc5), task-owned wrappers sampled one in 2,048 calls using the Windows performance counter. Each run used private media and current cached libraries. Instrumentation adds work and changes executable layout; sampling is deterministic, not random, and may alias periodic device behavior. Values are inclusive, so the nested functions must not be added to their parent.

| Width | Elapsed | Publication calls / samples / ticks | Board deadline calls / samples / ticks | Board before / after sampled ticks |
| --- | ---: | --- | --- | --- |
| x64 | 22,433 ms | 10,012,784 / 4,889 / 35,185 | 10,302,393 / 5,030 / 16,908 | 13,191 / 8,501 over 5,026 samples each |
| x86 | 26,946 ms | 10,008,708 / 4,887 / 46,759 | 10,298,185 / 5,028 / 21,566 | 16,116 / 11,458 over 5,024 samples each |

At a 10 MHz counter frequency, mean sampled publication costs were about 0.72 microseconds x64 and 0.96 x86. Board deadline costs were about 0.34 and 0.43 microseconds. These are investigation estimates, not exact exclusive costs or proof of equivalent optimization. No throughput improvement is claimed.

In these samples, the board returned no L1 compatibility, blocked, immediate, or absent deadline cases. x64 deadlines were 1 tick for 18 samples, 2-8 for 163, 9-64 for 784 and over 64 for 4,065; x86 returned 18, 163, 784 and 4,063 respectively. This suggests many per-instruction re-evaluations occur before a board event, but it is not evidence for late Personaware execution or all device configurations.

The production scheduler advances the timeline, arbitration, board clocks, devices and provider in a definite order at every published CPU interval. A safe consolidation must preserve guest-time state at every observable port/memory access, input, interrupt, snapshot, pause and failure boundary, and invalidate any derived deadline after every relevant state mutation. The source-clock and device phases must remain owned by the existing clocks. An unconditional skip, unchecked cached deadline or a second owner would break the contract. Further measure actual mutation rates and late workload before proposing a minimal change.

No Shared production code, API, media or published executable changed. Desktop/RDP performance was not tested. Logs SHA-256: B939C8EC3F2BF0D301E214783F92F5017BAAC5984685FFFEA66D2B55CDE429F2, 05E21B24071385B0299D4C14F38F80F1FB3C2B5E84D05BBEBB4A1D7570EE38E0, ACB3C59613961F4435D16520374E9910EABED3EC45EB6338C960A9849DD2CCB0.
