# M5 T593 S2: Indexed timing and exact clocks

The coordinator reviewed the fixed timing representation, fractional
conversion, receiving changes and verification through `168f253bc`.
The [actual-change and test record](../etc/evidence/t593-s2-ob300-indexed-timing.md)
closes the verified batch; original DOS still exceeds the usability targets.
S3 receives the shared memory and board performance investigation.

## Archived S2 packet

# M5 T593 S2 - Active: Exact CPU timing and clock hot paths

[S1 is closed](../history/M5-T593-S1-ob300-performance-baseline.md).
The [performance proposal](../proposals/m5-ob300-performance.md) owns
the original usability goals. The 0.5.0592 release is the measurement baseline.

| Task | Status |
| --- | --- |
| T592 | Closed. [Product-stage verification](../history/M5-T592-ob300-product-qualification.md). |
| T593 S2 | Active. Replace fixed timing searches and verify the exact clock optimization independently. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T593 S2 after S1. |
| Admission And Approval | The owner authorized the proposed fixed timing table and exact clock investigation on 2026-11-21. |
| Objective | Directly index fixed source timing by the existing form. Check the independently justified exact fractional conversion. Measure both against unchanged guest outcomes. |
| Non-goals | No new dispatch path, preview cache, timing downgrade, phase approximation, or public ABI. |
| Reference Baseline | S1 evidence at f45c34b85 and current 0.5.0592 releases. |
| Candidate Proposal | [Performance scope and S sequence](../proposals/m5-ob300-performance.md), S2. |
| Files And ABI Surface | Shared X86 Core timing and clock internals, their manifest, relevant existing tests, NXVM static test and evidence. Shared sources trigger 16 release EXEs with one target per P. |
| Applicable Rules | NXVM reading set, shared Execution/Architecture/Coding/Documentation, X86 Core local guidance and source policy. |
| Verification | Check all old table cells and missing forms; reuse both-width timing/clock coverage and compare isolated changes on original DOS. Build, unit, integration and 16-artifact qualification through the cached graphs if changes are retained. |
| Expected Markers | O(1) fixed lookup with identical ticks and attribution, exact clock phase and tick delivery, measured guest performance and no material regression. |
| Asset Needs | build/t593-s2, owned by NXVM. Use disposable read-only DOS fixture and copied System Card, max 200 MiB and 300 s per replay, 120 s no progress. Remove disposable media after probes exit. |
| Reporting Requirements | Give per-change timing and actual diff, quantitative limits and unverified native paths. |
| Stop Conditions | Reject known guest regressions and report timing classification evidence before expanding scope. |
| Exit Criteria | Reviewed production diff and evidence, unchanged timing contracts, required repository tests and verified releases. |
| Original Owner Request | Apply the S1 optimization recommendation after reviewing use of timing and decode tables. |
| Similar-Issue Sweep | Check fixed lookup callers, related repeat rows, source-form checks and existing clock coverage. |
