# M5 T589 S3: Am386SXLV word bus and prefetch

Coordinator accepted Shared 01d60a30e and receiver publications f2bc27e48,
0e63d8091 and 420529dc9. Review reconciled lane order, address wrapping,
single operand admission, fault publication, all fourteen PE/hash checks
and the full 716-name unit inventory on both widths. The x64 aggregate
reached its outer deadline at 699 successes. The remaining 17 succeeded
on the unchanged tree. The x86 aggregate passed in one bounded run.
The S3 scratch directory was cleared after its background jobs exited.

[Implementation evidence](../etc/evidence/t589-s3-am386sxlv-word-bus.md)
includes the bounded CPU result and the S4 firmware handoff. In order,
the x64 aggregate, x64 complement and x86 aggregate log SHA-256 digests were:

- `23cc98e5955a6af3fb8da117cd706f5129680447fe7b3257ad878d61af8720ae`
- `72bcf968d9412e9641219f8e475b758db593d0a50b8b1a22aaa5a62a941f3e5d`
- `b5b8b2805356e570c14fed9b8fb48607cb59521f88e99aaf0042c939f0f1a903`

## Archived packet

# M5 T589 S3 - Delivered: Am386SXLV 16-bit transfers and fetch

[S2 is closed](../history/M5-T589-S2-sxlv-execution.md).
[S3 qualification evidence](../etc/evidence/t589-s3-am386sxlv-word-bus.md)
records the modeled word bus, transaction checks, 716/716 unit coverage on
each host width, and the fourteen verified receiver artifacts. x64 coverage
combines the original bounded run with the exact remaining inventory.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S3. S2 accepted and closed. |
| Admission And Approval | Owner authorized basic hardware execution and sequential single-session dual-role operation on 2026-11-19. |
| Objective | Implement Am386SXLV 16-bit external memory and I/O data cycles and two-byte prefetch transactions on the existing Core CPU path. |
| Non-goals | No exact OB300 wait-state schedule, separate prefetch engine, SMM implementation, or HP board wiring. |
| Reference Baseline | 49aef45c6 and AMD 21020 Rev A, SX/DX differences and bus sections. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S3 and etc/evidence/t589-s2-am386sxlv-execution.md S3 handoff. |
| Files And ABI Surface | Shared X86 Core CPU and existing test corpus, manifests, with no planned public ABI change. NXVM documentation and required NXVM, MyNES, MyRaidenII receiver publication. |
| Applicable Rules | NXVM guide and shared execution, architecture, coding, and documentation rules. |
| Verification | Check aligned and odd memory/port byte, word and dword cycle addresses and widths, instruction prefetch, fault cancellation, retained 386/V30 behavior, and dual-width scope and required full cached qualification. |
| Expected Markers | Architectural instruction results and 24-bit address logic are preserved. External 16-bit lane widths, order and commit/cancel are observable through existing cycle contracts. |
| Asset Needs | Stable Ninja trees, synthetic fixtures, S3-owned build/t589-s3 scratch up to 64 MiB, with cleanup after reviewed process exit. Release publication budget is 40 minutes; each unit aggregate retains its 300-second test deadline. |
| Reporting Requirements | Actual delta, evidence grade, qualified artifact hashes and test outcomes. |
| Stop Conditions | Do not classify nominal transfer timing as verified OB300 physical pacing. Do not publish a second clock or retry a partially committed device write. |
| Exit Criteria | Required cycles and prefetch verified in both widths, regressions and receiver publication checked, reviewed complete commits pushed. |
| Original Owner Request | Implement MyOB300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | Operand width versus cycle width, port validation before partial commits, reset routing and odd/page-crossing fetch behavior. |
