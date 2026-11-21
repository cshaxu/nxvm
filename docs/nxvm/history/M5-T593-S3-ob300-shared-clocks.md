# M5 T593 S3 acceptance

Coordinator reviewed the actual shared code and scheduler regression in
`c878d358b` and the deployment series through `a1e0e1b86`. The four retained
changes remove work from the existing scheduler, timeline, RTC and pacing
paths with no public ABI change. The tested routing variants and optional
board checks are excluded. The [functional release and guest samples](../etc/evidence/t593-s3-release-qualification.md)
support the bounded S3 delivery. The separate x64 sample was slower and the
x86 sample was faster than their S2 counterparts. These are not evidence of
portable end-to-end gains. Both DIR targets remain unmet and are transferred
within T593. This review does not qualify the original usability target.

## Archived S3 packet

# M5 T593 S3 - Active: Core routing and board clock work

[S1 is closed](../history/M5-T593-S1-ob300-performance-baseline.md).
[S2 is closed](../history/M5-T593-S2-ob300-indexed-timing.md).
The [performance proposal](../proposals/m5-ob300-performance.md) owns
the original usability goals. The 0.5.0592 release is the measurement baseline.

| Task | Status |
| --- | --- |
| T593 S2 | Closed after coordinator review of the fixed timing and clock delivery through 168f253bc. |
| T593 S3 | Delivered for review. [Release qualification](../etc/evidence/t593-s3-release-qualification.md) passed the dual-width software matrix. MyOB300 DIR still exceeds its target on both widths. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T593 S3 after S2. |
| Admission And Approval | Owner authorized architectural and algorithmic X86 Core and X86 NXVM optimizations. |
| Objective | Reduce duplicate byte-route checks and skip unnecessary optional-device and Turbo host pacing work. Prove exact chip and scheduler behavior with focused Release benchmarks. Batch retained changes before original DOS qualification. |
| Non-goals | Preserve guest cycles, provider precedence and faults, the scheduler, timing levels and public ABI. No dynamic cache or OB300-specific branch. |
| Reference Baseline | 168f253bc and the [S2 measurements](../etc/evidence/t593-s2-ob300-indexed-timing.md). |
| Candidate Proposal | [Performance S sequence](../proposals/m5-ob300-performance.md), S3. |
| Files And ABI Surface | Private Core memory routing, scheduler and timeline, X86 NXVM RTC clock arithmetic, board advance/deadline and pacing, applicable manifests, existing regressions and NXVM evidence. |
| Applicable Rules | NXVM reading set, shared Execution/Architecture/Coding/Documentation, NXVM source policy and component contracts. |
| Verification | Both widths: route precedence, partial overlap, faults, aliases and board deadline contracts; RTC phase and IRQ, followed by one original DOS replay per width for the accepted batch. Retained production changes require cached repository build, units, integrations and sixteen verified releases. |
| Expected Markers | Identical route and guest outcomes with less repeated work and no material regression. Separate wall time and work counts. |
| Asset Needs | NXVM-owned build/t593-s3. Disposable readonly guest media, 250 MiB and 300 s wall per replay, 120 s no progress. Remove temporary copies after processes exit. |
| Reporting Requirements | Explain each owner, actual diff and workload, quantified limits and unavailable native paths. |
| Stop Conditions | Reject material regressions and preserve evidence. Do not change the usability targets. |
| Exit Criteria | Actual-diff review or evidence-backed rejection per candidate. Retained changes pass required tests and are published. Carry unresolved usability evidence to S4. |
| Original Owner Request | Investigate shared Core and board performance. Accept bounded architectural, time and space reductions. |
| Similar-Issue Sweep | Raw/decoded priority, instruction/data routing, optional PIT, deadline flags and Standard/Turbo pacing. |
