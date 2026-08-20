# CPU Timing Evidence Ledgers

This indexed subtree holds the evidence-first inputs for the queued CPU timing
program.  It is not a timing authority: Intel manuals remain primary, and a
third-party emulator is a non-imported, version-pinned L2 candidate only.

Each profile block contains a corpus manifest, an exhaustive successful-
retirement form/context ledger, and a reference-model assessment.  A ledger
must state whether its manual gives an exact value, formula, range,
principle-only information, or no timing information.  It must then state a
single proposed L3/L2/L1 outcome.  An L1 row is explicit and never represented
by a silent one-tick default.

T435 S1 block 1 is the 8086 material in
[the 8086 ledger](t435-s1-8086-ledger.md).  Blocks 2--4 are intentionally not
started until owner review of that record.

The current 8086 S2 audit normalizes that ledger's narrative prefix conditions
into the finite [8086 context-key catalog](t435-s2-8086-context-key-catalog.md).
Its [implementation tracker](t435-s2-8086-implementation-tracker.md) is the
sole S2 record for expanded key status, implementation batches and regression
closure.

The binding [Td S122 closure-manifest governance](../evidence/td-s122-cpu-timing-manifest-governance.md)
requires that tracker to become a generated, machine-readable per-key manifest
before any 8086 timing repair begins.
The admitted [8086 manifest](t435-s2-8086-timing-manifest.json) is that
machine-readable source; its templates expand exactly 228 L3 and 16 L2 base
keys before context expansion.
