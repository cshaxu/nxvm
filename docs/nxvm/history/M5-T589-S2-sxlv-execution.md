# M5 T589 S2: Am386SXLV ordinary execution

Coordinator accepted Shared f37c5eabb and receiver publications b1f203f8d,
3da11633b and 49aef45c6. Review covered CPU versus board address ownership,
reset signature, wrap boundaries, guest paging, test enum probes, manifest
contents and all fourteen PE architecture/hash records. Both full unit
aggregates passed 716/716. No original-ROM or device assembly qualification
was performed. S2 scratch was cleared after the background process exited.
See [implementation evidence](../etc/evidence/t589-s2-am386sxlv-execution.md).

## Archived packet

# M5 T589 S2 - Delivered: Am386SXLV ordinary execution and reset

[S1 is closed](../history/M5-T589-S1-cpu-contract.md).
[S2 implementation and focused proof](../etc/evidence/t589-s2-am386sxlv-execution.md)
are in qualification. An AMD reset-identity review corrected the SXLV
model to DX=2300h. All fourteen refreshed EXEs have verified hashes and
PE architecture. Full unit aggregates passed 716/716 on x64 and x86 after
two stale negative fixtures were repaired.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S2. S1 accepted and closed. |
| Admission And Approval | Owner authorized basic hardware execution and independent S sequencing in single-session dual-role mode on 2026-11-19. |
| Objective | Implement Am386SXLV ordinary CPU selection, 24-bit external addressing and reset on the existing Core CPU path. |
| Non-goals | No SMM implementation, detailed 16-bit bus timing, HP board or firmware boot in this S. |
| Reference Baseline | 8d9e78260, AMD 21020 with its electrical and architectural distinctions, T589 S1 core baseline. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S2 and etc/evidence/t589-s1-am386sxlv.md. |
| Files And ABI Surface | Shared X86 Core CPU and Machine, matching Core tests and manifests. Append one profile enum and retain existing signatures and values. Receiving publication: NXVM, My5160, My5170, MyDeskPro386, MyPC110, MyNES, MyRaidenII. NXVM evidence and governance. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Synthetic reset/far transfer, ordinary instruction and paging regression, A20 separation and physical boundary cases on x86/x64. Apply full affected Shared consumer build/publication/test requirements before delivery. |
| Expected Markers | Existing executor selects a distinct tested SXLV CPU profile. Ordinary accesses resolve within the 24-bit physical address domain without truncating logical addresses or changing other profiles. |
| Asset Needs | Synthetic focused tests and existing stable Ninja build trees. S2 owns build/t589-s2, up to 64 MiB, for scripts and qualification logs. Product publication allows 40 minutes and each unit aggregate allows 30 minutes, following the aggregate's own deadline rules. Review logs before cleanup after process exit. |
| Reporting Requirements | Actual implementation delta, named retained limitations, dual-width tests and required artifact qualification. |
| Stop Conditions | Do not conflate the board A20 bit with fixed CPU pin width or relabel Intel/486 execution as full SXLV. |
| Exit Criteria | CPU ordinary contract proved by tests, actual scope/quality reviewed, applied manifests and affected artifacts verified, complete delivery pushed. |
| Original Owner Request | Replan four layers and begin the hardware T. Execute sequentially in single-session dual-role mode. |
| Similar-Issue Sweep | Fetch, data, page walks, debug inspection, reset alias and DMA at the CPU/board boundary. |
