# T435 S8 - 80386DX Decoder Reconciliation Contract

## Coverage predicate

The S8 verifier is the only mechanical membership predicate for this profile.
It requires each S2 base key to have a source-located Chapter 17 rule and a
concrete accepted primary or `0F` decoder recipe. It then expands every legal
semantic context and rejects cardinality, family-partition or status drift.

The current closed input universe is 450 base forms plus 961 legal contexts:
1,411 canonical keys. It excludes invalid encodings, faults, event delivery,
external waits and device response; those are not silently treated as L1.

## Current implementation disposition

| status | base keys | canonical keys | implementation meaning |
| --- | ---: | ---: | --- |
| `missing-test` | 257 | 338 | a current selector may exist, but no per-key focused result proves its manual function/timing rule |
| `missing-input` | 192 | 1,053 | form/path/size/segment/repeat/multiplier or privilege input is not yet published to the timing program |
| `unallocated` | 1 | 20 | `XLAT` is the only base form without a 80386 route; the remaining 19 are separately generated legal LOCK contexts whose terminal is also not yet allocated |
| `conforming` / `wrong-value` | 0 | 0 | S8 makes no runtime correctness claim |

The figures are checked by `Verify-80386DecoderLedger.ps1`; a future S11 P
must update the manifest and focused result evidence together, never merely
change a summary count.

## Receiver boundary

S8 closes only the source/manual/decoder/current-status audit. S11 receives
every canonical nonconforming key: it must publish the decoded form, all
manual formula/path inputs, calculated ticks and result origin, then provide a
focused assertion before moving that key to `conforming`. A successful decoder
or a generic fallback cannot satisfy this predicate.

Markers: `M5:T435:S8:I386-RECONCILIATION-CONTRACT:OK`.
