# M5 T257: Protected-Mode Admission Closure

## Status

**Complete.** S5 defined the closure, S6 implemented the bounded decoder gate
and corpus, and S7 verified artifact `0.5.0256`.

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

## S7 Evidence

`current-gcc` built `nxvm_0_5_0256.exe`; `current-gates-gcc` passed all 37
static/ownership checks and 91/91 CTest cases. The artifact SHA-256 is
`09E0C96D2D6AF6B180013344FDEC63B1D7DF66102620D1E107C47946E21D9CB8`.

The S6 corpus proves PE-state `LIDT` stops as `#UD`, and a real-mode configured
IDT still cannot transfer a protected-mode `INT3` to its handler. Real-mode
interrupt and table behavior remains on the retained path.
