# M2 T8: Cycle And Interrupt Refinement

## Admission And S Plan

T8 starts from the accepted CPU/bus and debugger products. It refines their one
execution path in four outcome-bearing stages:

| Subtask | Outcome | Closure proof |
| --- | --- | --- |
| S1 | Admit authoritative timing sources and a bounded transfer ledger. | Each claimed bus and interrupt dimension names its source, owner and test form. |
| S2 | Instrument and prove ordered CPU bus transfers. | Fetch, read, write, dummy, RMW, branch and stack/control traces match independently derived fixtures. |
| S3 | Implement and prove cycle-sampled reset/IRQ/NMI behavior. | Pin transition fixtures prove recognition, priority, vector entry, pushes and retired-instruction accounting. |
| S4 | Close timing regression and transfer end-to-end reliability work. | Step/run checkpoints and full x64/x86 suites pass; deferred lifecycle/fault work names its receiver. |

## Boundaries

This task refines the existing CPU and bus; it must not introduce a second
interpreter, global cycle adjustment or an emulator-comparison-only result.
PPU, APU, controller and sub-cycle claims remain outside the accepted scope.

## S1 Ledger And Source Disposition

The primary timing basis is MOS Technology's 1976 *MCS6500 Microcomputer
System Hardware Manual*, publication 6500-10A, Appendix A, already registered
as MOS-H. Its single-cycle tables establish the admitted sequence classes:
opcode/operand fetch; indexed no-cross/cross reads; indexed stores; zero-page
and absolute RMW old/new writes; stack/control/BRK/return; and taken/crossing
branches. NES-IRQ and the Visual6502 timing research remain the variant-specific
authority for RP2A03 IRQ/NMI recognition and vector gates.

The S1 source audit finds that the accepted CPU has only aggregate cycle counts:
its bus calls occur for functional fetches and reads/writes while dummy cycles
are not represented as ordered transfers. Consequently, aggregate cycle tests
cannot prove the T8 outcome. S2 therefore owns one trace-aware, cycle-ordered
CPU/bus path with copied records. It begins with the source classes above and
does not claim PPU/APU timing or sub-cycle transistor equivalence.

## S2 Ordered Transfer Closure

S2 closes through the existing production CPU and Bus path. The recorder
captures copied reads and writes, including discarded reads, so that direct
fixtures can assert order, address, direction and data. It covers opcode and
operand fetches; implied and indexed reads; zero-page, absolute and indirect
indexed writes; no-cross and cross-page dummy reads; RMW read/old-write/new-
write sequences; taken and cross-page branches; and stack/control forms
including BRK/RTI and JSR/RTS.

`m2-t8-s2-transfer-closure.md` records the fixture-to-class mapping. Both x64
and x86 rebuilt component and integration executable suites pass. S2 does not
claim cycle-sampled interrupt behavior; that work transfers intact to S3.

## S3 Reset And Interrupt Closure

S3 closes reset/IRQ/NMI entry through the one production CPU/Bus path. It adds
the bounded cycle hook used by future hardware and current fixtures to update
the existing interrupt pins immediately before a bus transfer. Reset, entry
transfers, priority, NMI vector takeover/defer/loss gates, I polling behavior,
branch boundaries and budget/retirement accounting are independently covered.
The full x64/x86 executable suites pass. Evidence is recorded in
`m2-t8-s3-interrupt-closure.md`; final end-to-end timing regression transfers
to S4.
