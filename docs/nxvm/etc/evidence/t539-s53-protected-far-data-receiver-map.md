# M5 T539 S53 Protected Far And Data Receiver Map

## Intake classification

S53 consumes exactly two direct-private sources (934 lines at work-plan
intake).  The old file boundary is not the receiving boundary.

| Old family | Receiver | Required proof |
| --- | --- | --- |
| Protected far `JMP`/`CALL`, direct and indirect forms, selector/cache validation, 286/386 prefix behavior, conforming-code cases and preflight rollback | `cpu_protected_far_smoke.c` | Instruction fixture owns memory and CPU state; proves target/cache state, stack frame, #UD/#DF and no partial state publication. |
| Protected DS/SS/ES reads/writes, operand/address-size variants, expand-down segments and failed access atomicity | `cpu_protected_data_access_smoke.c` | Instruction fixture owns data/segment state; proves result bytes/registers, segment limits/attributes and unchanged state on fault. |
| Far-transfer IRQ no-shadow check | `machine_protected_far_pic_board_smoke.c` | Real `x86_pic`, PIC board adapter and CPU-bus acknowledge prove the far transfer is followed by IRQ delivery, IRR→ISR transition and the interrupt frame. |
| Data-access IRQ no-shadow check | `machine_protected_data_pic_board_smoke.c` | The same board contract proves DS access has no hidden interrupt suppression before PIC delivery. |

## Boundary decision

No production or public interface changes are permitted.  CPU receivers use
the existing instruction fixture rather than a Core reset wrapper, physical
memory borrow, or copied CPU mirror.  Both PIC receivers use one small
test-only PIC board adapter with an explicit CPU bus; they do not access a
`core_machine` field.
The CPU-local and board cases remain separate even when they share a protected
mode descriptor layout, because the latter additionally owns a real external
interrupt source and acknowledgement path.

## Verification

- The four affected x64 receiver targets pass.
- The complete repository-only x64 unit suite passes with parallel execution.
- The complete repository-only x86 unit suite passes with serial execution
  (879.00 seconds aggregate unit time).
- T317 confirms 41 target-local strict CPU compile commands; T332 confirms
  all 41 CPU fixture owners; T344 confirms the 101-row direct-constructor
  inventory.
- A direct source sweep confirms the two CPU receivers and their protected
  fixture do not retain a Core-private machine access path.

## Similar-issue scope

Outer-return, task-switch, protected I/O, interrupt/VM86 and timing ledger
families are not S53 cases.  They retain their named later work-package
receivers and must not be copied into an S53 test merely because they also use
protected-mode descriptors.
