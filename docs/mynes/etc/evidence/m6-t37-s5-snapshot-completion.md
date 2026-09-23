# M6 T37 S5 Snapshot Completion Disposition

The committed SoftPC corpus `6251f896` is the reference for App completion
ordering. It performs the stream operation synchronously, but it does not
report a running-machine save or a successful load at that return point:
Common still owns the later pause/start completion. SoftPC records a pending
snapshot result and reports it when the runtime callback reaches `PAUSED`.

MyNes now uses the same ownership boundary. A paused save remains immediately
complete. A running save and a successful stopped load retain one
`pending_snapshot` result while `transition_pending` blocks another command.
The runtime `PAUSED` fact turns that result into the existing deferred monitor
outcome. Failed operations that do not lead to a pause remain immediate errors.

The implementation adds no worker, queue or forwarding interface: Common's
existing state stream and Session runtime-completion path are already the
asynchronous boundary. The direct integration test proves both delayed cases,
and the x64/x86 Core snapshot, transaction and App command tests pass.
