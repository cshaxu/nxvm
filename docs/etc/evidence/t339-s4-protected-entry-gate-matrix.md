# M5 T339 S4: 80286 Protected 16-Bit Entry And Gate Matrix

## Mechanism Inventory

S4 closes the 80286 entry-side mechanism as one validation-to-publication
path.  It does not treat direct transfer, software interrupt, external
interrupt, and call-gate spelling as interchangeable evidence.

| Entry form | Decode/origin owner | Validation and publication owner | Accepted 80286 owner |
| --- | --- | --- | --- |
| Direct far `CALL` / `JMP` | `_e_call_far`, `_e_jmp_far` | `_ser_call_far_*`, `_ser_jmp_far_*`, `_kec_{call,jmp}_far` | `current.core-machine-protected-far-s1-smoke` |
| Software `INT` and synchronous error entry | `_e_int_n`, `_e_except_n` | `_ser_int_protected_16` | `current.core-machine-protected-16-gate-s3-smoke`, `current.core-machine-protected-16-external-s4-smoke` |
| PIC IRQ and NMI | `ExecInt`, `_e_intr_n` | `_ser_int_protected_16` | `current.core-machine-protected-16-external-s4-smoke` |
| 16-bit call gate | `_e_call_far` descriptor dispatch | `_ser_call_far_call_gate` | `current.core-machine-protected-16-call-gate-s7-smoke` |

The caller/write sweep found a deliberate split only at architectural layout
boundaries: `_ser_int_protected_16` and `_ser_call_far_call_gate` read
TSS16 `SP0:SS0`, preflight the selected 16-bit stack, then publish a distinct
five- or six-word frame.  `_ser_int_protected_32_*` and the 32-bit call-gate
serializer retain their separate 80386 layouts.  No duplicate 16-bit entry
publisher or parallel TSS16-stack reader exists in S4 scope.

## 80286 Matrix

| Boundary | Actual proof | Disposition |
| --- | --- | --- |
| Direct far `CALL` / `JMP` | Immediate and memory-indirect 16-bit far forms execute under the 80286 profile.  Code selector/type/DPL/present/limit and call-stack preflight failures retain source state. | Complete. |
| Same-CPL software gate | `INT` through 16-bit interrupt and trap gates records the three-word `{IP, CS, FLAGS}` image, selector-cache publication, gate accessed state, IF/TF behavior, error-code frame, and DPL/present/type rejection. | Complete. |
| Same-CPL external gate | 80286 PIC IRQ and NMI run through both 16-bit gate kinds.  IRQ is acknowledged only after valid entry; NMI pending state clears only after valid entry. | Complete. |
| Outer software entry | New S4 owner vector runs 80286 CPL3 `INT` through a DPL3 16-bit interrupt gate to CPL0 using a busy TSS16.  It proves `{IP=2, CS=001b, FLAGS, SP=8000, SS=0023}` on selected `SS0:SP0`, and live IF/TF effects. | Complete. |
| Outer external/NMI entry | New S4 vectors run 80286 CPL3 PIC IRQ, then NMI, through 16-bit gates to CPL0.  They prove TSS16 `SP0:SS0` selection, the five-word frame, interrupt/trap IF distinction, TF clearing, IRQ IRR-to-ISR consumption, and NMI acknowledgement. | Complete. |
| Error entry and ordering | A CPL3 `INT` denied by gate DPL produces `#GP` through an installed 16-bit gate.  The owner proves the six-word outer stack image `{error, restart IP, CS, FLAGS, old SP, old SS}` and no original INT publication. | Complete. |
| Call gate | CPL3 to CPL0 through a 16-bit call gate proves busy TSS16 selection, two parameter-word copy, target frame, source/target preflight, same-CPL behavior, and IRQ order. | Complete. |
| Outer-stack failure boundary | `_ser_int_protected_16` produces the 80286 `#TS/#SS` prepublication failure before target-stack/cache publication.  The retained owner has the equivalent invalid-TR/null-or-non-present-SS0 sentinel proof under 80386; it is not evidence of 80286 final delivery.  The 80286 producer boundary is audited, but its generic `#TS/#SS` terminal-versus-delivered contract needs the shared finalizer owner. | Transferred to T342; no 80386 contributory-fault behavior is inferred for 80286. |

## Exact Transfers

- `LOCK` legality for the memory-capable far forms is the accepted T328
  pre-386 matrix, not an inference from the 80386 prefix policy.
- `66/67`, 32-bit gate/frame/call-gate layouts, FS/GS, and 80386DX form
  composition transfer to T341.
- VM86 gate entry, paging fault composition, 80386 contributory/double-fault
  treatment, and any general 80286 `#TS/#SS` delivery-policy expansion
  transfer to T342's system-state/delivery owner.  S4 does not introduce a
  generic exception-finalizer rewrite merely to turn the documented terminal
  80286 outer-stack failure proof into a different delivery contract.
- IRET/RETF consumption remains T339 S5; direct/task-gate task transition,
  busy/backlink, and TSS image commit remain S6.

## Verification

S4 runs the four exact owners above, fresh CMake configuration, exact CTest
registration discovery, the artifact verifier, documentation governance,
`git diff --check`, and the complete current gate.  The extended external
owner retains its existing target-local strict GCC/Clang options and marker
`M5:T323:S4:PROTECTED-16-EXTERNAL:OK`.

This evidence retires into the T339 history record at task closure.
