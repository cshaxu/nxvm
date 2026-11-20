# M5 T590 S3: OmniBook display and input wiring

The coordinator accepted NXVM c6859eae8 after both widths passed the App
board smoke and all 717 unit registrations. The App owns one 65510
instance for port, memory and copied display paths. The existing 8042
wiring delivered keyboard and auxiliary bytes through guest IRQ1/12
handlers. X86's remaining four registrations were checked individually
after the aggregate deadline. The isolated Types rerun used its
original budget.

The scratch card-route replay reached real code on the proposed second
card page before the 3Ah write. It is evidence for the next phase,
not a shipped board decode. All 32 task-owned files (2,170,066 bytes)
were removed after the workers exited. No product binaries were changed.

[S3 verification record](../etc/evidence/t590-s3-ob300-display-input.md).

## Archived packet

# M5 T590 S3 - Delivered: OmniBook 300 display and input wiring

[S2 is closed](../history/M5-T590-S2-ob300-board.md). The
[board verification](../etc/evidence/t590-s2-ob300-board.md#display-and-input-handoff)
records existing routes and the App ownership boundary.
[S3 board evidence](../etc/evidence/t590-s3-ob300-display-input.md)
records the App display and input paths.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T590 S3 after S2 delivery at 52b0d4ca3. |
| Admission And Approval | Owner authorized autonomous ordered OmniBook implementation and single-session coordinator/executor operation. |
| Objective | Bind the board's C&T 65510 legacy VGA, display snapshot and 8042 keyboard/auxiliary paths. Verify the HP-relevant register, memory and interrupt connections. |
| Non-goals | No Shared component changes, replacement BIOS, generic display path, host UI or System Card implementation. No production EXE before machine/product assembly. |
| Reference Baseline | T589 S5/S6 device qualification, S2 App board plan, HP 5965-0244 Appendix A/B and C&T 65510 rev 0.7. |
| Candidate Proposal | proposals/m5-ob300-board.md S3; architecture O06 and O07. |
| Files And ABI Surface | NXVM only. src/app-myob300 and mirrored unit tests, with product-local evidence. Use the existing Core route, 65510 and Board/8042 interfaces. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Cached x64/x86 board tests for paired indexed I/O, aperture visibility and decline behavior, copied display data, IRQ1/12 delivery, keyboard and auxiliary bytes. Run the full unit inventories before delivery. |
| Expected Markers | One chip instance for VGA port, memory and capture routes. One Core clock. One 8042 keyboard/auxiliary owner through existing board wiring. |
| Asset Needs | Local manuals and original ROM as read-only evidence. Synthetic fixtures for tests. NXVM owns build/t590-s3, 64 MiB maximum and 300 seconds per qualification run. Apply safe cleanup upon task exit. |
| Reporting Requirements | Record file changes, exact register/memory/IRQ evidence, model fidelity, unit outcomes and routes not yet exercised by original firmware. |
| Stop Conditions | Report a demonstrated Shared interface gap before implementing it. Preserve existing model paths and external owner assets. |
| Exit Criteria | App-owned VGA and 8042 board paths pass on both widths, existing unit regressions pass, scope and evidence reviewed, delivery pushed and worktree clean. |
| Original Owner Request | Implement the OmniBook 300 from basic hardware through board wiring, machine and product qualification. |
| Similar-Issue Sweep | Paired byte lanes, aperture selection, display reset and ownership, keyboard/AUX acknowledgements and interrupt routing. |
