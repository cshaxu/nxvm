# M5 T12 S5 Inactive Synchronized Request Transport Verification

`adapters/nxvm_baseline/vm_request_transport` owns separate copied ingress and
egress FIFOs guarded by a C11 atomic lock. It accepts requests until close,
permits an existing queue to drain after close, and discards both queues during
teardown. The transport has no Win32, QDX, firmware, or execution-loop caller.

The focused smoke verifies direction separation, FIFO copying, overflow,
close-time rejection, post-close drain, and discard. GCC built the user-facing
target and focused smokes. The transport and request-bridge smokes, FDD-backed
execution lifecycle, sequential CPU probe, FDD/HDD reset-vector smoke,
no-media Console, and debugger-prompt regressions passed. No raw recorder ran
and no user-visible NXVM behavior changed.
