# T435 S2 - 80286 L3 Implementation Tracker

The machine-readable [80286 manifest](t435-s2-80286-timing-manifest.json) is
the sole closure ledger. It expands 286 unique L3 successful-retirement keys,
459 legal single-axis context keys and 62 legal combined keys from the
accepted [80286 ledger](t435-s1-80286-ledger.md). Every key has an Appendix-B
source rule, observed route, current status, implementation batch and focused
regression identity.

Run `powershell -NoProfile -ExecutionPolicy Bypass -File
tools/Verify-80286TimingManifest.ps1` before every tracker-changing P.  The
gate rejects an incorrect profile, absent record fields, non-L3 substitution,
duplicate keys and count drift.  A later focused timing test must consume the
same IDs and emit value, mode/path/EA/repeat/next-byte inputs, origin and the
unallocated bit under the shared [manifest result contract](t435-s2-manifest-result-contract.md).

| key families | required L3 rule | closure batch |
| --- | --- | --- |
| arithmetic, Group-2, moves, stack, ports, strings and flags | Appendix B exact, EA/alignment or repeat formula | B0+B1+B3+B4 |
| branches and `INTO` | outcome-specific Appendix B result | B0+B1+B4 |
| call/jump/return/IRET/INT paths | real/PM/privilege/gate/task path plus next-byte terms | B0+B1+B3+B4 |
| system instructions | privilege/memory form and Appendix-B result | B0+B1+B3+B4 |

The existing 80286 ledger smoke is named evidence for its representative
vectors only; it is not a manifest-key result producer. Therefore the initial
manifest reports 239 `missing-test`, 47 `missing-input`, zero `unallocated`,
and zero conforming. `I286-XLAT` has a real 5-clock smoke vector through the
80286 fallback; it remains `missing-test` until the generated-key runner
records its complete result-contract observation.
No aggregate smoke outcome may change an individual key's status.

`B0` is one sole 80286 form/context publisher. `B1` makes all 286 manual
rules exact/formula-conforming. `B3` supplies each legal EA, odd-word,
real/protected, privilege/path, segment, next-byte, repeat and legal LOCK input. `B4` requires one
focused result per expanded key and removes old successful selectors. There is
no L2 or L1 exit: every key must be L3 `conforming`.

Markers: `M5:T435:S2:80286-IMPLEMENTATION-TRACKER:OK`;
`M5:T435:S2:80286-TRACKER-CLOSURE-PREDICATE:OK`.
