# Td S132 8086 Derived Result Reconciliation

## Decision

Publish the regenerated
[`T435 S5 8086 result artifact`](../cpu-timing/t435-s5-8086-timing-results.json).
It is not a timing or decoder change: it restores the artifact's required
correspondence to the **current** Core retirement observation.

`form_id` is an opaque Core-private source-form identifier.  Commit
`52997d58` added earlier enum members while implementing cross-profile x87
handoff/timing ownership.  The identifier is intentionally copied to the
result record, so all existing 8086 form values shifted even though their
decoder, timing, origin, and source-rule observations did not.  The former
artifact therefore described a superseded private enumeration.

## Reproduction and comparison

From the current source baseline, the existing repository-owned runner
`build/audit-current-head/core-machine-8086-timing-manifest-runner.exe` was
run from the repository root.  It rewrote the artifact and reported:

```text
M5:T435:S5:I86-DECODER-LEXEME-CANDIDATES:233:233
M5:T435:S5:I86-MANIFEST-PROBE:PASS:1053/1053
```

The result verifier then reported `conforming_keys=1053`; the decoder ledger
reported both the XLAT correction and
`M5:T435:S5:I86-DECODER-LEDGER-ZERO-DIFFERENCE:PASS:1053`.

A field-by-field comparison of the regenerated artifact to `HEAD` found 1,053
records on each side and **zero** differences across `key_id`, `profile`,
`level`, `source_rule`, `context`, `ticks`, `formula_inputs`,
`retirement_origin`, `source_timing_unallocated`, and `passed`.  All 1,053
opaque `form_id` values changed.  Thus the diff is complete derived-metadata
refresh, not a partial timing-result mutation.

## Boundary and closure

This Td changes no Core source, decoder path, timing value, manual/source
claim, runnable artifact, or public ABI.  It does not reopen T435, which
remains stopped by owner-approved requeue.  Its sole purpose is to leave no
stale generated evidence after the private enum evolved.
