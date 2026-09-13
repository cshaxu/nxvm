# M5 T531 S8 Common Debug executor contract

## Finding

The current Common Machine `debug_execute_with_lease()` validates a paused
lease but then directly calls the adapter's `execute_debug` callback on the
monitor/CLI caller thread. NXVM's paused Core boundary permits that today, but
SoftPC's restored CCPU state is executor-thread/TLS-owned. Making SoftPC
access CCPU from the monitor would create a second machine-mutation route and
would not be a valid Common consumer.

## Required shared contract

`common_machine_debug_execute_with_lease()` remains the synchronous,
product-neutral caller API. It retains its current paused-lease validation and
direct typed adapter callback. Common must not acquire a second Debug FIFO,
worker, executor-safe-point requirement or native thread handle merely because
one consumer has stricter CPU affinity.

The adapter callback itself is synchronous: it returns a copied status/result
to Common before `common_machine_debug_execute_with_lease()` returns. NXVM
may complete it directly through its paused Core boundary. SoftPC must keep
the same interface but synchronously relay a copied request to its existing
CCPU executor rendezvous, wait for a copied reply, and return that reply. The
relay is SoftPC adapter machinery, not another Common route.

## SoftPC relay protocol

The SoftPC adapter owns one bounded request/reply slot protected by its
existing runtime synchronization. A Common Debug callback first confirms the
runtime is paused and no prior Debug request is outstanding, copies the typed
request, signals the existing executor command wake, then waits for the copied
completion. The CCPU rendezvous takes that slot before returning to its paused
wait, executes the request against CCPU-owned state, copies status/result,
clears the slot and signals completion. A resume, reset, stop or shutdown
invalidates an outstanding request and wakes its caller with failure. This is
one synchronous callback transaction, not a new product command grammar or a
second executor.

## SoftPC adapter map

At its existing `app_runtime_executor_event()` CCPU rendezvous, SoftPC will
service the adapter-owned Debug relay. Its driver maps:

- ordered input/lifecycle/media requests to the existing runtime and original
  host-machine owners;
- `is_paused` to the executor-published paused state; and
- the synchronous `execute_debug` callback to a request/reply completed only
  by that rendezvous.

The adapter must implement the already-declared typed register, memory, port,
snapshot, watchpoint, execution-plan and result operations. Debug command
parsing remains Common Debug; no SoftPC parser, synthetic snapshot or direct
UI-to-CCPU call is allowed. NXVM continues using the same Common API directly
at its established paused Core boundary.
