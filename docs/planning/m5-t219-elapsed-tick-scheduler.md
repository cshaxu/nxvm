# M5 T219: Core Elapsed-Tick Scheduler

## S1: Contract And Migration Design

**Status:** S1/S2 complete; S3 artifact recording active. T219 moves time-sensitive core device advancement from the
pre-instruction refresh chain into one `core_machine` elapsed-tick scheduler.
It does not change PIT waveform semantics, CGA raster semantics, DMA protocol,
or VM-only device behavior; those are separately admitted tasks.

### Ownership And Order

After every successfully completed CPU execution boundary, `core_machine` adds
the already-frozen coarse tick quantum and invokes core devices in one fixed
order: DMA, PIT, VADP, then PIC visibility refresh. KBC remains event-driven
and may refresh its local queue before CPU execution, but it receives no
elapsed-tick advancement in T219. VM composition's provider refresh remains
VM-only device maintenance and must not advance core guest time or any core
device.

The scheduler receives only `delta_ticks`; devices do not read host wall time.
If CPU execution faults, stops, resets, or cannot begin because its tick budget
is exhausted, it delivers no delta. A completed `HLT` receives its normal
completed-instruction delta before returning the paused boundary.

### S1 Probe Plan

The new scheduler smoke will bind a VM refresh probe and observe that it cannot
advance core elapsed time, assert a fixed core-device order for one completed
instruction, verify zero advancement on fault and pre-execution tick exhaustion,
and retain IRQ visibility through the existing PIC/PIT/KBC/FDD/HDD matrix.

### S2 Implementation And Regression

`core_machine_run()` is the only scheduler path. After each completed CPU
instruction it advances the frozen tick quantum once and calls DMA, PIT, VADP,
then PIC refresh in that order. The former device `refresh` entry points are
renamed to explicit `advance(..., elapsed_ticks)` operations; VM composition
does not call them. KBC remains pre-execution and event driven.

`core-machine-scheduler-smoke` proves that a tick budget smaller than the
frozen quantum stops before the VM refresh provider runs, and that one completed
instruction invokes that provider once while advancing exactly one quantum.
Existing PIT, VADP, PIC, KBC, FDD, HDD, and T218 fault/tick probes retain
device-local and no-fault-advance coverage.

### S3 Evidence

Focused scheduler, PIT, VADP, and real-mode tick smokes pass. The current
GCC/CTest matrix passes 55/55 tests. The T219 developer artifact record is the
remaining closure evidence.

### Stop Conditions

Stop if this needs a second device loop, platform timing, VM-side core-device
calls, duplicate device state, or a Console/debugger/startup behavior change.
S2 changes only the real `core_machine_run()` path. S3 requires focused probes,
the current GCC/CTest matrix, and a T219 artifact record.
