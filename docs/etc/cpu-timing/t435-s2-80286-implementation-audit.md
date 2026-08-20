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
helpers.  That is useful coverage, but not evidence that a manual key is
correct: all 293 keys begin `missing-test` or `missing-input` until a focused
result proves ticks, formula inputs, origin and unallocated=false.

The manifest deliberately begins all mode/path and next-instruction-byte keys
as `missing-input`: existing helpers do not publish enough normalized inputs
to prove gate, task, privilege-return or direct-transfer byte terms.  This is
an implementation requirement, not a deferral.  B0 creates one form/context
publisher; B1 implements every Appendix-B exact/formula rule; B3 exposes EA,
odd-word, mode/path, next-byte and repeat inputs; B4 consumes every key and
removes all parallel successful selectors.  External READY/BUSY/arbitration
remain excluded by the ledger boundary.

Closure is impossible while any key is `wrong-value`, `unallocated`,
`missing-input` or `missing-test`.  There is no L1 fallback.

Markers: `M5:T435:S2:80286-IMPLEMENTATION-AUDIT:OK`;
`M5:T435:S2:80286-REPAIR-SET:OK`.
