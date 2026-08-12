# T323 S5: Protected CPL3-To-CPL0 16-Bit External Gate Entry

## Scope And Result

This record closes T323 S5's bounded Intel 80286/80386 protected, non-VM86,
outer-CPL entry through 16-bit interrupt and trap gates. The owner-local smoke
reuses the committed S3 protected-gate fixture without exposing a test-support
API. It creates a DPL3 source code/stack state, directs the gate to DPL0 code,
and supplies a cached busy TSS16 or TSS32 with a selected CPL0 stack.

No production correction was required. The existing `_ser_int_protected_16`
outer branch already reads TSS16 `SP0:SS0` and TSS32 `ESP0:SS0`, validates and
preflights the target stack, writes descriptor accessed state, then publishes
the two old-stack words and the three-word saved execution image. The retained
T307 32-bit outer path and T320 VM86 route remain unmodified.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286 TSS16 outer entry | Both interrupt/trap gates run both IRQ0 and NMI from CPL3 to CPL0. The target is selector `0008h`, SS is `0010h`, target EIP is `0100h`, and the selected stack holds five words `{IP=1, CS=001bh, FLAGS=CF|IF|TF, SP=8000h, SS=0023h}`. | Complete. |
| 80386 TSS16 outer entry | The same four gate/source combinations execute through the TSS16 route with the same 16-bit stack/frame contract. | Complete. |
| 80386 TSS32 outer entry | The same four combinations execute through the TSS32 `ESP0:SS0` route. The admitted 16-bit gate contract keeps the target stack pointer and five-word frame 16-bit. | Complete. |
| Gate effects and source ownership | Interrupt gates clear live IF and TF; trap gates retain IF and clear TF. IRQ0 transitions PIC IRR to ISR; NMI clears its pending flag. Non-stack GPRs and ES/DS/FS/GS caches are preserved. | Complete. |
| Invalid TSS prepublication boundary | A pending IRQ0 with an invalid task register reaches the established terminal boundary after its source NOP. User CS/SS/cache state, GPRs, FLAGS, and the seeded target-stack five-word window remain unchanged; PIC IRR remains set and ISR clear. | Complete. |
| Invalid and non-present SS0 boundaries | A null TSS-selected SS0 and a non-present target SS descriptor each retain the same source/target-stack/PIC boundary. The smoke records the terminal diagnostic rather than treating exception-delivery state as identical. | Complete. |

## Verification And Transfer

`core-machine-protected-16-outer-s5-smoke` is registered exactly once as
`current.core-machine-protected-16-outer-s5-smoke` and has target-local
GCC/Clang `-Wall -Wextra -Wpedantic -Werror`. Its marker is
`M5:T323:S5:PROTECTED-16-OUTER:OK`.

Outer protected `IRET`, 32-bit and VM86 gate/frame breadth, task/call gates,
software interrupt breadth, error-code producer breadth, paging, and generic
exception/PIC policy remain explicit transfers. The current slice consumes the
existing TSS delivery mechanism but does not claim task switching or a general
TSS subsystem.
