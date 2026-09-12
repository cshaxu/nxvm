# M5 T528 S4 Provider-Lifecycle Cleanup

Baseline: `ac554fb0` after accepted T528 S3.  This is a subtraction-only
ownership correction.  It does not change Core execution, Common FIFO or
paused-debug lease semantics, profile topology, firmware mapping, image modes
or host synchronization.

## One Owner

`src/vm/machine/runtime/provider_lifecycle.{c,h}` contained no state or
independent ordering.  Its entire contract was forwarding these calls to
`machine_devices`:

| Former facade operation | Final direct owner and call site |
| --- | --- |
| initialize | `vm_machine_devices_initialize_media()` followed by `vm_machine_devices_bind_media()` in `runtime/control.c` |
| reset | `vm_machine_devices_reset()` in the existing Core execution-provider reset callback in `runtime/lifecycle.c` |
| finalize | `vm_machine_devices_finalize()` in `runtime/control.c` |

The initialization order remains media initialization, registry bind/freeze,
then Core execution-provider binding.  Reset remains a Core execution-provider
callback, and finalization remains control teardown.  Thus this removes a
forwarder without making device state visible to another owner or creating a
second provider route.

The exit sweep found no production, test, CMake or tool reference to a
provider-lifecycle source or symbol.  The only retained textual references are
negative verifier checks that require its absence.  `machine_devices` is the
single FDD/HDD provider lifecycle owner.

## Verification

- Focused CTest: `vm-machine-media-lifecycle-s3-smoke`,
  `vm-machine-initialization-atomicity-smoke`, and
  `vm-machine-reconfigure-smoke` all passed.
- Full repository-only unit suite: **299/299 passed** with
  `ctest --parallel 8`.
- `verify-current-specialized-gates` passed, including provider-composition,
  media sole-route, deadline scheduler, Core/VM authority and direct-build
  ownership gates.
- `git diff --check` and the provider-lifecycle/source-owner sweep pass.

## Retained Boundary

This S does not consume the proposed Common executor work.  Common already
owns the copied request FIFO, generation, wake and paused-debug lease; NXVM
continues to own its bounded-quantum Core runner, while SoftPC keeps its
CCPU/timer-rendezvous runtime.  No generic worker is introduced here.
