# M5 T539 S57 — TSS32 state receiver map

| Context group | Receiver | Owner |
| --- | --- | --- |
| Direct, operand-size and indirect/address-size TSS32 task jumps | `cpu_task_switch32_state_smoke` | CPU |
| Invalid code, busy target, short old/target TSS and stack-limit failures | `cpu_task_switch32_state_smoke` | CPU |
| LDT load plus bad descriptor, non-present, short, bad-code and bad-data rows | `cpu_task_switch32_state_smoke` | CPU |

The receiver owns a copied memory image, GDT, LDT, IDT and TSS32 states. It
executes the CPU's real `LGDT`/`LTR` bootstrap before each row, then observes
the target state or delivered CPU exception. It links only `x86-cpu`.

Paging, debug-trap, LOCK, nested/call/task-gate, pending-PIC IRQ and 16↔32
state-image rows remained in `core_machine_task_switch_smoke.c` for S58-S62.
No board/PIC/VM state is present in this receiver.
