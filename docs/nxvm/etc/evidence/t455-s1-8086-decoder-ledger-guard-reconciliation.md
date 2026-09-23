# T455 S1: 8086 Decoder-Ledger Guard Reconciliation

## Cause And Owner Boundary

T447 decomposed the former Core machine coordinator. The 8086 decoder table is
now owned by `cpu_instructions.c`, and source-timing selection is owned by
`cpu_timing_model.c`. `Verify-8086DecoderLedger.ps1` still read `machine.c`
and required the old adjacent POP-segment case layout. That made its static
guard fail despite correct execution and result records.

The sole verifier now reads the two owning files. It verifies `D7 -> XLAT`,
`0F -> INS_0F`, the 8086-only `INS_0F -> POP_CS` route, the 8086-only POP-CS
timing case, the XLAT form/tick selector, the ledger wording, canonical corpus,
decoder inventory, and every generated result. No CPU source, timing rule,
canonical key, or result record changes.

## Verification

- Pre-fix direct decoder verifier: failed at the stale source-layout predicate.
- Manifest runner: `M5:T435:S5:I86-MANIFEST-PROBE:PASS:1053/1053`.
- Result verifier: 1,053 conforming keys.
- Repaired decoder verifier: `I86-XLAT-CORRECTION-SLICE:PASS` and
  `I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053`.
- Direct `ctest -L current-gate --parallel 4`: all 292 selected tests completed
  with zero exit status, including the decoder-ledger and three Console tests.
- `current-gates-gcc`, documentation governance, and diff hygiene pass.

## Change Accounting

Tracked code/test path: `tools/Verify-8086DecoderLedger.ps1`, +10/-6 lines
(net +4). The added lines replace a retired one-file/layout assertion with the
two current owner checks; no wrapper, compatibility path, or second verifier
was introduced.
