# M5 T321 S3: Hardware Delivery And Gate Composition

## Scope And Route Sweep

S3 closes only the shared 80386 external-event composition boundary.  The
source sweep covered `ExecInt`, `_e_intr_n`, `_ser_int_real`, the protected
interrupt serializers, `flagNMI`, `flagMaskNMI`, `flagMaskInt`, `flagHalt`,
and the PIC scan, peek, and acknowledge operations.  `ExecInt` orders an
unmasked pending NMI before an IF-qualified PIC interrupt and handles TF only
after those external sources.  `_e_intr_n` uses the real serializer in real
mode and the external-origin protected serializer in protected and VM86 mode;
it does not apply software-gate DPL checks.

No production route changed.  The audit found no hardware-delivery defect and
no requirement for a PIC, RTC, PIT, NMI-device, provider, or public-API
change.

## Evidence Matrix

| Cell | Evidence and result |
| --- | --- |
| Real 80386 NMI versus IRQ0 versus TF | `core-machine-hardware-delivery-s3-smoke` asserts NMI, IRQ0, and TF before a NOP. NMI vector 2 wins, saves post-NOP IP and the pre-entry CF/IF/TF image in the no-error-code real frame, clears live IF/TF, clears NMI pending, and leaves IRQ0 in IRR rather than ISR. |
| Protected 80386 same-CPL NMI versus IRQ0 versus TF | The same owner smoke uses a 32-bit interrupt gate. NMI wins, publishes the three-dword no-error-code frame with post-NOP IP and original FLAGS, clears live IF/TF, clears NMI pending, and retains IRQ0 pending without acknowledging it. |
| VM86 NMI versus IRQ0 versus TF | The owner smoke uses the retained T320 GDT/IDT/TSS setup. An unmasked NMI enters the CPL0 32-bit interrupt gate first, stores the nine-dword VM86 external frame with no error code, clears live VM/IF/TF, clears NMI pending, and leaves IRQ0 in IRR. |
| VM86 NMI mask boundary | With `flagMaskNMI` set, pending NMI remains pending while IF-qualified IRQ0 enters its vector, moves from IRR to ISR, and clears live VM/IF/TF. This distinguishes masking from acknowledgement and proves no NMI-side PIC change. |
| Gate type, external DPL bypass, rejected-entry retention | Retained `core-machine-interrupt-entry-smoke` remains the detailed real/protected 16/32-bit interrupt/trap-gate matrix: interrupt gates clear IF and TF, trap gates retain IF while clearing TF, hardware origin bypasses software DPL, and rejected PIC/NMI entry retains its pending source. |
| VM86 IRQ frame and return boundary | Retained `core-machine-vm86-delivery-smoke` proves IRQ0 frame order, IRR-to-ISR publication, CPL0 delivery, and the bounded IRET round trip. It remains the owner of VM86 stack/TSS preflight failures. |
| TF post-instruction trap boundary | Retained `core-machine-tf-db-s60-smoke` proves real and protected post-instruction `#DB` saved-IP/FLAGS behavior and the absence of a spurious trap after prefix/LOCK rejection. |

The S3 owner smoke reads only architectural CPU, PIC, memory-frame, and
diagnostic state.  It does not inspect decoder-private state.

## Boundaries Transferred

S3 does not close software interrupt, INTO, INT3, or IRET composition; those
remain T321 S4.  VM86 LGDT/LIDT remains S5.  Processor-control composition,
task/descriptor/debug breadth, VME/PVI, paging policy, x87 execution, and
reset/shutdown/triple-fault policy remain assigned to later T321 or Queue
packages.  NMI-device behavior remains the existing machine-owned boundary;
S3 proves only the executor's already exposed pending/mask contract.
