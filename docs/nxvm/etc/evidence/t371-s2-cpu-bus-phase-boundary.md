# T371 S2: 80286 CPU And Bus Phase Boundary

## Decision

The Model-339 executor has one successful-retirement clock publisher.  After a
successful instruction, it records the selected 80286 source cost and advances
the deterministic timeline.  Intel 80286 Appendix-B rows, as accepted by
T368, describe execution under the manual's no-bus-delay assumption.  They
therefore establish the CPU-side retirement contribution but do not select
prefetch refill, instruction fetch, memory/ROM/CGA/ISA READY, port-I/O wait,
or interrupt-delivery phase length for this IBM board.

The shared transaction owner exposes a usable model vocabulary -- CPU/DMA
begin, commit, cancel, logical HOLD request/acknowledge/release and copied
trace -- but it has no elapsed duration field.  That omission is correct at
this boundary: neither the selected IBM board material nor current Intel
authority maps those vocabulary elements to project elapsed ticks.  No S2
runtime value, prefetch policy or wait insertion is admitted.

## Source-to-model matrix

| Boundary | Retained source fact and current owner | S2 phase disposition |
| --- | --- | --- |
| Instruction execution | Intel Appendix B selected 80286 rows; the executor is the sole successful-retirement publisher. | Retain source tick cost as CPU execution contribution only. It is not a bus-cycle ledger. |
| Effective address and odd word | Appendix B additions are already constructed at the same publisher. | Retain the source-defined CPU cost; do not infer physical memory cycle count or READY state. |
| Fetch/prefetch | 80286 instruction completion and decoder paths consume instruction bytes through current memory access. | No selected source maps queue size/refill or fetch overlap onto Model-339 elapsed ticks. Keep as S3/S5 phase transfer. |
| Memory/ROM/CGA/ISA access | Checked memory, immutable ROM and VADP/port owners retain one transaction boundary. | IBM topology selects surfaces, not numeric waits. No synthetic per-region wait table. |
| Port I/O | Appendix B selected IN/OUT rows and shared port provider retain protocol ownership. | Instruction cost is not device completion or I/O bus duration; device service remains separate. |
| Fault/interrupt delivery | The run loop handles delivery before successful-retirement publication. | Non-retiring delivery remains outside the source-cost publisher; INTA and propagation move to S3. |
| Reset/cancellation/trace | Transaction reset cancels/releases; trace is copied by the machine adapter. | Deterministic lifecycle order remains observable; no reset or bus settling time is allocated. |

## Sweep and S3 receiver

The sweep reviewed the 80286 source-cost constructor, executor retirement and
elapsed-tick publication, fetch/decode and memory/port accessors, shared
transaction begin/commit/cancel paths, Model-339 timing configuration, and the
80286 ledger, transaction, timeline and competition smokes.  It found one
retirement publisher and one transaction owner; no second clock, prefetch
queue owner or implicit wait insertion exists.

S3 receives DMA/HOLD, PIT/PIC and logical interrupt-acknowledgement phase
composition.  It must preserve this S2 boundary: a trace sequence or logical
transaction phase does not itself establish HRQ/HLDA, INTA or READY duration.
**This S makes no 5170 Model-339 L3 or physical-cycle claim.**
