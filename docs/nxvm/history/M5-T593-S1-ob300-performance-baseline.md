# M5 T593 S1: OmniBook performance baseline accepted

The owner approved proceeding with the measured optimization sequence.
The coordinator reviewed [the baseline](../etc/evidence/t593-s1-ob300-execution-baseline.md)
at f45c34b85. The x64/x86 repository units each passed 717/717. The
instrumented DOS runs reproduced the throughput shortfall; the x64
Windows replay observed switch delivery and the firmware wait. Real RDP
and whole-screen power feedback remain separate verification limits.

S2 takes the fixed timing lookup and the independently measured exact
clock conversion candidate. The underlying timing and release remain
unchanged by S1.

## Archived S1 packet

# M5 T593 S1 - Active: OmniBook execution and interaction baseline

The owner closed [T592](../history/M5-T592-ob300-product-qualification.md)
and admitted [OmniBook performance](../proposals/m5-ob300-performance.md).
MyOB300 retains the published 0.5.0592 x64/x86 pair during the baseline.
Original ROM Windows qualification remains queued behind performance.

| Task | Status |
| --- | --- |
| T592 | Closed by owner on 2026-11-21. S1 through S3 are archived with the product verification and explicit receiving limits. |
| T593 S1 | Active. Establish the controlled baseline and rank measurable causes. |

## S1 brief

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T593 S1 after T592 closure. |
| Admission And Approval | Owner on 2026-11-21 directed closure of the product stage and admission of the queue-head performance investigation. |
| Objective | Measure actual OB300 startup and interaction, and locate where elapsed time accumulates before proposing changes. |
| Non-goals | No production, timing-model or ABI changes in S1, no target relaxation, no Windows installation or RDP qualification claim. |
| Reference Baseline | Git 597aad29d, current 0.5.0592 release pair, T591 S4 controlled DOS baseline, and T592 S3 native DOS and Program Manager observations. |
| Candidate Proposal | [Performance scope and S sequence](../proposals/m5-ob300-performance.md). |
| Files And ABI Surface | NXVM evidence and disposable profiling probes only. No runtime, build or artifact changes in S1. |
| Applicable Rules | NXVM guide and Contributing, Roadmap, design architecture/layout, source policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Reuse cached GCC Release configurations. Sequence fixed-input x64 and x86 runs. Measure cold ROM DOS, ten key echoes and DIR-to-ready, plus separate Windows first-draw, interaction-ready and switch-response checkpoints where observable. Compare guest progress and attributable host cost before choosing changes. |
| Expected Markers | Timestamped guest endpoints, both-width baseline, ranked cost observations with measurement limits, and one justified next batch. |
| Asset Needs | Read-only external System Card and C master; disposable overlay or copy. NXVM executor owns build/t593-s1, capped at 200 MiB and 300 seconds per native probe. Stop after 120 seconds without measurable progress, wait for owned processes, archive evidence and clean disposable data before closure. |
| Reporting Requirements | Separate actual guest observations, sampling estimates, real desktop/RDP coverage, and unresolved waits. |
| Stop Conditions | Preserve the single execution and clock path. Stop a probe at its budget or if it risks owner media. Do not infer a hotkey or performance root cause from first paint or a synthetic loop alone. |
| Exit Criteria | Reproducible both-width measurements, a ranked cost ledger, explicit gaps, and required governance/closure checks. No new EXE for evidence-only changes. |
| Original Owner Request | Close the product stage and start the OmniBook performance investigation. |
| Similar-Issue Sweep | Compare guest polling, device scheduling, instruction/timing work and output delivery across the measured workloads. S1 applies no defect fix. |
