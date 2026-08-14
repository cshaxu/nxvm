# T366 S16: 80286 XLAT Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists `XLAT = Translate byte to AL` (`D7`) at five clocks in both real-address
and protected virtual-address modes. S16 applies that fixed value only after
the existing unprefixed 80286 XLAT handler successfully retires.

## Construction And Sweep

`core_machine_80286_source_instruction_cost()` remains the sole
successful-retirement publisher. Its `D7` branch assigns five ticks after the
shared primary and control/stack classifiers decline the opcode. The focused
80286 timing-ledger smoke writes the selected `DS:BX+AL` byte, executes `D7`,
and proves both the resulting `AL` value and five-tick publication.

The sweep covers the `D7` dispatch, XLAT execution handler, primary and
control/stack timing classifiers, all profile classifiers, direct 80286
fallback routing, and existing semantic/fault tests. Prefix timing, fault and
delivery timing, 8086/80186/80386 values, memory/bus waits, device service and
physical time remain their existing explicit receivers. No public ABI or
device behavior changes.

## Verification

The focused `core-machine-80286-instruction-timing-ledger-smoke` passed and
emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`. The configured
`run-current-smokes` gate passed. Documentation governance and `git diff
--check` are recorded with the implementation delivery and acceptance review.
