# M5 T221: Deterministic Timing Calibration

## S1: Checkpoint Contract

**Status:** complete. T221 makes the T217--T220 coarse timing model auditable;
it does not claim cycle accuracy or final PIT waveform compatibility.

### Checkpoint Surface

A core-owned probe must record deterministic checkpoints containing elapsed
ticks, executed-instruction count, run reason, and `3DAh` status at fixed
budget boundaries. Identical reset, profile, program bytes, and budget inputs
must produce an identical ordered checkpoint sequence. The default profile
continues to freeze one tick per completed instruction and the 48/8/8 text
raster tuple.

No host time, thread scheduling, renderer cadence, or platform handle may be
part of an acceptance value. An optional Bochs/PCjs comparison is only a
documented, bounded status-sequence reference; it cannot set a runtime clock
or replace owned probes.

### S1 Probe Plan

Add a core-only checkpoint smoke that maps deterministic NOP bytes at the reset
vector, samples `3DAh` through the port bus after fixed one-instruction runs,
then repeats the entire sequence after reset. It must assert elapsed tick,
executed count, and status equality, including one full 64-tick default raster
period. Retain T218 fault/no-tick behavior and the normal current matrix.

### Stop Conditions

Stop if calibration introduces host pacing, profile mutation after freeze,
second device timing state, or a Console/debugger/startup behavior change.

### S2 Implementation

`core-machine-timing-checkpoint-smoke` uses 64 reset-vector NOPs and the
existing paused-boundary debug port borrow. For each one-instruction run it
records the run result's one tick and elapsed total, then samples `3DAh`.
It repeats the full reset/program/budget sequence and requires byte-identical
status checkpoints. The default 48/8/8 tuple is pinned at active display for
checkpoints 1--47, horizontal blank for 48--55, vertical retrace for 56--63,
then active display again at checkpoint 64.

### S3 Evidence

The deterministic checkpoint probe passes. The current GCC/CTest matrix passes
56/56 tests, including FDD prompt, keyboard, `MEM`, video-port, HDD boot,
Console, debugger, and the retained timing/device probes.

Developer artifact: `build/output/nxvm_0_5_0221.exe`.

SHA-256: `527FC2845DBDF46D8A83516755D0A49BE62BBD83C898FE0EAB9332C6EEFEBEF4`.

Source commits: `5455b61` (`M5 T221 S2 P1`) and this S3 evidence record.

### Later T222 PIT Relative-Rate Evidence

T221 deliberately did not claim PIT waveform correctness. T222 records the
additional deterministic relation without changing T221's 48/8/8 VADP tuple:
given the same frozen elapsed-tick input, mode 2 reload `N` repeats every `N`
ticks after its initial low pulse, and mode 3 reload `N` repeats every `N`
ticks with ceil/floor high/low phases. This is core scheduler evidence, not a
host-time calibration.
