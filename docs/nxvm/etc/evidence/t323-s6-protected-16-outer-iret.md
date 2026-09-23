# T323 S6: Protected CPL0-To-CPL3 16-Bit Outer IRET

## Scope And Result

This record covers the bounded Intel 80286/80386 protected, non-VM86,
16-bit outer-privilege `IRET` path. The owner-local smoke starts in CPL0 with
a five-word frame `{IP, CS, FLAGS, SP, SS}` on the current kernel stack and
returns to the prepared DPL3 code/data descriptors. No production correction
was required: `_ser_iret_protected_outer` already preflights all frame reads,
target code and stack candidates, then publishes both caches, CPL, SP, EIP,
and permitted FLAGS together.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286 default 16-bit outer IRET | Consumes the exact five-word frame, returns to CS `001bh` / SS `0023h` at CPL3 and EIP `0010h`, preserves non-target GPR/cache state, and restores permitted CF/IF/IOPL. | Complete. |
| 80386 default and `67 IRET` | Both retain the same 16-bit operand/frame meaning. The target 16-bit SS changes only ESP low half, proving `12348000h -> 12344000h`. | Complete. |
| Invalid target CS and SS | Null CS and null SS frames reach the established terminal boundary without consuming or changing the five-word source image or publishing target CPU/cache state. | Complete. |
| Source frame limit | A current SS limit shorter than ten bytes fails before any frame consumption or target publication. | Complete. |
| Restored IF and pending IRQ | A 80386 outer IRET restores IF; the pending IRQ0 is delivered immediately at returned EIP `0010h` through the existing TSS32 stack route. The second outer frame is `{0010h,001bh,CF|IF|IOPL,4000h,0023h}` at SS:`6ff6h`; live ESP retains its prior high half (`12346ff6h`) because the kernel stack is 16-bit. PIC IRR transfers to ISR and live IF clears. | Complete. |

## Verification And Transfer

The owner target is registered once as
`current.core-machine-protected-16-outer-iret-s6-smoke`, uses target-local
`-Wall -Wextra -Wpedantic -Werror`, and emits
`M5:T323:S6:PROTECTED-16-OUTER-IRET:OK`.

32-bit and VM86 return, task/NT returns, call/far returns, same-CPL IRET
breadth, VME/PVI, paging, and generic exception/PIC policy remain explicit
transfers.
