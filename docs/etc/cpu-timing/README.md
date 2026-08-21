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

The active 80186 S2 block applies the same contract through the
[80186 manifest](t435-s2-80186-timing-manifest.json), its
[implementation audit](t435-s2-80186-implementation-audit.md), and its
[implementation tracker](t435-s2-80186-implementation-tracker.md).  Its
templates expand 253 L3 and 25 explicit fixed-midpoint L2 base keys; legal
context axes are generated from the manifest rather than left as prose.

The active 80286 S2 block records its all-L3 universe in the
[80286 manifest](t435-s2-80286-timing-manifest.json),
[audit](t435-s2-80286-implementation-audit.md), and
[implementation tracker](t435-s2-80286-implementation-tracker.md).

The active final profile block uses the [80386DX manifest](t435-s2-80386-timing-manifest.json), [audit](t435-s2-80386-implementation-audit.md), and [tracker](t435-s2-80386-implementation-tracker.md).

All four profiles share the [context-legality contract](t435-s2-context-legality.md) and implementation-time [manifest result contract](t435-s2-manifest-result-contract.md). Together they require legal semantic contexts, frozen expansion cardinalities and canonical per-key runtime proof; tracker expansion alone cannot close a key.

The [four-profile S2 readiness audit](../evidence/t435-s2-four-profile-implementation-readiness-audit.md) records the only implementation handoff: 3,295 mechanically generated canonical keys, their current nonconforming dispositions, the `cpu_timing.c` ownership boundary and the B0--B4 closure sequence.

The [S3 pipeline evidence](../evidence/t435-s3-cpu-timing-pipeline.md) records
the implemented single selector/publication seam and the result-verifier input
contract. It does not turn any S2 key into a passing runtime result.
