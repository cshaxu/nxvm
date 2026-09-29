# T539 S20 CPU Observation And Board Adapters

Admission baseline d6922b97d; Current owns the active packet. S18 retained much
of this production migration. S20 audits it rather than creating a second API.

## Initial Consumer Audit

- devices/debug.c enforces stopped/paused/faulted access before calling CPU
  operations; that lifecycle boundary is not an unnecessary forwarding layer.
- entry_plan_interface.c validates board memory/preloads around a CPU-owned
  prepared-entry candidate. CPU layout and rollback stay behind that operation.
- retirement_observation_interface.c consumes copied instruction/timing records
  and explicit entry/current snapshots. Its board publication sequence and
  timeline metadata are not live CPU register owners.
- trace_interface.c and memory_interface.c obtain the current PC through CPU
  operations. Machine/board/scheduler request reset, NMI and execution through
  the execution context, rather than modifying register/decoder fields.
- Search of production app-nxvm C files found no executor_cpu.data or
  executor_cpu_instructions field access. machine.h still embeds the original
  storage, and machine.c binds it once; this is the explicit S30 lifetime
  receiver, not an S20 exception permitting new private consumers.
- Board/executor support fixture headers do not directly access those fields.
  The instruction-group fixture and raw test consumers remain assigned to
  S21-S29 by the original-case inventory.

## Added Proof

Extended the existing retirement pre-mode snapshot case without changing its
original assertions. After LMSW retirement, a debug write and CPU reset must
leave the callback's copied entry/current EAX, EIP and CR0 values unchanged;
the current snapshot must instead reflect reset, and reset must not publish
the previous pending instruction again. The x64 registered regression passes.
This is initial proof, not complete S20 verification or acceptance.

The entry-plan board test retained a private cpu.h include solely for IF.
Removed that include and expressed the unchanged test input as 00000200h,
without exporting another constant/API or changing guest behavior.

The existing CPU boundary gate now also scans product production files for
direct accesses to embedded CPU/decoder/execution-context fields and private
CPU includes. CPU-owned files are scoped to their exact directory; machine.h
alone may retain its private include for the original storage until S30.
Five added negative controls inject three field bypasses and two private
includes into the copied board bus source and require the specific rejection.
This does not prohibit public operations or weaken the remaining-test inventory.

Debug code review confirms current/entry snapshot selection happens at the CPU
owner before a value copy, and invalid point selection returns before writing
the output. Board diagnostics retain first-fault and first/last delivered
exception copies. Instruction diagnostics remain separate from fault-only
binding, so ordinary observer-free runs do not acquire a new per-instruction
consumer. The FPU binding borrows the public FPU handle; no private FPU layout
or duplicate FPU state is introduced by the adapter.

No production, public API, Shared, MyNES, INI or executable input has changed.
Remaining work: review the rest of debug/FPU/fault/observer-free adapters and
tests, enforce the allowed private-lifetime exception without hiding consumer
access, then complete full units/gates and actual-diff review.

Both existing build trees are being incrementally built and tested sequentially
with the full unit label. The current run is still live (execution session
18574); x64 has passed the expanded boundary negative test. This is not a full
suite result. No replacement run should start while this handle remains live.
FPU instruction/cache tests still using private fixtures belong to S29 in the
existing inventory; this S qualifies their production binding/diagnostic
adapters, not silently accepting those later test migrations.

## Requirement Review

| S20 requirement | Evidence and disposition |
| --- | --- |
| Entry/current snapshots | CPU capture_snapshot selects current CPU or instruction-entry oldcpu internally; invalid selector exits before output mutation. Retirement test retains both mode-transition snapshots and now proves lifetime across write/reset. |
| Decode and fault records | CPU copy_observation and diagnostic_publish_snapshot construct value records at the owner. CPU-context regression checks retained instruction/fault copies after live register mutation. Board debug.c copies first fault and first/last delivered exceptions; no borrowed pointer escapes. |
| Debug operations | machine_debug_smoke covers stopped access, run/step, bounded instruction observation, register patches including invalid-mask atomicity, ports and watchpoints. Public board lifecycle checks remain before CPU operations. |
| Prepared entry/reset | CPU candidate owns its copied preparation and finish/rollback. machine_entry_plan_smoke retains all mapping, overlap, invalid preload and state checks through public operations, with its unnecessary private include removed. |
| Retirement and observer-free execution | Existing retirement test covers entry/current, context/formulas, invalid running mutation and snapshot requests, then unregisters observer and executes again without another callback. Physical eligibility remains a distinct required consumer. |
| Board memory/scheduler/trace | Successful board memory writes invalidate prefetch through the CPU operation; scheduler advances prefetch reservation through its operation; trace captures PC by value. Neither rewrites private state. |
| FPU and fixtures | Public FPU borrowing and copied diagnostic delivery retain their existing owner; private instruction tests remain S29. Board/executor fixtures use machine/provider operations. Original embedded lifetime remains S30 only. |
| Prevention of regression | Product-wide scan rejects private CPU field access and includes, except CPU-owned files and the exact machine.h embedded-layout receiver. The positive tree and all 77 chip/board negative controls pass in x64 unit. |

Actual diff review finds no handler/table, timing or production API change.
No original test case is deleted. The four test/gate paths add 57/remove 2 lines
(net +55); the growth is five negative controls, an ownership check and a
snapshot-lifetime assertion, not a runtime layer. NXVM production, Shared and
MyNES link inputs are unchanged, so no executable replacement or external
integration rerun is needed for this package.

All 66 specialized steps, six source/test manifests and documentation governance
pass. x64 full unit passes 371/371 in 222.85 seconds; the same live command has
advanced to the x86 build and full suite. No S acceptance is claimed before its
terminal result and actual-commit review.

The sequential command has now terminated successfully (exit 0): x86 full unit
passes 371/371 in 39.17 seconds. Diff whitespace and final documentation checks
also pass. All S20 executor requirements are satisfied; delivery is ready for
P1 and coordinator review, without claiming S21-S32 work complete.
