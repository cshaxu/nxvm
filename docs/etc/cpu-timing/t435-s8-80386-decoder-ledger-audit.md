# T435 S8 - 80386DX Decoder-Ledger Audit

## Result

The audit repairs the former compressed S1/S2 80386DX ledger and establishes a
reproducible decoder-to-manual-to-manifest check. Manual distinctions now
include bit-operation source/index/size forms, I/O port forms, control-transfer
paths, system and special-register forms, XLAT and flag/control instructions.

`Verify-80386DecoderLedger.ps1` passes with:

```text
M5:T435:S8:I386-MANUAL-DECODER-PARTITION:OK:base=454:primary=253
M5:T435:S8:I386-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:canonical=1431
M5:T435:S8:I386-S2-CANONICAL-STATUS-RECONCILED:PASS:missing-test=261:missing-input=192:unallocated=1
M5:T435:S8:I386-CANONICAL-STATUS-RECONCILED:PASS:missing-test=342:missing-input=1069:unallocated=20
```

The 66 accepted escaped families are exact-match checked, and the 253 primary
opcodes are range-partitioned with prefixes and x87 escapes explicit. This is
decoder membership evidence only; it does not promote any timing result to
conforming.

## Transfer

The complete 1,431-key nonconforming universe transfers intact to S11. No
manual row, valid decoder family, or legal context is left as an unnamed later
audit. The remaining work is implementation and focused per-key proof under
the existing manifest-result contract.

Marker: `M5:T435:S8:I386-DECODER-LEDGER-AUDIT:OK`.
