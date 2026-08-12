# M5 T320 S1: VM86 CPL0 Delivery Matrix

## Scope

This evidence covers only 80386 VM86 entry through a present 32-bit protected
IDT interrupt gate to CPL0. It excludes inverse `IRET`, VME/PVI, task gates,
task switching, paging, NMI, and VM86 instruction-family completion.

## Owner Smoke

`tests/machine/core_machine_vm86_delivery_smoke.c` is registered once as
`current.core-machine-vm86-delivery-smoke` and emits
`M5:T320:S1:VM86-DELIVERY:OK`.

| Origin | Actual producer | Asserted entry boundary |
| --- | --- | --- |
| `#UD` | VM86 `UD2` | Vector 6, no error-code dword, restart EIP, TSS `SS0:ESP0`, nine-dword VM86 frame. |
| `#GP(0)` | VM86 `CLI` with IOPL below 3 | Vector 13, error-code dword, restart EIP, TSS stack and ten-dword frame. |
| `#NM` | VM86 `ESC` with CR0.EM | Vector 7, no error-code dword and restart EIP. |
| IRQ0 | Actual master-PIC IRQ0 after VM86 `NOP` | Vector 32, PIC IRR-to-ISR transition, post-instruction saved EIP, no error-code dword. |

Every success vector verifies a present 32-bit interrupt gate, CPL0 CS/SS and
TSS-selected ESP, cleared VM/IF/TF, and non-retention of the VM86 ES/DS/FS/GS
caches. The frame is stored on the selected CPL0 stack in Intel dword order:
optional error code, EIP, CS, EFLAGS, ESP, SS, ES, DS, FS, GS. Selector slots
are zero-extended dwords.

An independent VM86 `NOP` with TF set reaches the installed vector-1 gate,
proves the post-instruction saved EIP and saved VM86 TF, and proves that the
interrupt-gate handler state has TF clear. It is gate-effect evidence, not a
substitute for the four admitted origins.

## Failure Boundaries

Each failure begins at VM86 `UD2`, records the terminal diagnostic, and checks
the complete captured CPU state before any frame/cache/CPL/ESP/VM publication.
The matrix supplies: absent, non-present, and wrong-type IDT entries; invalid,
non-busy, and too-short 32-bit TR/TSS states; null and RPL-invalid SS0; SS0
descriptors that are non-writable or not-present; and an otherwise-valid SS0
whose limit cannot contain the complete frame. The exact terminal boundaries
are the existing protected-delivery diagnostics (`#GP(32)`, `#NP(32)`,
`#TS(0/#TS(18))`, `#SS(0/#SS(10))`, and `#GP(10)` as applicable). They are
failure boundaries, not successful VM86 entry claims; the smoke never claims
that a second exception was delivered with the source state intact.

For a VM86 source, a non-32-bit gate is classified locally as `#GP` and an
invalid or non-busy 32-bit TR as `#TS`; the existing contributory delivery
route then preserves the established terminal `#DF` boundaries for S50, S55,
S62, and S63 invalid-facility fixtures. A valid 32-bit gate/TSS instead enters
the VM86 outer path. This is a local protected-delivery classification repair;
`ExecFinal` priority is unchanged.

## Direct Caller Sweep

The implementation sweep reviewed `_ser_int_protected`,
`_ser_int_protected_32_same`, `_ser_int_protected_32_outer`, `_e_except_n`,
`_e_intr_n`, `ExecFinal`, and `_e_iret`. The admitted change is confined to
32-bit outer interrupt-gate delivery and protected exception vector selection.
IRET remains unmodified as a T320 S2 transfer. Real mode, same-CPL protected
delivery, 16-bit gates, task gates, paging, VME/PVI, and PIC policy do not
change.

## Directly Affected Historical Assertions

The T316 `CLI`/`STI`, `PUSHF`/`POPF`, and `HLT` VM86 fixtures had deliberately
installed valid CPL0 `#GP` gates but retained the pre-T320 expectation that
the producer ended as an undelivered fault. Their narrow VM86 IOPL-failure
assertions now require a delivered vector-13 CPL0 entry instead. The instruction
success cases and all non-VM86 assertions remain unchanged; the S47/S48 smoke
sources include those owner fixtures and therefore receive the same corrected
assertion without a duplicate implementation.
