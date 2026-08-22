# T435 S2 - 80286 Timing Implementation Audit

The [80286 manifest](t435-s2-80286-timing-manifest.json) consumes every
successful-retirement family in the accepted
[Intel ledger](t435-s1-80286-ledger.md).  It freezes 286 L3 base keys; 80286
has no range-derived L2 key.

Current selection is the 80286-private `string I/O -> dynamic arithmetic ->
primary -> control-stack -> 80286 fallback` branch in
`src/core/machine/cpu_timing.c`; it cannot enter 80386 candidates or the
compatibility endpoint. Static inspection finds real 80286-specific
primary/fallback treatment for Group-2, segment loads/stores,
`BOUND`, `ARPL`, system opcodes, mode-sensitive paths and 80286 EA/odd-word
helpers.

The existing `core-machine-80286-instruction-timing-ledger-smoke` was rebuilt
and run successfully. It exercises representative real/PM segment forms,
LDS/LES, stack, BOUND/ARPL, descriptor and task registers, EA/odd word,
Group-2, strings, ports and branches. It does **not** emit a manifest key ID,
retirement origin or every Appendix-B input, so it proves only those vectors.
No key is marked conforming from that aggregate result.

S10 adds `core-machine-80286-timing-manifest-runner`, which attaches the
retirement observer to generated I286 metadata and validates 127 exact fallback/
primary
base recipes. It intentionally cannot emit a partial result JSON, so this is
mechanism evidence only; the 807-key result-contract verifier remains the
conforming gate.

The strict manifest baseline is 286 base keys: 239 `missing-test`, 47
`missing-input` and zero `unallocated`; zero is conforming or wrong-value.
`I286-XLAT` has an exact 5-clock functional smoke vector but remains
`missing-test` until a generated-key result records its provenance and inputs.
It also generates 459 legal single-axis keys and 62 legal combined keys. `EA-BID`,
`ODD-WORD`, `REP-PHASE`, `NEXT-BYTE` and legal memory-RMW `LOCK` are not prose annotations: each has
one finite key, source rule, route, batch and regression ID. The two latter
classes remain `missing-input` because current retirement observation cannot
publish the required normalized inputs.

This is an implementation requirement, not a deferral. B0 creates one
form/context publisher; B1 implements every Appendix-B exact/formula rule;
B3 exposes EA, odd-word, mode/path, segment, next-byte, repeat and legal LOCK inputs; B4 consumes
every key and removes all parallel successful selectors. External
READY/BUSY/arbitration remain excluded by the ledger boundary.

Closure is impossible while any key is `wrong-value`, `unallocated`,
`missing-input` or `missing-test`.  There is no L1 fallback.

Markers: `M5:T435:S2:80286-IMPLEMENTATION-AUDIT:OK`;
`M5:T435:S2:80286-REPAIR-SET:OK`.
