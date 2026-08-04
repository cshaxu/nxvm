# M5 T196 Linux Source Convergence

## Closed Scope

T196 closes the Linux code and CMake parity needed for the session-owned
platform run-handle. Non-Windows CMake selects only Linux platform sources,
requires `Curses` and `Threads`, and no longer requires PowerShell merely to
configure. The Linux Console reports terminal startup failure, waits for a
ready/failure result, uses joinable pthreads, and keeps terminal/lease/backend
destruction in the finalizer. F9 reports cancellation to composition rather
than directly controlling guest execution.

The new Linux smoke performs two terminal start/report-stop/join/finalize
cycles and requires a cleared run owner after each. `Verify-T196-Posix.sh`
builds it under a pseudo terminal in a native environment.

## Evidence

- `verify-linux-platform-contract` emitted
  `M5:T196:S1:LINUX-PLATFORM-CONTRACT:OK`.
- The shared `vm-platform-run-handle-smoke` emitted
  `M5:T194:S3:RUN-EVENT:OK` after removal of its Win32-only sleep dependency.
- The 12-target Windows GCC regression suite passed after the source/CMake
  change. `nxvm_0_5_0196.exe` printed its `0.5.0196` identity and accepted
  piped `EXIT`; SHA-256:
  `EA897F13DE140C55D2366815BD8931519D28640789737298A777D5DE0D57010D`.

## Explicit Limit

The owner approved deferring native POSIX compilation and terminal runtime
verification. No WSL installation was performed. The deferred native harness,
terminal startup failure, F9, lease, and DOS-prompt checks remain in `TODO.md`.
This record is Linux source/CMake evidence, not a Linux support claim.
