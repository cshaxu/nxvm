# T375 S4: Explicit Machine-Time Publication

## Decision

Core machine now has one explicit production operation,
`core_machine_advance_time(machine, source_ticks)`. It accepts a positive,
bounded count of already-selected virtual source ticks only while the machine
is stopped or paused. It increments the same `elapsed_ticks` source and uses
the same scheduler path as a successful CPU retirement. It is not a test
operation, a direct device mutation, a guest FIFO shortcut, or a conversion of
host sleep duration into hardware time.

`core_machine_publish_elapsed_ticks()` is the sole internal publication
mechanism. Successful CPU retirement calls it with a CPU-retire trace record;
the explicit source operation calls it without that record. Both therefore
preserve timeline ordering, clock-domain conversion and execution-provider
time callbacks without a second device-time path.

The run loop now snapshots whether the CPU was already halted before its
refresh. If refresh neither delivers an interrupt nor clears HLT, it returns
`CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT` with zero executed instructions,
zero run ticks and unchanged elapsed time. A newly executed HLT still receives
its normal accepted instruction cost. Existing interrupt-delivery timing is
not reclassified in this S.

## Focused Proof

`core-machine-explicit-time-s4-smoke` proves:

- an unstarted machine rejects the operation without mutation;
- a first HLT retires once, but the next no-interrupt run does not re-retire it
  or advance the timeline;
- explicit source ticks advance elapsed time, the same timeline and RTC;
- zero and overflow inputs leave state unchanged; and
- reset reinitializes elapsed time and device state before the operation may
  advance from the stopped lifecycle.

The local focused replay passed:

```text
M5:T375:S4:EXPLICIT-MACHINE-TIME:OK
M5:T256:S3:RATIONAL-CLOCK:OK
M5:T346:S2:TIMELINE:OK
M5:T346:S3:ARBITRATION:OK
```

The timeline regression is material evidence: it confirms the refactor retains
the established `CPU_RETIRE -> DMA -> PIT -> PIC` trace ordering while sharing
the time-publication mechanism.

## Transfer

This S deliberately does not choose when VM composition supplies source ticks,
how it paces them, or whether it observes host duration. Those are product
source-policy questions and must remain deterministic/replayable rather than
be hidden in the HLT sleep loop. It also does not add Model-339 waits, DMA
rate/grant phase, PIC latency, FDC/KBC/CGA service rate, physical waveform or
an L3 conclusion. The next T375 S must bind and verify the selected Model-339
source policy through this sole production operation.

No ROM, media, binary, third-party source, host clock or reference-runtime
scalar was imported or used.
