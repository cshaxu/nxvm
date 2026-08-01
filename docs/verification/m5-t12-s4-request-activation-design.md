# M5 T12 S4 Request-Bridge Activation Boundary Design

The S3 FIFO is an inactive caller-owned container and has no synchronization;
it must not be shared directly by a Win32 thread and the execution thread. The
activation owner is a VM composition adapter, initially
`adapters/nxvm_baseline`, rather than `machine` or `platform`.

The adapter owns two synchronized, copied transports. Ingress carries keyboard
state, key press, and stop requests from `platform/vm` to the machine execution
boundary. Egress carries display-mode requests from default-profile firmware to
`platform/vm`. The execution thread is the only ingress consumer and the
platform thread is the only egress consumer. Neither module includes or mutates
the other's state.

At shutdown, product/runtime first stops new platform submissions, joins the
platform owner, then drains or discards adapter requests under the session stop
result before machine teardown. Full queues reject deterministically; requests
are never executed on the submitting thread. This design changes no runtime
code, enables no recorder, and leaves retained behavior untouched.
