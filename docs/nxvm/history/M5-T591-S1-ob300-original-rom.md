# M5 T591 S1: Original OmniBook BIOS receivers

The coordinator accepted and closed S1 after reviewing App commit
`8209ce97c` and the [dual-width qualification](../etc/evidence/t591-s1-ob300-original-rom.md).
All 717 unit registrations passed on each host width. The real card progressed
through E000:0003, extended CMOS, and 8042 self-test. A scratch replay
isolated the PIT checkpoint to a shared 8254 select polarity defect and
identified a separate, still unclassified protected-mode shutdown after it.
No Shared source or existing product executable was changed.
The delivery removed 79 task-owned temporary files, totaling 11,841,293 bytes,
after verifying the tasks had exited.

## Archived packet

# M5 T591 S1 - Active: OmniBook original ROM machine receiver

[T590 is closed](../history/M5-T590-S5-ob300-board-qualification.md).
The [production and scratch verification](../etc/evidence/t590-s5-ob300-board-qualification.md)
defines the real-card boundary and the next reproducible diagnostics.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T591 S1 after T590 board qualification. |
| Admission And Approval | Owner authorized ordered OmniBook implementation and single-session coordinator/executor operation. |
| Objective | Wire the original E000 option code, CMOS address and 8042 reply timing into the App-owned machine configuration. Advance through the next reproducible BIOS checks. |
| Non-goals | No protected firmware in the repository, host DOS shim, replacement ROM, other App or shared component changes without a demonstrated need. |
| Reference Baseline | T590 board qualification and immutable owner System 1.0 ABA image. |
| Candidate Proposal | proposals/m5-ob300-rom-dos.md T591 S1; architecture O02/O03/O05/O07/O08. |
| Files And ABI Surface | NXVM only. App board/machine assembly and mirrored tests/evidence. Borrow the immutable image through the existing machine lifetime. |
| Applicable Rules | NXVM README, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Stable cached x64/x86. Test E-region mapping and addressed CMOS/KBC behavior, then replay original BIOS on both widths. Full repository unit qualification for delivery. |
| Expected Markers | Original E000:0003 call proceeds and original firmware sees the correct 8042 self-test reply. Explain the next remaining diagnostic from observed behavior. |
| Asset Needs | Owner read-only System Card. Use build/t591-s1 for disposable probes and copies, at most 64 MiB and 300 seconds per run. |
| Reporting Requirements | Exact source/diff and evidence limits. Keep owned board status separate from later machine and product acceptance. |
| Stop Conditions | Report any required Shared ABI change before editing Shared. Preserve original inputs and fail explicitly for unavailable model effects. |
| Exit Criteria | Native original-ROM checkpoints after the prior failures on both widths, cached unit passes, reviewed delivery pushed and clean worktree. |
| Original Owner Request | Implement HP OmniBook 300 in ordered hardware, wiring, ROM machine and product stages. |
| Similar-Issue Sweep | F and high reset aliases, E option entry, extended CMOS index masks, cancelled/early KBC replies. |
