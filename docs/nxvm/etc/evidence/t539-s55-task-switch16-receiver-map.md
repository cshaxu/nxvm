# M5 T539 S55 16-bit Task-Switch Receiver Map

## Frozen intake batch

S55 consumes only the 16-bit and task-gate contexts in
`core_machine_task_switch_smoke.c`. The source remains the S56 input for its
32-bit and cross-width contexts; it is not a retained S55 receiver.

| Context | Profiles | Receiver boundary | Disposition |
| --- | --- | --- | --- |
| Direct 16-bit task JMP, descriptor/cache save-load and busy-bit publication | 80286, 80386 | CPU-local | S55 receiver |
| 16-bit LDT load and indirect task JMP | 80286, 80386 | CPU-local | S55 receiver |
| Task CALL and GDT task-gate entry, backlink and NT | 80286, 80386 | CPU-local | S55 receiver |
| Invalid selector, not-present, busy, short-TSS, LDT-not-present, stack-limit and LOCK failure atomicity | named 80286/80386 forms | CPU-local | S55 receiver |
| Nested task return, IDT task-gate and double-fault task-gate delivery | named 80286/80386 forms | CPU-local | S55 receiver |
| Pending hardware IRQ after a task switch, including PIC IRR-to-ISR acknowledgement | 80286, 80386 | Public PIC board | S55 receiver |
| Operand-size/address-size 32-bit task JMP forms | 80386 | CPU-local | S56 |
| 16-to-32, 32-to-16 and direct 32-bit TSS transitions, paging/debug-TSS rows | 80386 | CPU-local/public board as classified at S56 | S56 |
| CPL/I/O-map authorization | 80286, 80386 | public Core board | S57 |

The CPU receiver must construct only CPU memory/state/bus fixtures. The sole
PIC receiver must connect a real `core_machine_pic_bus` through the public CPU
bus acknowledgement contract. No receiver may retain an `atomic_machine`, an
embedded executor CPU, or direct shared-PIC field access.

## Transfer boundary

The 32-bit cases begin with `TASK_SWITCH_CASE_OPERAND32_SUCCESS` and the
`task_switch_smoke_tss32_state` family. They remain intact in the residual
source for S56. S55 may retain common source helpers temporarily only when a
remaining S56 caller uses them; it must delete every S55-only caller and test
path rather than leave an alternate execution route.

## Acceptance verification

- Both S55 receivers pass on x64 and x86:
  `M5:T539:S55:TASK16:OK` and `M5:T539:S55:TASK16-PIC:OK`.
- Complete repository-only units pass 423/423 on each width.
- The 66 specialized gates pass, including T317's 42 strict CPU targets,
  T332's 42 lifecycle owners, the CPU/PIC authority sweep and the repaired
  CMake negative mutation for direct CPU-field access.
- `git diff --check`, the Lib manifest verifier and NXVM documentation
  governance pass. No S55-only private helper or conditional dead-code block
  remains in the residual source.
