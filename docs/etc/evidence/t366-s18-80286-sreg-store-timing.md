# T366 S18: 80286 Segment-Register Store Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists `MOV Segment register to register/memory` (`8C`) as two clocks for a
register operand and three clocks plus the documented effective-address term
for memory, in both modes. S18 applies the determinate two-clock value only to
successful, unprefixed legal 80286 register-direct forms (ES/CS/SS/DS).

## Construction And Sweep

`core_machine_80286_source_instruction_cost()` remains the sole successful
retirement publisher. Its `8C` branch verifies a register-direct ModR/M form
and a legacy segment-register encoding before assigning two ticks. The focused
timing smoke executes `MOV AX,DS`, confirms the transferred selector and
upper-EAX preservation, and proves two-tick publication. The existing segment
MOV smoke retains the full form, profile, protected-mode, prefix, rejection
and interrupt-shadow matrix.

Memory/EA `8C`, all `8E` selector-load paths, invalid/386-only encodings,
prefix/fault/delivery timing, other profiles, bus/device service and physical
time remain their established receivers. No public ABI or device behavior
changes.

## Verification

The focused `core-machine-80286-instruction-timing-ledger-smoke` passed and
emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; the configured
`run-current-smokes` gate passed. Documentation governance and `git diff
--check` are recorded with delivery and review.
