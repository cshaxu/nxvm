# M5 T194 Platform Run-Event Verification

## Closed Scope

The embedded `vm_platform_run_handle` records the last worker event and one
pending cancellation report. Display/input workers report only; the VM session
runner consumes cancellation at its command boundary. Completion events are
observational and cannot stop a later run. Existing session lifecycle remains
the sole requester, joiner, and finalizer.

## Evidence

- Windows GCC `current-gates-gcc` passed all 11 configured gates, including
  retained Console lifecycle, executor closure, session manager, facade,
  dependency-DAG, and lifecycle-contract checks.
- `vm-platform-run-handle-smoke` emitted `M5:T194:S3:RUN-EVENT:OK`. It proves
  `KERNEL_COMPLETED` does not create a pending stop, a stop report is consumed
  exactly once, and a window session stops via the runner before reset and a
  later run.
- `nxvm_0_5_0194.exe` printed its `0.5.0194` banner and accepted piped `EXIT`.
  SHA-256:
  `A04CFBAA34FC13DB6C17440848A0F5AD30DBA9397EFAF60BD120E24109E0ADC0`.
- Static source review places `CloseHandle`, renderer destruction, and
  backend `STD_FREE` only in backend finalizers. The only platform calls to
  `vm_platform_execution_stop_for()` are the explicit request-stop functions;
  display/input code reports events.

## Limit

This closes the common reporting boundary, not native failure-path evidence.
T195 tests Win32 startup failures in disposable test builds; T196 supplies
native POSIX compilation and runtime evidence.
