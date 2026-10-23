# T7 S117: V30 Execution-Path Decision

## Result

No production change is retained.  The bounded source review identified one
plausible repeated cost, then rejected it before implementation because it is
part of the existing public CPU observation contract.

## Reviewed Candidate

`src/x86-core/cpu/cpu_instructions.c` creates a completed instruction
observation after successful timing selection.  MyRaidenII's deterministic
route uses the fault-only diagnostic provider and does not subscribe to
per-retirement observations, so skipping that copy initially appeared to be a
safe no-observer optimization.

It is not safe.  `x86_core_cpu_execution_copy_observation()` explicitly
promises to preserve completed facts through asynchronous entry until the next
refresh.  Existing CPU and Machine callers use that public function.  The copy
therefore protects callers from an interrupt or other entry that mutates the
live CPU state later in the same refresh.  A current receiver's lack of a
subscriber is not authority to weaken that contract.

The remaining reviewed paths have distinct responsibilities: `ExecIns()` is
the single V30 execution/decoder route; the timing selection preserves the
accepted timing model; transaction tracing retains Machine bus-locality phases;
and the already-disabled diagnostic/retirement publication paths were handled
by S114.  Static code size is not runtime attribution, and the available
MinGW profiling attempt from S116 produced no usable function samples.

## Measurement And Disposition

The stable selected-set checkpoint remains 8,000,000 instructions,
133,667,500 guest ticks and frame digest `0364633fdce1196d`.  The ordinary
x64 Release measurement is 20,223.1 ms.  Release already uses `-O3`; S116
measured IPO at about 9.4% faster on x64 but about 72% slower on x86, so it is
not a cross-width release policy.

No candidate satisfying the task's no-JIT, single-executor and exact-timing
constraints remains.  Reaching real-time host throughput requires a separately
approved execution-strategy design rather than another micro-patch.  This S
does not claim gameplay, desktop/RDP or audio qualification.

## Cleanup

No diagnostic process or task scratch was created by this source review.  No
ROM payload, profiler data or generated output is retained.
