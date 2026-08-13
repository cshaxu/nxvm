# T323 S7: Protected 16-Bit Call-Gate Entry

## Scope And Result

This record closes the bounded Intel 80286/80386 protected, non-VM86,
16-bit call-gate entry slice. The local `_ser_call_far_call_gate` previously
rejected every nonzero call-gate count with an internal consistency exception.
It now mirrors the already-established 32-bit call-gate discipline: it reads
each declared word from the old stack before changing privilege state,
preflights all `4 + count` target-stack words, then copies the words in
architectural order between the return pair and the old-stack pair.

No descriptor, TSS, stack, PIC, or generic far-transfer helper changed. The
caller sweep is limited to far-call descriptor dispatch and both call-gate
serializers; the 32-bit serializer remains unchanged.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286 TSS16 outer call | A DPL3 `9A` call through a present DPL3 16-bit gate enters DPL0 CS `0008h` / SS `0010h` and uses TSS SP0 `7000h`. | Complete. |
| 80386 TSS16 and TSS32 outer call | Both TSS layouts select the same 16-bit target stack and publish the same CPL0 target state. | Complete. |
| Parameter words and frame | A two-word old stack `{1234h, abcdh}` becomes the target image `{return IP 0005h, return CS 001bh, 1234h, abcdh, old SP 8000h, old SS 0023h}` at `6ff4h`. | Complete. |
| Same-CPL gate | A DPL0, RPL0 call enters the same CPL0 target and pushes only `{0005h, 0008h}`; the declared outer-only parameter count has no effect. | Complete. |
| Gate/target/TSS/stack prepublication | Gate DPL denial, invalid TR, null TSS SS0, invalid target code, a short parameter source, and a short target stack reach their synchronous fault boundary before target cache/CPL/stack publication. | Complete. |
| Pending IRQ ordering | CPL3 `STI; CALL gate` executes the call during STI's one-instruction inhibition. IRQ0 then enters at the call target, with the call frame retained and an IRQ frame `{0100h, 0008h, CF|IF|IOPL}` above it; PIC IRR transfers to ISR. | Complete. |

## Verification And Transfer

The owner target is registered once as
`current.core-machine-protected-16-call-gate-s7-smoke`, uses target-local
`-Wall -Wextra -Wpedantic -Werror`, and emits
`M5:T323:S7:PROTECTED-16-CALL-GATE:OK`.

The refreshed `vm-0-5-0323` developer artifact SHA-256 is
`0C901AC5E883FDB26D70B317814E580BB9000CE50899BBCA2AF273912DF1F785`.

32-bit call gates, call/far returns, direct/indirect far calls, task gates and
switches, VM86, paging, and generic exception/PIC policy remain transferred.
