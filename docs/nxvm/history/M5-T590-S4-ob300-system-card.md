# M5 T590 S4: OmniBook System Card window decode

Coordinator accepted NXVM c5b773b80. Both widths passed 717 unit
registrations and the unchanged original card entered its banked code
at F000:51AC through the production board. The App owns both immutable
card spaces and guest window selection. Synthetic attribute access
proves the modeled distinction, while physical attribute/CIS and full
POST remain unverified. No Shared package or recipient EXE changed.

The scratch matrix and refresh runs isolated the next board contracts:
the protected-mode startup RAM sizing needs a valid response beyond
installed DRAM, and the earlier refresh diagnostic needs port 61h
REFDET on the existing guest time axis. These probes are not production
boot proof. All 53 task-owned files (9,850,716 bytes) were removed after
verification and any protected scratch copies were removed by their
individual runners.

[S4 verification record](../etc/evidence/t590-s4-ob300-system-card.md).

## Archived packet

# M5 T590 S4 - Delivered: OmniBook System Card window decode

[S3 is closed](../history/M5-T590-S3-ob300-display-input.md).
The [card-route handoff](../etc/evidence/t590-s3-ob300-display-input.md#system-card-handoff)
contains the candidate decoder evidence from original execution.
[S4 card evidence](../etc/evidence/t590-s4-ob300-system-card.md) records the
production board replay and the synthetic route checks.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T590 S4 after S3 delivery at c6859eae8. |
| Admission And Approval | Owner authorized ordered OmniBook implementation and single-session coordinator/executor operation. |
| Objective | Implement App-owned immutable System Card reset, guest window selection, common and attribute memory separation, and prove both decoded window reads and instruction fetch. |
| Non-goals | Reuse existing Core provider and timing. No generic PCMCIA framework, host DOS loader, full POST claim, firmware substitution or firmware payload distribution. |
| Reference Baseline | S2 and S3 original BIOS traces, the owner 1.0 ABA common dump and HP 5965-0244 memory map, VL82C316 register table and original descriptor evidence. |
| Candidate Proposal | proposals/m5-ob300-board.md S4; architecture O02/O03/O04. |
| Files And ABI Surface | NXVM only, under src/app-myob300, mirrored tests and NXVM evidence. Owner System Card inputs stay external. No public package changes or existing artifact refresh for this App board batch. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Cached x64/x86 with synthetic immutable inputs. Verify reset window, correct pages, word spans, disabled routing and common versus attribute. Compare original BIOS checkpoints on both widths using read-only owner media. |
| Expected Markers | F000:7FF0 discovery, observed page selection, read and fetch of the selected real card span, and truthful reporting of the next unsupported firmware path. |
| Asset Needs | NXVM owns build/t590-s4 for synthetic fixtures and disposable external input copies. Limit 64 MiB total and 300 seconds per qualification run. Apply safe cleanup after workers exit. |
| Reporting Requirements | Record exact register facts, explicit L1 assumptions, routes and declined cases, actual test outcomes and remaining POST work. |
| Stop Conditions | Report demonstrated Shared interface gaps before implementation. Keep unsupported board behavior visible. |
| Exit Criteria | App-owned System Card windows verified on both host widths, original checkpoint recorded, required regressions passed, scope reviewed, delivery pushed, worktree clean. |
| Original Owner Request | Implement the OmniBook 300 from basic hardware through board wiring, machine and product qualification. |
| Similar-Issue Sweep | Stale instruction fetch after bank writes, ROM alias bounds, priority if two windows select the same span, attribute/common separation and absent or read-only data. |
