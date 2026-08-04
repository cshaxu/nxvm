# M5 T201 Host Cancellation Boundary Verification

## Contract

The platform run handle is the only host-cancellation report boundary. A
backend borrows its live handle and reports `STOP_REQUESTED`; the session
runner consumes that report at its execution boundary and performs the single
session-control stop operation. Keyboard transport carries only modifier reads
and guest key presses.

## Evidence

- `vm-host-cancellation-smoke` invokes the Windows F9 handler and proves one
  run-handle report plus retained guest F9 ingress behind its keyboard-state
  snapshot.
- `vm-platform-run-handle-smoke` proves runner-side consumption stops the
  session through the established boundary.
- `verify-host-cancellation-boundary` rejects the former keyboard callback and
  operation, direct platform session/core stop calls, missing Win32/Linux
  `STOP_REQUESTED` reports, and missing Win32 backend owner borrows.

## Full Gate

Windows GCC current CTest passed 42/42 cases. Linux source/CMake contract
verification passed, while native Linux runtime validation remains explicitly
deferred. `nxvm_0_5_0201.exe` retained the Console banner and piped `EXIT`;
SHA-256:
`95499E795F79AE5ED3D32283A57D22FB099C0E93FA0A53E8B14DF734F98B5C76`.
