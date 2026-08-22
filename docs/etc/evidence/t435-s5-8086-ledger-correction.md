# T435 S5 - 8086 Ledger Correction Evidence

## Corrective scope

S5 corrects successful-decoder corpus omissions found after S4 acceptance:
opcode `D7` (`XLAT`) and the 8086-only opcode `0F` (`POP CS`) were implemented
by the decoder but absent from the S1 ledger, S2 manifest, and S4's frozen
649-key execution universe. S4's artifact remains immutable historical
evidence; it is not rewritten as a later result.

Intel *The 8086 Family User's Manual*, order 9800722-03, Table 2-21, gives
`XLAT source-table` an exact 11 clocks. The source-table byte reference can
legally carry a segment override, so Table 2-21's documented +2 prefix term
produces the second key `I86-XLAT-SEGMENT` at 13 clocks. An odd-word term does
not apply to this byte transfer.

## Actual implementation and result

`core_machine_legacy_source_instruction_cost()` now assigns the owner-local
8086 `XLAT` source form and returns 11 plus the observed segment-override
term, and assigns the 8086-only `0F` form the existing 8-clock `POP` source
form. The runner verifies both `XLAT` timing keys, observes the segment formula
input, separately proves the functional `DS:BX+AL -> AL` byte transfer, and
proves that `POP CS` loads `1234h`, derives base `12340h`, and advances SP by
two on a real 8086 Core machine.

The generated [`S5 runtime artifact`](../cpu-timing/t435-s5-8086-timing-results.json)
contains 1,053 unique records: 989 L3 and 64 named `L2:G3`, with zero
unallocated successful retirements. It supersedes S4 only as the current
implementation result.

## Result-record integrity correction

The S5 runner originally retained its selected manifest-record index after a
retirement. A following semantic-only probe could therefore overwrite that
record's generated result even though the probe itself passed. The defect was
observable in the generated `I86-POP-SEG-CS` row: the real `POP CS` probe
asserted the manual 8 clocks, while the artifact could contain a later,
unrelated observation. This was an evidence-publication defect, not an
accepted timing disposition.

The capture now consumes and clears the selected record index after precisely
one real retirement. Regenerating the artifact records `I86-POP-SEG-CS` as
8 clocks, source form 84 and origin 6, and restores the independently checked
observations for the affected repeat/context rows. The runner's semantic
predicates also now assert the architectural results for ALU, adjustment
(`AAA`/`AAS`/`DAA`/`DAS`/`AAD`/`AAM`), data/stack, Group-3, branches, direct
flags, compare/test, unary, `LAHF`/`SAHF`, and string index updates; they do
not replace the existing wider form-specific CPU smoke suite.

## Reproducible evidence

The following commands passed from the configured MinGW Makefiles build:

```text
core-machine-8086-timing-manifest-runner
M5:T435:S5:I86-MANIFEST-PROBE:PASS:1053/1053

Verify-8086TimingResults.ps1 -ResultPath .../t435-s5-8086-timing-results.json
8086 timing results verified: conforming_keys=1053

Verify-8086DecoderLedger.ps1 -ResultPath .../t435-s5-8086-timing-results.json
M5:T435:S5:I86-XLAT-CORRECTION-SLICE:PASS
M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053
```

The implementation applies the 8086 `LOCK` two-clock term at the one
post-selection owner. The runner executes every S2-expanded direct-form and
legal context retirement, including LOCK crossings of legacy, primary,
control-stack and string routes. The decoder verifier compares all canonical
S1/S2 keys with all results in both directions and reports the zero-difference
marker above; the former 19-key RMW-only LOCK subset is historical only.
The runner also writes a decoder-derived inventory of all 233 lexical primary
opcode candidates; the verifier compares its exact byte set and semantic-only
`LOCK` prefix against the 227-direct-plus-seven-prefix reconciliation contract.
The four-profile manifest contract now has 3,699 canonical keys (8086: 1053;
80186: 602; 80286: 812; 80386DX: 1232).
