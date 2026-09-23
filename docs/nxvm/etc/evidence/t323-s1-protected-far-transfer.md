# T323 S1: Protected Direct Far Control Transfer

## Scope And Result

This record closes only the direct protected-mode code-descriptor transfer
slice of the Queue's 80386DX protection and privilege-transfer candidate.  The
owner-local `core-machine-protected-far-s1-smoke` covers `CALL ptr16:16/32`
(`9A`), `JMP ptr16:16/32` (`EA`), and their memory-only `FF /3` and `FF /5`
counterparts.  The Intel 80386 PRM direct far-transfer rules are the semantic
authority.  The implementation audit found no production defect: the existing
`_ser_call_far_cs_conf`, `_ser_call_far_cs_nonc`, `_ser_jmp_far_cs_conf`,
`_ser_jmp_far_cs_nonc`, `_e_call_far`, `_e_jmp_far`, and `FF` decoder routes
already apply the required direct-code-descriptor classification.

No call gate, task gate/TSS transfer, outer-privilege return, VM86, paging, or
shared stack/descriptor/delivery helper was changed.  Those routes remain
separate consumers in T307, T320, T321, or later Queue work.

## Form-To-Proof Matrix

| Form or condition | Focused proof | Result |
| --- | --- | --- |
| 80286 and 80386 default `9A`, `EA`, `FF /3`, `FF /5` to same-CPL nonconforming code | Separate prepared fixtures run each form to a target `HLT`; direct CALL checks the two-word return image and both CALL forms publish only the expected frame/CS:EIP state. | Complete. |
| 80386 operand/address attributes | `66 9A` and `66 EA` select ptr16:32 and the four-byte frame; `67 FF /5` selects a 32-bit EA while retaining ptr16:16; `66 67 FF /3,/5` select both 32-bit EA and ptr16:32.  Target offsets, CS, EIP, and ESP are asserted. | Complete. |
| Direct code descriptor classification | Nonconforming DPL mismatch and RPL mismatch, non-present code, and data-selector targets have terminal protected fault boundaries with entry CS/cache/EIP/ESP/EFLAGS/GPR preservation.  A conforming DPL0 target succeeds without changing CPL. | Complete. |
| Target and stack preflight | A target-code limit failure and a too-small SS limit for CALL preserve entry CPU/cache state.  The actual future return-frame stack slots retain their sentinel. | Complete. |
| Profile and LOCK boundaries | 80286 `66` CALL/JMP and 80386 `F0` CALL/JMP reject as `#UD` before CPU or cache publication. | Complete for the declared attribute boundary. |
| Event composition | A protected 80386 IRQ0 pending before a successful direct far JMP is delivered at the target boundary: the saved IP is zero in the new code segment, the gate handler halts at `08:0101`, and PIC ISR/IRR ownership is exact. | Complete: direct far transfer has no interrupt shadow. |

## Similar-Issue Sweep

The four direct descriptor serializers were reviewed as one set.  The CALL
paths preflight their complete return frame before their first push; the JMP
paths publish only after candidate CS and logical target validation.  The
dispatcher retains distinct call-gate, task-gate, TSS, and outer-return paths;
they are deliberately not inferred from this evidence.  No equivalent local
defect was found outside the owner smoke's declared matrix.

## Verification

- `core-machine-protected-far-s1-smoke` emits
  `M5:T323:S1:PROTECTED-FAR:OK`.
- Its CMake target has target-local GCC/Clang `-Wall -Wextra -Wpedantic
  -Werror`; this does not claim strict coverage for `core-machine` or its
  inherited sources.
- Configure, exact current-gate discovery, the complete current gate,
  documentation governance, and `git diff --check` are required by the active
  packet before acceptance.
