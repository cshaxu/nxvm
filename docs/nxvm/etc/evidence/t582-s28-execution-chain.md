# T582 S28 — execution-chain audit (in progress)

## Causal chain

`x86_core_machine_execute_budget()` checks stop, reset, debug, refresh and bus-hold conditions at the instruction boundary. It calls `x86_core_cpu_execution_refresh()` and obtains the CPU timing result. Fault or asynchronous-entry work does not necessarily retire an instruction; external wait and event time may be published separately. Normal retirement is qualified and observed before `x86_core_machine_publish_elapsed_ticks()` publishes its time. Bypassing these boundaries can change faults, interrupt timing, budget exits or debug behavior.

`x86_core_machine_advance_scheduler()` observes the timeline, FPU, CPU wait and board deadlines, chooses the next due tick, advances the shared timeline and arbitration, then calls the board before-internal phase, internal FPU/CPU wait progression and board after-internal phase in that order. The board advances PIT and interrupt readiness before internal time; RTC, keyboard, PIC and video follow. Device callbacks can alter the next deadline. A stable previous deadline alone is not sufficient to batch multiple instructions.

The CPU implementation and board effects must therefore be optimized at their existing owners; removing per-instruction work requires evidence that every observer and mutation boundary remains intact. This record is an audit, not a change to runtime behavior.

## Candidate checked against earlier evidence

The retirement trace computes the linear PC and calls the recorder even when the trace callback is absent. S26 already measured the candidate that guards this computation: [trace audit](t582-s26-trace-audit.md). Its repeated x64/x86 measurements were noisy and did not show a reliable cross-width benefit; the proposed guard duplicates the recorder's admission check. Do not repeat that experiment without a new hypothesis or claim it solves the performance problem. The initial S28 source review rediscovered this candidate before its existing evidence was located; the investigation index now links it.

Further work must identify candidates with clear reduced production complexity or measured material benefit, while proving interrupt, memory, timing, trace and lifecycle ordering. S28 remains active; the whole-product performance objective is not yet met.
