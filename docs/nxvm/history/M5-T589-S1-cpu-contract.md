# M5 T589 S1: basic hardware admission and CPU contract

Coordinator reviewed 8d9e78260 under owner-authorized single-session operation.
The queue implements the requested hardware, board, machine and product order.
The AMD contract and both-width 5/5 baseline are recorded. No source or release
changed. The S2 implementation uses the audited CPU address/reset boundary.

## Archived packet

# M5 T589 S1 - Delivered: Am386SXLV hardware contract

[CPU contract and baseline](../etc/evidence/t589-s1-am386sxlv.md) records the
existing dual-width results and remaining CPU implementation batches.

| Field | Required record |
| --- | --- |
| Identifier Mode | New M5 T589 S1. T588 is closed at 82c7e5ff3. |
| Admission And Approval | Owner on 2026-11-19 requested the four-layer reorder, explicit admission of the hardware T and sequential single-session coordinator/executor operation. |
| Objective | Resolve the AMD 386SXLV execution, address and external bus implementation delta. Establish which behavior needs dedicated modeling and a dual-width diagnostic baseline. |
| Non-goals | No production ROM boot or board wiring in S1. No shared CPU API changes before exact delta review. |
| Reference Baseline | 82c7e5ff3, T588 reset proof, HP 5965-0244 and AMD 21020. |
| Candidate Proposal | proposals/m5-ob300-hardware.md, CPU row; S2 receives the resulting implementation contract. |
| Files And ABI Surface | NXVM queue/current/proposal/evidence and bounded diagnosis only. No ABI changes. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, source policy, architecture/layout and shared execution, documentation, architecture, coding rules. |
| Verification | Inspect actual Core profile, reset address, bus and SMM owners. Run existing targeted CPU and ROM mapping tests in both cached Release trees. |
| Expected Markers | The current 80386 path, proven reusable behaviors and required SXLV differences each have an owner and observable verification. |
| Asset Needs | No proprietary BIOS execution in S1. Temporary output build/t589-s1 is owned by S1. Each process 120 s, 60 s without progress, total scratch limit 16 MiB. |
| Reporting Requirements | Summarize evidence, exact scope and qualification limits. |
| Stop Conditions | No copying a 486 SMM model or declaring the generic 80386 profile SXLV-complete. |
| Exit Criteria | CPU gap and two-width foundation checks recorded, queue and phase boundaries reconciled, reviewed delivery committed and pushed. |
| Original Owner Request | Replan the four layers as T tasks and begin the first one, basic hardware. |
| Similar-Issue Sweep | Check reset alias versus ordinary bus address, architectural operand width versus electrical data bus, and 486-only feature gates versus AMD 386 SMM. |
