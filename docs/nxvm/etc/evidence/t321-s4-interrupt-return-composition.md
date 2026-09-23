# M5 T321 S4: Software Interrupt, Trap, And Return Composition

S4 audits `_e_int3`, `_e_int_n`, `_e_into`, `_e_iret`, `_e_intr_n`, and their
real/protected serializers.  No production route changed: the existing entry
and return planners meet the admitted composition contract.

| Boundary | Evidence |
| --- | --- |
| INT3, `INT ib`, and INTO forms | Retained `core-machine-software-int-s50-smoke` owns their real/protected/VM86, prefix, DPL, fault, and individual pending-PIC matrices. It distinguishes interrupt-gate IF/TF clearing from trap-gate IF retention. |
| Same-CPL and outer IRET | Retained `core-machine-iret-s51-smoke` and `core-machine-iret-outer-s52-smoke` own frame-width, selector, privilege, fault, and restored-IF pending-IRQ matrices. T320 remains the owner of the bounded VM86 IRET return. |
| Direct composition | `core-machine-interrupt-return-composition-s4-smoke` executes real-mode `INT 31h`, whose handler is `IRET`, while IRQ0 is pending. The software interrupt consumes its own frame, IRET restores IF, then IRQ0 is acknowledged and delivered before the resumed NOP. The IRQ frame saves IP 2, CS 0, and the restored CF/IF image; the handler HLT reaches 0121h with IRQ0 in ISR and IRR clear. Thus software and external frames are not confused, and external acknowledgement occurs only after the return enables it. |
| S2/S3 interaction | S2 remains the owner of synchronous exception vectors and error-code frames. S3 remains the owner of IRQ/NMI priority and gate acknowledgement. The S4 sequence is intentionally no-error-code and does not alter NMI, TF, or exception-recursion policy. |

Transferred boundaries remain task gates/switches, VME/PVI, VM86 IRET breadth,
generic PIC/NMI design, debug/breakpoint policy, paging, reset/shutdown, and
x87 execution.
