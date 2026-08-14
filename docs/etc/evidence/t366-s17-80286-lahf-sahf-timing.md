# T366 S17: 80286 LAHF/SAHF Timing

## Source And Scope

The [AMD 80286 instruction summary](https://www.bitsavers.org/components/amd/x86/_dataSheets/1985_80286.pdf)
lists `LAHF` (`9F`) and `SAHF` (`9E`) at two clocks in both real-address and
protected virtual-address modes. S17 allocates those fixed values only after
the existing unprefixed 80286 handlers successfully retire.

## Construction And Sweep

`core_machine_80286_source_instruction_cost()` remains the only successful
retirement publisher. Its combined `9E`/`9F` branch assigns two ticks after
the shared primary and control/stack classifiers decline the opcodes. The
focused timing-ledger smoke proves each opcode publishes two ticks and asserts
the transferred/preserved AH and FLAGS state. The existing
`core-machine-lahf-sahf-smoke`, exercised by the current-smokes gate, retains
broader semantic, prefix, profile and fault coverage.

The sweep covers the opcode dispatch/handlers, primary and control/stack
classifiers, every profile classifier, direct 80286 fallback routing and
semantic proof. Prefixes, fault/delivery timing, other profiles, memory/bus
waits, device service and physical time remain their existing receivers. No
public ABI or device behavior changes.

## Verification

P1 correctly established the source timing branches but omitted the
packet-required focused AH/FLAGS checks. P2 adds that one consolidated proof.
The focused `core-machine-80286-instruction-timing-ledger-smoke` passed and
emitted `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; the configured
`run-current-smokes` gate passed. Documentation governance and `git diff
--check` passed. Coordinator review of P2 `7d9e27be` confirms the one
retirement-owner timing change remains bounded and the new direct checks cover
both time publication and AH/FLAGS preservation/transfer.
