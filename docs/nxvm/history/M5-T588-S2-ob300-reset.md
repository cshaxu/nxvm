# M5 T588 S2: closed reset proof

Coordinator reviewed 031bb4dc2 under owner-authorized autonomous progression.
Both widths execute identical original reset bytes to the first unmodeled
FBh write; callback failure and run failure are separately identified.
Fixed-work baseline is process-inclusive, not full OB300 performance.
The local memory-map figure was visually checked. No production changes or
artifact refresh; owned scratch removed after processes exited. S2 is closed.

## Archived packet

# M5 T588 S2 - Delivered: OB300 reset proof and execution baseline

Dual-width reset proof and fixed-work results are retained in
[the S2 evidence](../etc/evidence/t588-s2-reset-proof.md). Diagnostic processes
have exited and the reviewed 1,697,489-byte scratch directory is removed.
No source, Shared API or product artifacts changed.

S1 is [closed](../history/M5-T588-S1-ob300-inventory.md) after coordinator review
under owner-authorized autonomous progression. T588 remains a design/proof T;
ROM DOS, Windows and full product qualification retain their queued receivers.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T588 S2; S1 closed, S3 planned. |
| Admission And Approval | Owner directed autonomous S planning, goals and execution on 2026-11-19; App architecture follows app-mypc110. |
| Objective | Consume O01-O05/O07/O11: prove candidate reset bytes through the existing executor, identify unsupported routes and measure bounded execution on both host widths. |
| Non-goals | No complete OB300 boot, production device implementation, Shared changes, ROM publication or unqualified silicon timing claims. |
| Reference Baseline | 6954bab1c source inventory; immutable owner System ROM 1.0 ABA and matching Drive C files. |
| Candidate Proposal | proposals/m5-ob300-design.md and etc/evidence/t588-ob300-convergence.md. |
| Files And ABI Surface | NXVM evidence/proposal/history/current plus temporary diagnostic only; no public API changes. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, shared Execution/Documentation/Architecture/Coding, source policy. |
| Verification | Exact input hash; bounded public-Core reset/ROM-alias execution and first-port observation; same fixed-work probe on cached x64/x86 libraries; local manual/config review; actual diff/docs/LF checks. |
| Expected Markers | Mapping hypothesis, first real executed destination and first unsupported hardware access distinguished; reproducible work/time and no complete-machine performance claim. |
| Asset Needs | External archives read only; scratch build/t588-s2 owned solely by S2, at most 256 MiB, each process 120 seconds and no-progress 60 seconds; archive evidence then remove on delivery. |
| Reporting Requirements | Report actual trace and limits, hardware owner decisions, timing comparability and next S3 decisions. |
| Stop Conditions | Bound diagnostics rather than fake devices; do not alter Shared or owner media; unresolved bus mapping remains explicit design evidence. |
| Exit Criteria | Dual-width executor proof/baseline and local source findings archived, no unqualified boot claim, actual diff reviewed, docs validated, complete P committed/pushed and scratch cleaned. |
| Original Owner Request | Independently plan goals and execute T588, using app-mypc110 architecture. |
| Similar-Issue Sweep | Alias versus physical decode, 386 family versus SXLV, file backup versus disk, synthetic throughput versus boot latency, shared test versus App external diagnostic. |

