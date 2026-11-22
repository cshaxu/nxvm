# M5 T593 S4 bounded delivery

The owner directed continuation with generic Core execution optimization.
S4 delivered and pushed through `7c6106379`. The audited
[qualification](../etc/evidence/t593-s4-release-qualification.md) verifies the
timing dispatch delivery and receiver artifacts. The original cold and command
latency gates remain open and transfer within T593.

## Archived S4 packet

# M5 T593 S4 - Active: CPU timing dispatch and MyOB300 qualification

[S1](../history/M5-T593-S1-ob300-performance-baseline.md),
[S2](../history/M5-T593-S2-ob300-indexed-timing.md) and
[S3](../history/M5-T593-S3-ob300-shared-clocks.md) are closed.
The [performance proposal](../proposals/m5-ob300-performance.md) owns
the original usability goals. The 0.5.0592 release is the measurement baseline.

| Task | Status |
| --- | --- |
| T593 S3 | Accepted for its bounded functional delivery through a1e0e1b86. No whole-machine acceleration claimed. |
| T593 S4 | Delivered for review. [Release and usability qualification](../etc/evidence/t593-s4-release-qualification.md) records the current performance limits. Owner scope decision pending for unmet gates. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T593 S4 after S3. |
| Admission And Approval | Owner continues to authorize bounded shared Core optimizations and requires static cost analysis before measurement. |
| Objective | Use decoded opcode to avoid impossible specialized 386 timing evaluations, preserving formula order. Complete receiving and MyOB300 usability qualification. |
| Non-goals | No changes to memory routing, guest clocks, timing formulas or grades, public ABI, decoder, protected media or release budgets. |
| Reference Baseline | a1e0e1b86 and [S3 qualification](../etc/evidence/t593-s3-release-qualification.md). |
| Candidate Proposal | [Performance S sequence](../proposals/m5-ob300-performance.md), S4. |
| Files And ABI Surface | Private Core timing selector, source manifest, existing CPU timing tests, NXVM evidence and required deployed artifacts. |
| Applicable Rules | NXVM task reading set, shared execution, architecture, coding and documentation, Core local contract and NXVM media policy. |
| Verification | Derive supported opcode predicates from the evaluator owners first. Use existing per-profile timing and semantics tests on both widths. Only when batching retained changes run the cached receiving matrix, original DOS and sustained input/redraw checks. |
| Expected Markers | Fewer evaluator invocations for ordinary 386 forms. Unchanged chosen evaluator, timing origin, form, next-lexeme and repeat outcomes. Actual MyOB300 latencies against original targets. |
| Asset Needs | NXVM-owned build/t593-s4. No firmware changes. Read-only replay media, 300 s wall and 120 s no progress per width. Remove copies after process exit. |
| Reporting Requirements | Show exact skipped predicates and source diff, verification extent, deployed hashes and separate usability and native limits. |
| Stop Conditions | Reject timing or functional regressions. Do not repeat exploratory performance replays or change the usability gates. Require owner scope decision if targets remain unmet at review. |
| Exit Criteria | Scoped change and receiving checks pass and are delivered. Reconcile boot, command and redraw/interaction targets independently. |
| Original Owner Request | Improve x86 emulator performance including MyOB300, using source-level complexity and operation count to select changes. |
| Similar-Issue Sweep | Both 386-semantic profiles, the remaining Intel profiles, V30, opcode escape and group forms, repeated strings and both architecture widths. |
