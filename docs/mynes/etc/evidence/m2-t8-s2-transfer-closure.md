# M2 T8 S2 Ordered Transfer Closure

S2 retains one production CPU/Bus execution path and its copied transfer
records. It does not add a timing interpreter or infer ordering from aggregate
cycle counters.

The independently expected fixtures cover the admitted transfer classes:

| Class | Direct fixture |
| --- | --- |
| opcode, operand, implied and indexed reads | `addressing_smoke`, `alu_smoke` |
| zero-page, absolute and indirect indexed writes | `addressing_smoke` |
| indexed no-cross/cross dummy reads | `addressing_smoke` |
| read-modify-write reads and old/new writes | `rmw_smoke`, `memory_increment_semantics_smoke` |
| taken and cross-page branches | `branch_smoke` |
| stack transfers, BRK/RTI and JSR/RTS | `stack_control_smoke` |

The recorded sequences include zero-page wrap, uncorrected-page reads, data
latch-bearing discard reads, old-value RMW writes, control-stack accesses and
vector reads. The fixtures execute through `core_machine_run` and the actual
Core bus; no test-only trace producer exists.

On 2026-09-21 the complete executable suites under `build/shared-x64/test-*`
and `build/shared-x86/test-*` passed after rebuilding both trees. This closes
only S2's ordered-transfer outcome. Reset/IRQ/NMI pin sampling, recognition,
priority and entry accounting remain S3 work.
