# M1 T7 S3 — Optional Host PCM Output

## Cause and scope

T7 S2 supplied a verified temporary adjacent-INI bundle to the real Product
executable.  Both widths exited with code 1 before the ten-second observation.
The composition path creates a Lib PCM stream before it creates Machine, while
Machine previously rejected a null stream and unconditionally invoked stream
operations.  That made a missing/unavailable host endpoint a startup blocker.

S3 changes only MyRaidenII Product/Machine behavior:

- Product converts the existing `LIB_STATUS_IO_ERROR` and
  `LIB_STATUS_UNSUPPORTED` stream-creation results to the no-host-output mode.
- The one Machine Audio bridge accepts a null stream, drains Board PCM and
  discards it locally.  It never calls a Lib stream operation without a stream.
- A real stream keeps its existing enqueue, partial-prefix, wait, clear,
  heartbeat, cancellation and destruction paths.
- Other stream-creation results, such as `NO_MEMORY`, remain fatal.

No Lib, Emulator, Core, API, timing model, audio synth, retry service, second
queue or second worker changes.  The fallback is **L1 host policy**: it has no
primary-manual or external-hardware claim.

## Verification

- Focused Machine audio, Machine driver and Product composition tests pass on
  x64 and x86.
- Complete `ctest -L myraidenii` selections pass 12/12 on x64 and 12/12 on
  x86.
- The existing `myraidenii_0_0_0006` Product executable was rebuilt on both
  widths.  With the same verified temporary selected-set bundle used by S2,
  each process remained live for ten seconds and was then intentionally stopped:
  `MYRAIDENII:PRODUCT-LAUNCH:X64:OK` and
  `MYRAIDENII:PRODUCT-LAUNCH:X86:OK`.
- The external diagnostic directory had no remaining child or Product process
  after each run.

The launch observation proves absence of S2's immediate construction exit for
these host conditions.  It does not prove audible sound, rendered-window
correctness, input handling, gameplay, protection behavior or release fitness.
