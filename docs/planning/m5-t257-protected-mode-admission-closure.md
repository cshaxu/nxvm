# M5 T257: Protected-Mode Admission Closure

## Status

**Complete.** S5 defined the closure, S6 implemented the bounded decoder gate
and corpus. M5 Td S28 reissued the current T257 artifact as `0.5.0257` under
the task-identity artifact rule.

## Original Request

Close the T257 audit finding without broadening protected-mode support. The
admitted 80286 path remains GDT-only, CPL0, 16-bit, and diagnostic-stop based.

## S5 Contract

| Concern | Required result | Non-goal |
| --- | --- | --- |
| IDT table forms | Protected-mode `SIDT` and `LIDT` stop as `#UD`; real-mode behavior remains unchanged. | Implementing IDT storage or delivery. |
| Software/hardware interrupts | In protected mode `INT3`, `INTO`, `INT n`, and injected interrupts stop as `#UD` at the core boundary. | Guest handler, gate, task, or privilege transfer. |
| Existing T257 path | `LGDT`, `LMSW`, GDT selector loads, and same-CPL far control remain unchanged. | Any 80386, paging, LDT, TSS, or CPL feature. |
| Evidence | A focused corpus proves a configured IDT cannot be used after PE, plus the established full GCC gate. | A second executor, host shortcut, or test-only runtime route. |

## Rules And Stop Conditions

`core/machine` remains the sole guest-state owner and `core_machine_run()` the
sole executor. The implementation must preserve real-mode interrupts and
real-mode `LIDT`/`SIDT`; it may not remove retained decoder code. Stop for
owner direction if the closure requires an IDT contract, a guest handler
transfer, or a profile-specific exception mechanism.

S6 may edit only the protected-mode decoder gate, the T257 corpus, task/status
tracking, and necessary CMake registration. S7 runs the current GCC gate and
records the task-identity artifact revision and SHA-256 before closing the task.

## S7 Evidence

The original S7 run used the former independent-revision name
`nxvm_0_5_0256.exe` and passed all 37 static/ownership checks and 91/91 CTest
cases. M5 Td S28 supersedes that current-artifact name with task-identity
`nxvm_0_5_0257.exe`, SHA-256
`E69FC24A8E1113E4A7AED552F9C5B944372F4FCF73FFB0D669CC14B6C375A1F1`.

The S6 corpus proves PE-state `LIDT` stops as `#UD`, and a real-mode configured
IDT still cannot transfer a protected-mode `INT3` to its handler. Real-mode
interrupt and table behavior remains on the retained path.
