# M5 T339 S5: 80286 Protected 16-Bit Return Matrix

## Mechanism Inventory

S5 closes the ordinary protected-return consumer as one source-frame
preflight-to-publication mechanism.  It distinguishes true frame-layout
variants from unrelated 80386 return modes.

| Return form | Decode owner | Validation and publication owner | 80286 proof owner |
| --- | --- | --- | --- |
| Same-CPL `IRET` | `_e_iret` | `_ser_iret_protected_same` | `current.core-machine-protected-16-outer-iret-s6-smoke` |
| Outer-CPL `IRET` | `_e_iret` | `_ser_iret_protected_outer` | `current.core-machine-protected-16-outer-iret-s6-smoke` |
| Same-CPL `RETF` | `_e_ret_far` | `_ser_ret_far_same` | `current.core-machine-protected-16-outer-iret-s6-smoke` |
| Outer-CPL `RETF` / `RETF imm16` | `_e_ret_far` | `_ser_ret_far_outer` | `current.core-machine-protected-16-outer-iret-s6-smoke`, `current.core-machine-protected-return-atomicity-smoke` |

The caller/write/fault sweep found the deliberate architectural split only at
the 16- versus 32-bit frame layout.  For the 16-bit serializers every stack
word is peeked and every target code/stack cache is prepared before a cache,
EIP, SP, FLAGS, or descriptor-accessed publication.  `_ser_iret_protected_to_vm86`
and task-return `IRET` retain their distinct 80386 and task-state layouts;
they are not alternate implementations of this mechanism.

## 80286 Matrix

| Boundary | Actual proof | Disposition |
| --- | --- | --- |
| Same-CPL `IRET` | New 80286 owner vector consumes `{IP, CS, FLAGS}`, preserves the 16-bit stack-address high half, loads the kernel code cache, retains the stack and non-target caches/GPRs, restores permitted CF/IF/IOPL, and leaves its source frame unchanged. | Complete. |
| Outer-CPL `IRET` | The 80286 vector consumes `{IP, CS, FLAGS, SP, SS}`, materializes both DPL3 caches, keeps the target 16-bit SS high half, restores permitted FLAGS, and leaves the source frame/non-target state unchanged. | Complete. |
| Same-CPL `RETF` | New 80286 `CB` vector consumes `{IP, CS}`, publishes only code/EIP/current-SP effects, retains FLAGS and non-target state, and proves the source frame is read-only. | Complete. |
| Outer-CPL `RETF` | New 80286 `CB` vector consumes `{IP, CS, SP, SS}`, materializes DPL3 code/stack caches, preserves FLAGS, and proves frame immutability. | Complete. |
| Outer-CPL `RETF imm16` | New 80286 `CA 04 00` vector proves the old-frame parameter skip precedes the selected target `SP + 4` publication; the distinguishable parameter words and full source frame remain unchanged. | Complete. |
| Return-frame and target rejection | The extended 80286 owner proves null CS, null SS, and short-source-frame IRET failure keeps CPU and source frame unchanged.  The retained atomic owner runs both IRET and RETF under the 80286 profile for non-present code/stack, invalid code type, and invalid privilege boundaries, with delivered exception frame or terminal state distinguished from pre-handler publication. | Complete. |
| Restored IF and external ordering | New 80286 outer IRET vector restores IF, then consumes pending PIC IRQ0 through the selected busy TSS16 `SP0:SS0`; it proves the new five-word entry frame, IRR-to-ISR transition, and live IF clearing on interrupt delivery. | Complete. |
| Real-mode disposition | The real IRET/RETF instruction-form matrix is retained as T338's 8086/80186 profile owner; S5 neither changes nor uses it to prove protected returns. | Retained earlier owner. |

## Exact Transfers

- 80286 `LOCK` legality for memory-capable return forms is T328's accepted
  matrix; it is not inferred from the 80386 lock whitelist.
- `66/67`, 32-bit frames, FS/GS and 80386DX form composition transfer to T341.
- Return-to-VM86, VME/PVI, paging-associated fault composition and the shared
  80286 `#TS/#SS` final-delivery boundary transfer to T342.
- `IRET` with NT and all backlink/busy/task-return state remain T339 S6.

## Verification

S5 runs both exact current-gate owners above, fresh CMake configuration,
exact registration discovery, the artifact verifier, documentation governance,
`git diff --check`, and the complete current gate.  The extended 16-bit owner
retains target-local strict GCC/Clang options and its historical marker
`M5:T323:S6:PROTECTED-16-OUTER-IRET:OK`.

This evidence retires into the T339 history record at task closure.
