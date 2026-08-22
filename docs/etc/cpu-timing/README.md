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

T435 S1 contains the reviewed [8086 ledger](t435-s1-8086-ledger.md), the
active [80186 ledger](t435-s1-80186-ledger.md), and the later 80286/80386DX
ledgers.  Each profile remains separately admitted for implementation; a
completed evidence register is not an implementation claim.

The current 8086 S2 audit normalizes that ledger's narrative prefix conditions
into the finite [8086 context-key catalog](t435-s2-8086-context-key-catalog.md).
Its [implementation tracker](t435-s2-8086-implementation-tracker.md) is the
sole S2 record for expanded key status, implementation batches and regression
closure.

The binding [Td S122 closure-manifest governance](../evidence/td-s122-cpu-timing-manifest-governance.md)
requires that tracker to become a generated, machine-readable per-key manifest
before any 8086 timing repair begins.
The admitted [8086 manifest](t435-s2-8086-timing-manifest.json) is that
machine-readable source; its templates expand exactly 230 L3 and 16 L2 base
keys before context expansion.

The active S5 [8086 decoder reconciliation contract](t435-s5-8086-decoder-reconciliation-contract.md)
defines the complete current decoder-byte partition that must be proven against
that ledger and manifest before S5 may make a whole-8086 closure claim.

The active 80186 S2 block applies the same contract through the
[80186 manifest](t435-s2-80186-timing-manifest.json), its
[implementation audit](t435-s2-80186-implementation-audit.md), and its
[implementation tracker](t435-s2-80186-implementation-tracker.md).  Its
templates expand 253 L3 and 26 explicit fixed-midpoint L2 base keys; legal
context axes are generated from the manifest rather than left as prose.  The
active S6 [decoder reconciliation contract](t435-s6-80186-decoder-reconciliation-contract.md)
and [P3 audit](t435-s6-80186-decoder-ledger-audit.md) prove decoder/manual
membership and preserve every current nonconforming status for later work.

The active 80286 S2 block records its all-L3 universe in the
[80286 manifest](t435-s2-80286-timing-manifest.json),
[audit](t435-s2-80286-implementation-audit.md), and
[implementation tracker](t435-s2-80286-implementation-tracker.md).
T436 S1's executable [canonical-key partition](t436-s1-80286-key-partition.md)
assigns every retained 80286 base and legal context key to one later
implementation S before any new runtime timing result is claimed.
T436 S2's [result producer](t436-s2-80286-result-producer.md) captures actual
80286 retirement observations but refuses final result emission until all 771
canonical keys have been observed.
T436 S4's [string/repeat closure](t436-s4-80286-string-repeat-closure.md)
proves the complete 129-key string, string-I/O and REP partition through real
retirements and actual repeat inputs.
T436 S5's [conditional-control closure](t436-s5-80286-conditional-control-closure.md)
proves all 54 ordinary conditional-control records, repairs both `INTO`
outcome next-byte timing paths, and records the 771-key legal-universe
correction.
T436 S7's [segment, descriptor and system closure](t436-s7-80286-system-closure.md)
proves all 92 remaining legal 80286 records through real retirement, including
the formerly missing effective-address plus odd-word intersections; final
all-key result publication remains an S8 responsibility.

The active final profile block uses the [80386DX manifest](t435-s2-80386-timing-manifest.json), [audit](t435-s2-80386-implementation-audit.md), [tracker](t435-s2-80386-implementation-tracker.md), and S8's executable [decoder-ledger verifier](../../../tools/Verify-80386DecoderLedger.ps1).

All four profiles share the [context-legality contract](t435-s2-context-legality.md) and implementation-time [manifest result contract](t435-s2-manifest-result-contract.md). Together they require legal semantic contexts, frozen expansion cardinalities and canonical per-key runtime proof; tracker expansion alone cannot close a key.

The [four-profile S2 readiness audit](../evidence/t435-s2-four-profile-implementation-readiness-audit.md) records the original implementation handoff: 3,699 mechanically generated canonical keys, their then-nonconforming dispositions, the `cpu_timing.c` ownership boundary and the B0--B4 closure sequence.

The [S3 pipeline evidence](../evidence/t435-s3-cpu-timing-pipeline.md) records
the implemented single selector/publication seam and the result-verifier input
contract. It does not turn any S2 key into a passing runtime result.
