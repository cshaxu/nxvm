# Td S122: CPU Timing Closure-Manifest Governance

`M5:Td:S122:CPU-TIMING-CLOSURE-MANIFEST:OK`

## Decision

T435 must use one versioned, machine-readable timing-closure manifest for each
admitted CPU profile. Narrative ledgers and family summaries are evidence
indexes only; they cannot close an instruction timing key. The manifest is the
sole source of key status, batch membership and derived counts. The 8086
manifest is the first required application; the same contract applies before
implementation begins for 80186, 80286 and 80386DX.

## Required manifest record

Every generated canonical key has exactly one record with: stable `key_id`;
`level` (`L3` or named L2 model); opcode/group/operand/outcome `encoding`;
canonical legal `context`; manual or model `source_rule`; observed
`current_route` (selector, origin, ticks and unallocated state);
`implementation_batch`; focused `regression_id`; and status chosen only from
`conforming`, `wrong-value`, `unallocated`, `missing-input`, `missing-test`.

The manifest generator owns finite expansion from approved key templates. It
rejects duplicate IDs, missing fields, illegal context combinations, unknown
source rules, unassigned nonconforming keys and mismatched derived counts. It
does not expand redundant prefix byte streams into separate timing keys.

## Verification and closure

The focused timing test consumes the manifest and emits one result per
`key_id`, including ticks, formula inputs, form ID, retirement origin and the
unallocated bit. The verifier fails for a missing key/result pair, source-rule
or model-bound mismatch, or any successful unallocated key.

An implementation batch closes only when every member passes and is
`conforming`; obsolete successful-retirement selectors in that batch are
removed. A profile closes only when its manifest has no other status. T435
closes only when all four manifests close. Summaries may report only
verifier-derived counts.

## Boundary and immediate application

This does not alter Intel evidence, select new values, import reference source,
change ABI, assign board time, or authorize L1. Td S121 remains the evidence
authority and T435 remains the implementation authority. T435 S2 resumes only
after it replaces the 8086 template tracker with the first concrete manifest
and verifier contract. Later profiles use the same schema, never a parallel
tracker.

Markers: `M5:Td:S122:CPU-TIMING-CLOSURE-MANIFEST:OK`;
`M5:Td:S122:CPU-TIMING-TRACKER-NO-SUMMARY-CLOSURE:OK`.
