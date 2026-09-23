# T436 S8 80286 Result Closure

S8 authorizes the one retained 80286 manifest writer after the accepted
S3--S7 partitions have together observed the complete canonical set. The
runner emits `t436-s8-80286-timing-results.json` from successful retirement
observations only; the result verifier accepts exactly 771 L3 records, with
no duplicate, unallocated, wrong-value, missing-input or missing-test record.
The partition totals remain S3/S4/S5/S6/S7 = 303/129/54/193/92.

The cross-profile audit found a real evidence defect only in the 8086 result
and smoke expectations: the old data charged one odd-word term to paths that
perform two word transfers, while Group-2 RMW paths require two terms. The
repair updates 72 8086 tick results: 56 Group-2 RMW contexts plus 16
memory-stack and indirect-far-pointer contexts. The existing Core
transfer-count implementation is unchanged. The rebuilt 8086 runner passes
1053/1053 and its result and decoder-ledger verifiers pass. The 80186 runner
passes 279 observed base records and its 616-key result verifier reports no
tick-value change (only 54 refreshed formula-input observations). The 80386
protected-I/O and paging regressions pass; it does not share the legacy
transfer-count owner.

Current-gate reconciliation found several stale smoke expectations that had
fallen behind these generated timing results. They are now tied to the
verified 8086/80186/80286 outcomes; no Core runtime, ABI, board/event policy,
source import, or public contract changed.

```text
M5:T436:S8:I286-RESULT-CLOSURE:PASS:canonical=771
80286 timing results verified: conforming_keys=771
8086 timing results verified: conforming_keys=1053
80186 timing results verified: conforming_keys=616
M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053
M5:T357:S7:80386-PROTECTED-IO-TIMING:OK
M5:T258:S3:I386-PAGING:CORPUS:OK
current-gate: 292/292 passed
```

The required developer artifact target `vm-0-5-0434` builds without work and
publishes `build/output/nxvm_0_5_0434.exe`, SHA-256
`F8562F4623D53303470408837249CF89C985E0A0B1130212BBD9CE26B5E1B38A`.
