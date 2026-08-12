# T323 S2: Protected Data-Access and 16-Bit IRQ Frame Boundaries

## Scope And Result

This evidence closes T323 S2's bounded Intel 80286/80386 protected,
non-VM86 data-segment access matrix for already-loaded DS, ES, and SS caches.
The owner-local `core-machine-protected-data-access-s2-smoke` performs the
actual memory accesses; it does not claim selector-load, paging, call/task
gate, outer-return, or VM86 breadth.

The focused IRQ vector reproduced one shared protected interrupt-gate defect:
the 16-bit gate route passed `!software_origin` as its error-frame condition.
That made an external IRQ0 push a spurious error-code word.  The narrow
`_ser_int_protected` call-site repair instead passes the existing `flagext`
classification.  Thus hardware IRQ and software INT have the ordinary
FLAGS/CS/IP frame, while CPU exceptions remain the only error-frame users.
The similar-issue sweep found one 16-bit gate call site and retained the
separate 32-bit same-CPL/outer serializers, which already receive `flagext`.

## Form-To-Proof Matrix

| Condition | Focused proof | Result |
| --- | --- | --- |
| 80286 and 80386 loaded DS/SS/ES caches | DS byte read/write, BP-selected SS read, and fixed-ES `STOSB` each use distinct physical bases.  The expected byte, EIP/index effect, and FLAGS publication are asserted. | Complete. |
| 80386 attributes | `66 MOV r32,[disp16]` proves operand-width publication; `67 MOV AL,[EBP]` proves 32-bit SS-default effective-address selection.  80286 `66` and 80386 `LOCK MOV` are `#UD` with complete CPU/cache nonpublication. | Complete for declared routes. |
| Null and access-right cache boundaries | A null/invalid DS read and read-only DS write both terminate through the established no-IDT protected `#DF` observable, preserving complete CPU/cache state; the write leaves its physical sentinel unchanged. | Complete. |
| Ordinary and expand-down limits | DS ordinary-limit, DS expand-down lower-bound, SS limit, and fixed-ES string-destination limit failures have the same no-publication boundary.  A 16-bit expand-down DS access immediately above its limit succeeds. | Complete. |
| Protected external IRQ0 frame | IRQ0 pending before a successful DS read reaches the 16-bit protected interrupt gate after the read: AL is published, saved IP is `4`, ESP is `0x7ffa`, the handler halts at `08:0101`, and PIC ISR/IRR ownership is exact.  The three-word stack result proves no spurious error-code word and no additional interrupt shadow. | Complete. |

## Verification And Transfers

The target has target-local GCC/Clang `-Wall -Wextra -Wpedantic -Werror` and
is registered exactly once in the current gate.  Its marker is
`M5:T323:S2:PROTECTED-DATA-ACCESS:OK`.

The required T323 developer artifact is `build/output/nxvm_0_5_0323.exe`.
Its focused-build SHA-256 is
`1B8DC97766E6A0C51CA6C14D2BEA295B2E7DFEE1D209F5AB541C020F4BA7F375`.

The narrow correction does not generalize descriptor helpers, selector loading,
32-bit or outer interrupt frames, TSS stack switching, paging, or VM86.  Those
remain the named protection, paging, and task/VM86 Queue packages.
