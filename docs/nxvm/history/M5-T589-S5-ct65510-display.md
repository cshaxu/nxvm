# M5 T589 S5: C&T 65510 display hardware

Coordinator accepted Shared 61118d3ee and receiver publications 09bb79784,
784ad55b9 and 5b0a97d5b. Actual review verified one C&T VRAM and latch
implementation, variant identity and retained 65535 semantics, packed and
planar frames, text start address and panel off behavior. Both current
cached unit inventories passed 716/716. The MyPC110 real-firmware reset
and two boot routes passed on both host widths before the 65510-only
packed scanout addition. All fourteen published artifacts match the
publisher hashes and PE widths.

The source basis, L2 grayscale and cursor limits, readback uncertainty,
existing NXVM static-gate drift, and input handoff are recorded in the
[S5 evidence](../etc/evidence/t589-s5-ct65510-display.md). Physical LCD
compensation and the HP board's actual decode still need guest validation.
The S5 scratch logs and scripts were removed after qualification and hash
retention.

## Archived packet

# M5 T589 S5 - Delivered: C&T 65510 display hardware

[S4 is closed](../history/M5-T589-S4-sxlv-management.md).
The [display handoff](../etc/evidence/t589-s4-am386sxlv-management.md#display-handoff)
records the 65510 option ROM and compared VGA paths.
The [S5 display evidence](../etc/evidence/t589-s5-ct65510-display.md)
records the device contract, 716 passing unit tests on each host width,
MyPC110 integration and the fourteen qualified receiver artifacts.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S5. S4 accepted and closed. |
| Admission And Approval | Owner authorized basic hardware work and sequential single-session dual-role admission on 2026-11-19. |
| Objective | Establish the OB300 display identity and implement the C&T 65510 register, VRAM and frame contract for later HP board and ROM assembly. |
| Non-goals | No motherboard address decode, complete firmware boot, second host renderer, invented panel timing or 65535/65536 relabeling. |
| Reference Baseline | Shared 4d9a248a3 and receiver baseline 1cf01a0ab; C&T 65510 preliminary data sheet rev 0.7 and HP 5965-0244; verified System ROM 1.0 option image. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S5 and proposals/m5-ob300-architecture.md O06. |
| Files And ABI Surface | X86-NXVM display chip code, tests/build/manifests. Share proven standard VGA mechanics with the existing C&T receiver while keeping variant-specific extensions. Limit any receiving MyPC110 changes to wiring needed by the common interface. Required receiving product releases and NXVM evidence. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Check variant identity, indexed register ownership and reset, standard latches/write modes, extended single/dual VRAM mapping and address boundaries, font/text and graphics capture, panel blanking and 640x480 grayscale output as supported. Cached dual-width chip and MyPC110 regressions, full Shared unit and receiving release requirements. |
| Expected Markers | One standard VGA backing and rendering owner, separate 65510 and 65535 extension semantics, evidence-classed 65510 register and frame output. |
| Asset Needs | Hash-verified readonly System ROM option window, C&T and HP manuals, synthetic tests. S5 owns build/t589-s5 up to 64 MiB, cleaned after process exit. Publication allows 40 minutes; each unit aggregate retains the 300-second deadline. |
| Reporting Requirements | Evidence supporting 65510 versus the 65536 lead, actual diff, preserved MyPC110 behavior, artifact hashes and bounded remaining panel/board work. |
| Stop Conditions | Do not silently substitute an unrelated VGA chip, retain a duplicate VGA backing, treat undocumented board straps as exact, or publish raw option ROM bytes. |
| Exit Criteria | Correct documented/firmware-observed C&T behavior verified independently on both host widths, retained PC110 paths tested, and required receiving delivery reviewed and pushed. |
| Original Owner Request | Implement MyOB300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | VGA storage routes, latches, bank crossing, text font/CRTC ownership, presentation and obsolete duplicate paths. |
