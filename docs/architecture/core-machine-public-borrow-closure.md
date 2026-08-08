# Core-Machine Public Raw-Borrow Closure

## M5 T299 S1 Frozen Map

T299 closes the remaining public raw-state access surface after T295--T298.
`core_machine` owns the one CPU, instruction state, executor context, RAM,
port table, and PC/AT controller storage for its complete lifetime. No product
or composition code borrows that storage. T297 firmware uses its opaque
capability and T298 debugger work uses copied observations and named operations.

| Declaration family | Existing production consumer | Test consumer | Replacement | Owner and lifetime |
| --- | --- | --- | --- | --- |
| `configuration_cpu_borrow`, `configuration_cpu_instructions_borrow`, `configuration_cpu_execution_borrow` | None | CPU, protected-mode, task, paging, FPU, and VM probe setup corpus | Fixture-specific CPU preparation/inspection operation | Core owns state; fixture call has a caller-owned `core_machine *` and ends before test teardown. |
| `configuration_memory_borrow` | None | Reset/vector setup and memory assertion corpus | Public checked memory operations, or fixture preparation only where the existing test requires private initialization | Core owns RAM; public calls copy bytes. |
| `configuration_port_borrow` | None | PIT and timing setup corpus | Named public port operation, or fixture-specific port preparation | Core owns the port table. |
| `configuration_shared_pic_*`, `shared_pit`, `shared_dma_*`, `shared_kbc` borrows | None | Storage/isolation and one shape fixture | Fixture-specific controller inspection only | Core owns controller storage; no product caller retains it. |
| `debug_cpu_borrow`, `debug_cpu_instructions_borrow`, `debug_cpu_execution_borrow` | None | VM outcome/timer/video setup and CPU diagnostic corpus | Existing copied debug observations/named operations, or fixture-specific setup where those cannot express a test precondition | Core owns state; T298 operations are paused/stopped and copy or commit named fields. |
| `debug_memory_borrow`, `debug_port_borrow` | None | Memory/video/timing observation corpus | Existing debug memory/port operation or fixture assertion | Core owns RAM/ports; no raw pointer leaves a public header. |
| profile-binding raw accessor | None | None | Delete; T297's opaque firmware capability is the sole retained firmware binding | Core invokes the provider synchronously; provider context expires during teardown. |

## Test Fixture Boundary

`tests/support` may include the private `core/machine/machine.h` definition,
but is intentionally not part of a library target, public header, or product
include path. Its functions each accept the already-owned `core_machine *`;
they have no allocation, global/TLS target, callbacks, retained pointer, or
duplicate machine/executor/state. A fixture is destroyed only through the
ordinary core-machine teardown used by the test.

The adapter must expose purpose-named fixture operations only: reset-vector
preparation; protected descriptor/segment preparation; paging, task, and FPU
prepared-state setup; checked memory image installation; port/controller
fixture setup; and copied fault/state inspection. It exports no `*_borrow`,
`get_*_ptr`, general raw getter, or macro alias. Tests use an existing checked
or named API whenever that API already describes their operation.

## Closure Queries

S1 records the following actual-source query:

```text
rg -n "core_machine_(configuration|debug)_.*borrow|profile.*(borrow|binding)" src tests --glob '*.[ch]'
```

The S3 gate scans all `src/**/*.h` and `src/**/*.c`: it rejects a public raw
CPU/RAM/port/controller/executor accessor or profile binding, rejects raw
borrow names in `src`, and rejects `tests/support` includes from `src`.

## T300 S4 Interface-Shape Recurrence

T300 S4 extends that gate to every `*_interface.h`. It rejects direct private
core-machine implementation headers and complete private CPU, decoder, RAM,
port, controller, or VADP layout names. The retained public values are only
the copied display/controller configuration needed by composition and the
explicit copied debugger observation consumed by the VM debugger.
