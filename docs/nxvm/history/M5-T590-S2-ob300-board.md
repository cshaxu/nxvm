# M5 T590 S2: OmniBook 300 board construction

The coordinator accepted Shared e7438fa22 and NXVM 52b0d4ca3 with
the MyNES/MyRaidenII publication pairs at 1c2179007 and 33ab8c071.
Both widths passed the App board smoke and all 717 unit registrations,
with an exact x64 follow-up after the aggregate deadline. The fourteen
published binaries passed PE-width and SHA-256 verification.

The original BIOS replay reached the PIC initializer and stopped at the
first unsupported register write for the System Card window. Complete
original-ROM POST is outside this batch. The board proposal retains
the card mapping checkpoint and the evidence lists unresolved chipset
behavior. The task's 63 temporary files (2,355,745 bytes) were removed
after all owned processes exited.

[S2 qualification record](../etc/evidence/t590-s2-ob300-board.md).

## Archived packet

# M5 T590 S2 - Delivered: OmniBook 300 board plan and chipset

[S1 is closed](../history/M5-T590-S1-ob300-reset-decode.md).
The [reset evidence](../etc/evidence/t590-s1-ob300-reset-decode.md)
defines the first cold and system-management paths for the board receiver.
The [S2 board evidence](../etc/evidence/t590-s2-ob300-board.md)
records the implemented routes and the two-width board smoke.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T590 S2 after S1 delivery at 81b4a899a. |
| Admission And Approval | Owner authorized the OmniBook sequence and sequential coordinator/executor execution. |
| Objective | Implement a flat App-owned board plan using Am386SXLV and verified base hardware, with the SCAMP-compatible decoder and its reset and clock routes. |
| Non-goals | No executable product, protected assets, existing PC model changes, invented firmware responses or second Core time axis. VGA/input wiring and System Card decode are later admitted batches. |
| Reference Baseline | T589 device closure a50285848, S1 original reset route, HP service 5965-0244 and VLSI SCAMP II Tables 40, 53, 54 and 59. |
| Candidate Proposal | proposals/m5-ob300-board.md S2; architecture O02, O04, O05, O08. |
| Files And ABI Surface | NXVM owns src/app-myob300, test/app-myob300, their registration under the root NXVM build branch, evidence and its five PC artifact pairs. Shared owns X86-NXVM Board Profile forwarding of the existing Core SMIADS read/write pair. Lib, Emulator and X86 Core have no changes. The Shared release requirement also covers the MyNES and MyRaidenII pairs in their own artifact-only Ps. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Use synthetic firmware and board tests on cached x64/x86 for reset, unclaimed ports, byte/index data decode, management separation, CPU profile, RAM, PIC/PIT/DMA, RTC and A20. Verify the full cached unit inventories and all fourteen receiver PE identities and hashes for the Shared Board Profile change. |
| Expected Markers | Single App board plan and Core time source, configuration gating from the observed firmware path, no silent acceptance of out-of-scope hardware. |
| Asset Needs | Source manuals and synthetic fixtures only. Temporary work in build/t590-s2 up to 64 MiB; 300 seconds for focused qualification runs. |
| Reporting Requirements | Record code and interfaces, hardware/model attribution, exact proof limits and display/input/card handoff. |
| Stop Conditions | Do not force the HP board through old PC quirks or shift validated device ownership. Report any proven public-contract gap before implementation. |
| Exit Criteria | Board path builds and passes scoped tests on both widths, App ownership is clear, and qualification is reviewed and pushed with a clean worktree. |
| Original Owner Request | Build the OmniBook 300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | Reset precedence, register lock and readback, SMM versus ordinary RAM, AT support-device service routes and clock ratios. |
