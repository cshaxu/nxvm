# M5 T539 S54 Outer Return Receiver Map

## Intake classification

S54 consumes the direct includer pair `core_machine_iret_outer_s52_smoke.c`
and `core_machine_protected_return_atomicity_smoke.c` (1,003 source lines at
work-plan intake).  The includer is part of the same batch: it reaches the
same private `atomic_machine` setup and cannot remain as a second route.

| Case family | Receiver boundary | Required proof |
| --- | --- | --- |
| Outer 286 `RETF`/`IRET`: privilege return, code/stack selector cache, frame and GPR restoration | CPU-local | Instruction fixture owns memory, CPU state and exception delivery; proves returned state without Core construction. |
| 386 operand/address-size forms, frame widths, IF restoration and non-present/limit descriptor failures | CPU-local | The same CPU fixture proves exact frame consumption and all-or-nothing state/memory failure behavior. |
| Legacy `IRET` prefix rejection and combined outer-return forms | CPU-local | CPU fixture proves #UD/#DF terminal state and no partial publication. |
| Outer return followed by hardware IRQ, including PIC IRR→ISR acknowledgement and interrupt frame | Public PIC board | A real PIC and explicit CPU-bus acknowledgement own the external source; this is the sole receiver for PIC delivery. |

The CPU receiver is `cpu_outer_return_smoke`; the board receiver is
`machine_outer_iret_pic_board_smoke`.  The former has no Core-machine
construction or PIC object; the latter has the sole real PIC assertion.  The
former retired both intake sources, including the former direct `.c` include.

## Non-S54 transfer

Task-switch, VM86 and generic interrupt/timing rows are not outer-return
receivers and remain with their named later work packages.  No runtime or
public interface needs to change: this batch replaces only private test setup.
