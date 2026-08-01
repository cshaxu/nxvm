# M5 T12 S3 Inactive Request Bridge Verification

`platform/vm/request_bridge` provides a caller-owned copied FIFO for keyboard
state, key press, stop, and display-mode requests. It has no Win32 include, no
machine global include, no producer, and no consumer. The new smoke verifies
copying, FIFO order, full-queue rejection, re-enqueue after dequeue, and
invalid output handling.

GCC built the user-facing target and focused smokes. The request-bridge smoke,
FDD-backed execution lifecycle, sequential CPU probe, FDD/HDD reset-vector
smoke, no-media Console, and debugger-prompt regressions passed. The raw
recorder was not enabled. Since no retained producer or execution loop uses the
bridge, user-visible NXVM behavior is unchanged.
