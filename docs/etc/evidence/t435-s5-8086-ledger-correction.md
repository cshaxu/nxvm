# T435 S5 - 8086 Ledger Correction Evidence

## Corrective scope

S5 corrects the successful-decoder corpus omission found after S4 acceptance:
opcode `D7` (`XLAT`) was implemented by the 8086 decoder but absent from the
S1 ledger, S2 manifest, and S4's frozen 649-key execution universe. S4's
artifact remains immutable historical evidence; it is not rewritten as a
651-key result.

Intel *The 8086 Family User's Manual*, order 9800722-03, Table 2-21, gives
`XLAT source-table` an exact 11 clocks. The source-table byte reference can
legally carry a segment override, so Table 2-21's documented +2 prefix term
produces the second key `I86-XLAT-SEGMENT` at 13 clocks. An odd-word term does
not apply to this byte transfer.

## Actual implementation and result

`core_machine_legacy_source_instruction_cost()` now assigns the owner-local
8086 `XLAT` source form and returns 11 plus the observed segment-override
term. The 8086 manifest runner verifies both timing keys, observes the segment
formula input, and separately proves the functional `DS:BX+AL -> AL` byte
transfer on a real 8086 Core machine.

The generated [`S5 runtime artifact`](../cpu-timing/t435-s5-8086-timing-results.json)
contains 651 unique records: 619 L3 and 32 named `L2:G3`, with zero
unallocated successful retirements. It supersedes S4 only as the current
implementation result.

## Reproducible evidence

The following commands passed from the configured MinGW Makefiles build:

```text
core-machine-8086-timing-manifest-runner
M5:T435:S5:I86-MANIFEST-PROBE:PASS:651/651

Verify-8086TimingResults.ps1 -ResultPath .../t435-s5-8086-timing-results.json
8086 timing results verified: conforming_keys=651

Verify-8086DecoderLedger.ps1 -ResultPath .../t435-s5-8086-timing-results.json
M5:T435:S5:I86-LEDGER-DIFF:PASS:0
```

The CTest slice containing the runner, result verifier and decoder-ledger
verifier passed 3/3. The four-profile manifest contract passed with 3,297
canonical keys (8086: 651; 80186: 602; 80286: 812; 80386DX: 1232).
