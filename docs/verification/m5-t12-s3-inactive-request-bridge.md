# M5 T12 S3 Inactive Request Bridge Verification

`vm/platform/request_bridge` provides a caller-owned, single-owner copied FIFO
for keyboard state, key press, stop, and display-mode requests. It has no Win32
include, no machine global include, no producer, and no consumer. It is not a
cross-thread transport; S4 defines the required adapter-owned synchronized
handoff before activation. The new smoke verifies copying, FIFO order,
full-queue rejection, re-enqueue after dequeue, and invalid output handling.

GCC built the user-facing target and focused smokes. The request-bridge smoke,
FDD-backed execution lifecycle, sequential CPU probe, FDD/HDD reset-vector
smoke, no-media Console, and debugger-prompt regressions passed. The raw
recorder was not enabled. Since no retained producer or execution loop uses the
bridge, user-visible NXVM behavior is unchanged. The canonical-root follow-up
also rebuilt the user-facing target and finite CPU probe, retaining
`M5:T1:S1:CPU-PROBES:OK`.
