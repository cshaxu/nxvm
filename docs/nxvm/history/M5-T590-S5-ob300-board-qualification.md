# M5 T590 S5: OmniBook board qualification and T590 closure

The coordinator accepted NXVM 2a7df600f. Both host widths passed all
717 unit registrations and the original card completed the board-level
refresh, memory probes and startup latches. The tested board scope does
not supply the next E-region ROM or writable media. The original guest
trace and separately labeled ROM/CMOS/8042 experiments identify the
first machine-level receivers. Protected inputs remain external.

S1 through S5 are complete at the board boundary. T591 inherits
original-ROM machine construction and the interactive DOS requirement.
All 82 task-owned temporary files (11,633,965 bytes) were removed
after the workers exited.

[S5 verification record](../etc/evidence/t590-s5-ob300-board-qualification.md).

## Archived packet

# M5 T590 S5 - Delivered: OmniBook board qualification

[S4 is closed](../history/M5-T590-S4-ob300-system-card.md).
The [card and refresh handoff](../etc/evidence/t590-s4-ob300-system-card.md)
contains the production checkpoint and the separately identified board gaps.
[S5 qualification evidence](../etc/evidence/t590-s5-ob300-board-qualification.md)
tracks the active board checks.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T590 S5 after S4 delivery at c5b773b80. |
| Admission And Approval | Owner authorized ordered OmniBook implementation and single-session coordinator/executor operation. |
| Objective | Qualify the combined board through the original chipset and memory probes. Reconcile matrix idle, guest-time refresh, installed RAM and unpopulated bus routes against the original 1.0 ABA path. |
| Non-goals | No generic device framework, second guest clock, replacement firmware, complete DOS boot assertion or protected-media distribution. |
| Reference Baseline | S1 through S4 accepted board results, HP 5965-0244, VL82C316 tables 10/50/64 and the identified original BIOS traces. |
| Candidate Proposal | proposals/m5-ob300-board.md S5 and the O02–O08 board obligations. |
| Files And ABI Surface | NXVM only under src/app-myob300, its tests and evidence. Existing Core/Board hardware APIs are reused. No Shared package or existing executable changes are expected. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Cached x64/x86 tests for actual refreshed Port B reads, matrix state, startup chipset semantics, and RAM visibility/bus behavior. Run the full unit inventory and compare the original-board boot paths and stop markers. |
| Expected Markers | Original guest progresses through refresh detection and installed-RAM sizing using real board routing. Identify unsupported effects precisely and assign the allowed remainder to machine composition evidence. |
| Asset Needs | NXVM owns build/t590-s5. Limit 64 MiB and 300 seconds per qualification run. External inputs are read only and any scratch copy is disposable. |
| Reporting Requirements | Record exact register sources, derived model levels, App routing choices, both unit outcomes and reproducible real-ROM checkpoint. |
| Stop Conditions | Report demonstrated Shared contract gaps before editing Shared. Never assert fully booted DOS from a diagnostic checkpoint. |
| Exit Criteria | Board handoff has verified dual-width unit and owner-media integration evidence, documented scope and remaining machine-level wiring, pushed delivery and clean worktree. |
| Original Owner Request | Implement the OmniBook 300 from basic hardware through board wiring, machine and product qualification. |
| Similar-Issue Sweep | Guest-time cadence before PIT programming, sources of A20, matrix versus 8042, real RAM probes and card/ROM priority under the high alias. |
