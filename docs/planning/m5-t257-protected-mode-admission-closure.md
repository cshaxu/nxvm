# M5 T257: Protected-Mode Admission Closure

## Status

**S5 active.** S6 implements the bounded closure; S7 verifies it and records
the next developer artifact revision.

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
tracking, and necessary CMake registration. S7 runs the current GCC gate,
records the next unused artifact revision and SHA-256, then closes the task.
