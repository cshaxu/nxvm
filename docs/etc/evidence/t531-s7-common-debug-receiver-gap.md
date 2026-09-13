# T531 S7 Common Debug receiver audit

## Conclusion

Common Debug and XASM32 are already the sole parser, continuation and
assembler/disassembler owners in NXVM.  NXVM's `vm/machine/runtime/debug_adapter`
maps every Common Machine paused-debug operation to Core through a bounded
lease.  It is a real receiver, not a mock.

SoftPC cannot yet be claimed as a second consumer.  Its read-only public
machine boundary exposes physical memory read/write and instruction address,
but not a paused-debug lease or the rest of the operation universe below.
Adding a no-op adapter in NXVM or returning synthetic values would violate the
single-owner and typed-operation contracts.  The entire SoftPC column is one
receiver to implement in that repository's approved work, not a series of
per-command NXVM patches.

## Frozen operation universe

| Common Machine operation family | NXVM receiver | Existing SoftPC capability | Required SoftPC adapter work |
| --- | --- | --- | --- |
| General/segment/control register read and write | Core debug register APIs via `vm_machine_common_debug_execute` | Only CS:EIP and linear instruction address read | Expose copied paused CPU snapshot plus typed register read/write. |
| Linear and real-mode memory read/write | Core checked debug memory APIs | Physical read/write only | Map linear/real semantics through CCPU's checked translation; retain physical access as implementation detail. |
| Port read/write | Core debug port APIs | None | Expose typed paused port operations through the CCPU/device owner. |
| Code default size/base and CPU snapshot | Core snapshot APIs | CS:EIP/address only | Publish copied code properties and complete CPU/segment snapshot. |
| Read/write/execute watchpoints | Core watchpoint APIs | None | Install, clear and report watchpoints at CCPU execution boundaries. |
| Trace/break execution plans and completion result | Core-owned plan, observed at each executed instruction | Continuous CCPU loop and timer rendezvous only | Consume copied plan at the CCPU safe point, observe each retired instruction, pause and publish result without a nested executor. |
| Lifecycle hand-off | Common Debug returns copied resume/step/stop request to Common Session | Runtime has start/pause/resume/stop | Bind the same Common Session lifecycle sink; do not let Debug call runtime directly. |
| Instruction memory-access observation | NXVM runner copies Core observation into Common Debug | No copied observation sink | Publish bounded CCPU memory-access observations or report the command subset unsupported explicitly during SoftPC implementation. |
| Assemble/disassemble and command continuation | Existing Common XASM32 and table-driven Common Debug command owner | No consumer binding | Register the same Common Debug CLI provider with SoftPC monitor after the target exists. |
| `L`/`W` Debug file commands | Common Debug uses Lib storage API | SoftPC can use the same Lib storage service | Bind after target work; no product file-handle API or callback is needed. |

## Boundaries and proof

- `common/debug/command.c` routes every machine read/write through
  `common_machine_debug_execute_with_lease`; it neither sees a Core nor a
  SoftPC pointer.
- `common/xasm32` has no product dependency and is already used by NXVM Debug
  and recorder.
- `common/machine` exposes copied requests/results and grants a lease only
  when the injected driver declares the machine paused.
- NXVM binds its real driver in
  `vm/machine/runtime/lifecycle.c` and maps the full declared operation set in
  `vm/machine/runtime/debug_adapter.c`.
- Read-only SoftPC `src/host/machine.h` has no register, port, snapshot,
  watchpoint, plan, result, lease or instruction-observation contract.

## Required receiver transfer

The next authorized SoftPC task must add one `common_machine_driver` binding
at its existing CCPU rendezvous boundary.  It must implement every row above,
use Common Session for lifecycle requests, and run real Common Debug command
tests against that adapter.  It must not create a second command parser,
expose CCPU pointers in Common, call a UI thread from the executor, or
substitute no-op/synthetic debug results.

No NXVM source changes are needed for this audit.  The task makes no final
two-consumer completion claim; S8/T531 final acceptance remain blocked on the
receiver and its real integration evidence.
