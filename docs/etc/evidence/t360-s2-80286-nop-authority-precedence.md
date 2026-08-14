# T360 S2: 80286 NOP Authority Precedence

## Decision

The Intel *80286 and 80287 Programmer's Reference Manual* (210498-005) has a
source conflict for `NOP`: the formal Appendix B opcode-clock table gives
three clocks, while prose describes two. The owner direction is explicit: when
Appendix B supplies an opcode-clock row, that row is the current source
authority. Therefore the project allocation remains **`NOP = 3` clocks**.

This is a source-selection decision, not an inference that combines, averages,
or otherwise reconciles the two statements. It applies only to the selected
source-clock allocation at successful instruction retirement. It does not
allocate bus waits, prefetch, DMA/HOLD, device service, exception delivery, or
cycle-exact behavior.

## Consumer sweep

| Consumer class | Files swept | Disposition |
| --- | --- | --- |
| Runtime source ledger | `src/core/machine/machine.c` | The 80286 ledger has one `CORE_MACHINE_SOURCE_TIMING_NOP` entry, `3u`; its explanatory comment records the Appendix-B precedence. |
| Profile classifier routes | `src/core/machine/machine.c` | The primary, control/stack, and string/I/O classifiers all return the same source-timing enum; none contains a second numeric NOP allocation. |
| Direct 80286 proof | `tests/machine/core_machine_80286_instruction_timing_ledger_smoke.c` | The focused 80286 ledger smoke executes `90h` and requires three elapsed ticks. |
| Generic and ledger timing proof | `tests/machine/core_machine_instruction_timing_smoke.c`, `tests/machine/core_machine_instruction_timing_ledger_smoke.c`, `tests/machine/core_machine_real_mode_tick_smoke.c` | Their 80286-profile NOP expectations are three ticks; prefixed-NOP cases remain explicitly unallocated-prefix cases, not a conflicting NOP base row. |
| Scheduler, trace, and device consumers | T357 S7 and T359 S3 evidence, plus timing tests named above | The retained downstream arithmetic uses the source NOP allocation of three; no consumer assigns two or one clocks to an unprefixed 80286 NOP. |
| Source and transfer records | T357 S6/S7, T357 S8, T359 S2/S3, T360 S1 evidence; Queue and TODO | All current records retain the formal-table allocation. This record and the TODO below make the unresolved prose conflict explicit rather than leaving an implicit provisional rule. |

The sweep used `rg -n -i "80286.*nop|nop.*80286|NOP = 3|NOP = 2|NOP.*3|NOP.*2|SOURCE_TIMING_NOP" src tests docs CMakeLists.txt cmake`, followed by direct review of every timing-owner and test hit. Product assembler uses of the mnemonic and non-80286 smoke instructions are not timing consumers.

## Deferred source review

`docs/states/TODO.md` carries the exact `TODO(Medium)` admission condition for
the prose/table conflict. A later review must establish edition, page, and
semantic context from primary Intel material, re-sweep every consumer in this
table, and either retain or deliberately revise the formal allocation with a
new source-backed task. It must not silently substitute the prose value,
average the two values, or use an emulator/host benchmark as authority.

## Verification boundary

S2 changes no runtime source, CMake, artifact, ABI, test target, or timing
value. The existing T360 source-inventory verifier and documentation governance
gate remain the mechanical checks; the direct consumer sweep above is the
bounded semantic proof for this source conflict.
