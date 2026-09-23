# T323 S4: Protected Same-CPL3 16-Bit External Gate Entry

## Scope And Result

This record covers the bounded Intel 80286/80386 protected, non-VM86,
same-CPL3 external-event path through 16-bit IDT interrupt and trap gates.
The owner smoke reuses S3's owner-local GDT/IDT/machine setup rather than
introducing a public test-support API. It adds a valid DPL3 data descriptor
and matching cached SS selector for the source stack; the gate target is the
existing DPL3 code descriptor. The handler is a two-byte self loop, not HLT:
a CPL3 HLT would itself raise `#GP` and cannot prove gate delivery.

No production code changed. The initial S4 diagnosis found two fixture issues:
the NMI path needs a gate at vector 2 rather than the PIC vector, and a
rejected external event follows a successfully executed source NOP. Therefore
the rejection observable preserves all GPRs, FLAGS, segment caches, stack
sentinel, target publication, and source ownership while correctly requiring
only EIP `0 -> 1` from that source instruction.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286/80386 CPL3 IRQ0 through DPL0 interrupt gate | A CPL3 NOP is followed by pending PIC IRQ0. External origin bypasses the gate DPL; target EIP is `0x100`, the frame is `{1, 0x001b, CF|IF|TF}`, SP falls by six, TF clears, IF clears, and PIC transfers IRR to ISR. | Complete. |
| 80286/80386 CPL3 NMI through DPL0 trap gate | NMI vector 2 uses the explicitly installed DPL0 trap gate. The same exact word frame and target progress occur; TF clears and IF remains set. NMI pending state is acknowledged. | Complete. |
| Gate kind and source cross-check | The smoke runs both gate kinds and both external sources across the two profiles: 80286 interrupt/IRQ and trap/NMI; 80386 interrupt/NMI and trap/IRQ. | Complete for the declared grid. |
| Invalid external gate | An IRQ0 to an invalid vector gate reaches the established no-IDT terminal boundary after the source NOP, preserves the stack sentinel and CPU publication state other than EIP=1, and leaves PIC IRR set/ISR clear. | Complete. |
| Non-present external gate | An NMI to a non-present vector-2 gate reaches the corresponding terminal boundary after the source NOP, preserves the same stack and CPU boundary, and leaves NMI pending. | Complete. |

## Verification And Transfer

`core-machine-protected-16-external-s4-smoke` is registered exactly once as
`current.core-machine-protected-16-external-s4-smoke` and uses target-local
GCC/Clang `-Wall -Wextra -Wpedantic -Werror`. Its focused marker is
`M5:T323:S4:PROTECTED-16-EXTERNAL:OK`.

The declared transfer remains CPL transitions/TSS stack switching, 32-bit and
VM86 gate/return composition, software INT breadth, error-code fault entry,
task/call gates, paging, and generic exception or PIC redesign. Those are not
claimed by this same-CPL external-event slice.
