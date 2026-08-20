# T435 S2 - 80286 Timing Implementation Audit

The [80286 manifest](t435-s2-80286-timing-manifest.json) consumes every
successful-retirement family in the accepted
[Intel ledger](t435-s1-80286-ledger.md).  It freezes 293 L3 base keys; 80286
has no range-derived L2 key.

Current selection is `string I/O -> dynamic arithmetic -> primary ->
control-stack -> 80286 fallback -> source-unallocated -> retirement
observation` in `src/core/machine/machine.c`.  Static inspection finds real
80286-specific primary/fallback treatment for Group-2, segment loads/stores,
`BOUND`, `ARPL`, system opcodes, mode-sensitive paths and 80286 EA/odd-word
helpers.

The existing `core-machine-80286-instruction-timing-ledger-smoke` was rebuilt
and run successfully. It exercises representative real/PM segment forms,
LDS/LES, stack, BOUND/ARPL, descriptor and task registers, EA/odd word,
Group-2, strings, ports and branches. It does **not** emit a manifest key ID,
retirement origin or every Appendix-B input, so it proves only those vectors.
No key is marked conforming from that aggregate result.

The strict manifest baseline is 293 base keys: 241 `missing-test` and 52
`missing-input`; zero is conforming, wrong-value or unallocated. It also
generates 369 legal single-axis keys and 43 legal combined keys. `EA-BID`,
`ODD-WORD`, `REP-PHASE` and `NEXT-BYTE` are not prose annotations: each has
one finite key, source rule, route, batch and regression ID. The two latter
classes remain `missing-input` because current retirement observation cannot
publish the required normalized inputs.

This is an implementation requirement, not a deferral. B0 creates one
form/context publisher; B1 implements every Appendix-B exact/formula rule;
B3 exposes EA, odd-word, mode/path, next-byte and repeat inputs; B4 consumes
every key and removes all parallel successful selectors. External
READY/BUSY/arbitration remain excluded by the ledger boundary.

Closure is impossible while any key is `wrong-value`, `unallocated`,
`missing-input` or `missing-test`.  There is no L1 fallback.

Markers: `M5:T435:S2:80286-IMPLEMENTATION-AUDIT:OK`;
`M5:T435:S2:80286-REPAIR-SET:OK`.
