# T531 S8 Common Machine Readiness Wake

## Finding

`common_machine` already owns the one copied safe-point FIFO. NXVM's bounded
runner waits on its internal event, but SoftPC's continuous CCPU executor
waits on its own rendezvous. An internal-only Common event cannot wake that
executor after a host request is copied into the Common FIFO.

## Contract

`common_machine_driver.wake_request` is optional and has no request payload or
return value. After `common_machine_submit()` copies a request and changes the
FIFO from empty to nonempty, it releases the Common lock and calls that hook.
It never calls the hook for a nonempty-to-nonempty enqueue. The hook merely
notifies an already-owned product executor; it cannot consume a request,
mutate Common state, create a worker, or provide a second execution path.

NXVM binds no wake hook because its runner already waits on Common's event.
SoftPC will bind its existing persistent CCPU executor wake at its product
adapter. The executor then consumes the same Common FIFO at its existing safe
point. This preserves one executor and one ordered request owner per product.

## Proof

`test/common/machine/common_machine_smoke.c` proves the first enqueue wakes,
the following nonempty enqueue does not, and a later enqueue after drain wakes
again. Its final wake immediately enters `observe_safe_point()` and sees the
fully copied removable-media request, proving Common releases its lock before
notifying an executor. `test/common/common_adapter_conformance.c` proves a
real neutral driver can bind the hook alongside the unchanged synchronous
paused-Debug callback.
