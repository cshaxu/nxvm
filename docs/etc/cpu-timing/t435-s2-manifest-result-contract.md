# T435 S2 - CPU Timing Manifest Result Contract

## Purpose

This is the common implementation admission contract for the four accepted
CPU timing manifests. It makes Td S122's per-key runtime-result requirement
executable before B0 begins. A tracker verifier is a structural gate only; a
profile cannot close from template expansion or representative smoke output.

## Generated key and result record

The B0 generator expands every manifest template to one canonical record:

```json
{
  "key_id": "I86-...",
  "level": "L3 or named L2",
  "encoding": "decoded opcode/group/operand form",
  "context": "canonical legal axes only",
  "source_rule": "S1 manual/model rule",
  "current_route": "origin selector or unallocated terminal",
  "implementation_batch": "B0..B4",
  "regression_id": "focused test identity"
}
```

The focused result file contains exactly one result for each generated key:

```json
{
  "key_id": "I86-...",
  "ticks": 0,
  "formula_inputs": {"name": "value"},
  "form_id": "published decoder form",
  "retirement_origin": "published origin",
  "source_timing_unallocated": false,
  "passed": true
}
```

The test may emit multiple probes for one key only when the key's S1 formula
requires partitions. The reducer must retain all probe identities and marks
the key conforming only when every required partition passes.

## Required verifier behaviour

For each profile, the result verifier rejects: missing/duplicate key results;
unknown keys; wrong source rule or level; missing required formula input;
unallocated successful retirement; a route/origin mismatch; a target value or
manual-bound failure; or any result that does not pass. It derives, rather
than hand-edits, the manifest status totals.

`encoding` and `context` are generated records, not free prose in a test.
Template IDs and axes remain the compact authoring form, but B0 must emit the
fully materialized record before a key can receive a result. Prefix bytes that
do not change a legal semantic axis never create a separate key.

## Cross-profile completion rule

All four result files use this schema. B0 is complete only when it covers the
union of the four generated manifests; B1/B2/B3 may not close a profile by
excluding another profile's corresponding legal form. B4 closes T435 only
when all four result verifiers report zero nonconforming keys and the old
successful-retirement selectors are absent.

Markers: `M5:T435:S2:MANIFEST-RESULT-CONTRACT:OK`;
`M5:T435:S2:CROSS-PROFILE-RESULT-CLOSURE:OK`.
