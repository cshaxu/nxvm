# T457 S1: Session Input Restart Recovery

## Defect and owner sweep

The owner reported that `nxvm_0_5_0448.exe` accepts keyboard input on its
first run but not after F9 and a second `START`. The only two F9 host-key
reporters are the Win32 adapter and the Linux console adapter. Both used the
existing run-handle stop report, then continued into their ordinary key-input
submission path. That admitted the host-only F9 command to the session input
transport and allowed it to survive into the next execution epoch.

The run-handle report remains the one lifecycle-control authority: the runner
consumes it and the session owns its reset, join and finalize paths. No KBC,
PIC, request-transport, guest scan-set or Console command code changes.

## Repair and regression

Both F9 reporters now return immediately after emitting their existing stop
report. Thus F9 is host control only and ordinary keys retain the sole platform
input-sink to session-request-transport to KBC path.

`vm-platform-run-handle-smoke` now exercises two complete epochs: it sends F9
through the Win32 keyboard adapter, resets and starts the same session, proves
the second epoch did not receive F9 scan `43h`, then sends `A` through that
same adapter and waits for the existing KBC output state to change. The test
does not add a test transport or mirror session state.

## Verification

- `cmake --build --preset current-gates-gcc --target vm-platform-run-handle-smoke verify-host-cancellation-boundary`
  passed; the focused two-epoch test and host-cancellation structural guard
  pass.
- The existing build directory automatically reconfigured after the artifact
  target advanced. `cmake --build --preset current-gates-gcc --target
  vm-0-5-0457` built the new product.
- `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -L current-gate`
  passed 293/293 in 304.39 seconds, including platform run-handle, multi-window,
  host-input, DOS keyboard and HDD boot checks.

The new artifact is `build/output/nxvm_0_5_0457.exe`, SHA-256
`371B4DE04324A5B4C046A9C69C3C82FCAE7B945D628934B867C2C45C522B1166`.

## Minimalism review

Counted production and test paths are two production source files and one
existing regression test: 35 added lines, 3 removed lines, net +32 by
`git diff --numstat -- src tests`. The product repair is the two early returns;
the remaining test lines replace a report-only stop check with one bounded
two-epoch observation. They observe the owned KBC endpoint rather than create
another input or lifecycle path. The retained run-handle report is live because
the runner remains its sole report-to-stop consumer.
