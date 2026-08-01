# M5 T12 S6 Execution-Boundary Binding Verification

The retained device loop now invokes a generic command-boundary callback from
its execution context before debug and machine refresh. The full-PC baseline
adapter initializes an adapter-owned request transport, binds its boundary
observer, and detaches/closes/discards it only during full-PC destruction. The
observer only records that an execution boundary occurred; it does not dequeue,
apply, or publish a request.

GCC built the user-facing target and focused smokes. The transport smoke,
FDD-backed execution lifecycle, sequential CPU probe, FDD/HDD reset-vector
smoke, no-media Console, and debugger-prompt regressions passed. No recorder
ran, no retained producer is connected, and no guest state is changed by the
new callback.
