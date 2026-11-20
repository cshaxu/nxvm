# M5 T589 S6: OmniBook 300 keyboard and pointing device

Coordinator accepted NXVM 15039a56f. The independent review confirmed the
original driver service boundary and the existing device and reference
board tests. Both host widths passed all eight focused tests. No source
or release bytes changed. The actual HP board and ROM control paths remain
explicit receiver work. The task scratch directory was cleared after the
background jobs exited and log digests were retained.

[S6 qualification record](../etc/evidence/t589-s6-ob300-input.md).

## Archived packet

# M5 T589 S6 - Delivered: OmniBook keyboard and pointing device

[S5 is closed](../history/M5-T589-S5-ct65510-display.md).
The [S5 input handoff](../etc/evidence/t589-s5-ct65510-display.md#input-handoff)
records the HP service evidence and original System 1.0 driver behavior.
The [S6 qualification](../etc/evidence/t589-s6-ob300-input.md) records the
two-width test results and the HP board handoff.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S6 after S5 publication at 5b0a97d5b. |
| Admission And Approval | Owner authorized sequential single-session coordinator/executor admission. |
| Objective | Qualify or implement the keyboard and auxiliary pointing-device contracts needed by the OmniBook 300. |
| Non-goals | No HP port decode guesses, new host input route, original ROM full boot or unsupported hardware precision claim. |
| Reference Baseline | HP service publication 5965-0244; System 1.0 ABA mouse driver and BIOS evidence; current X86-NXVM AT controller, keyboard and mouse tests. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S6 and proposals/m5-ob300-architecture.md O07. |
| Files And ABI Surface | NXVM evidence first. Change existing X86-NXVM device owners and tests only for a demonstrated hardware-contract gap. Apply receiver obligations to actual Shared changes. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Establish the controller protocol from supplied evidence. Verify keyboard make/break, auxiliary motion/buttons, reset, command and IRQ delivery on cached x64/x86. Retain clear separate board receiver conditions. |
| Expected Markers | One keyboard and auxiliary owner using existing Machine input admission. Hardware reset and guest command evidence remains distinguishable from inferred board routing. |
| Asset Needs | Local HP service manual and read-only System 1.0 files. Temporary work in build/t589-s6 up to 32 MiB; 300 seconds for a test batch, then retain necessary hashes and remove owned temporaries. |
| Reporting Requirements | State reuse or demonstrate a concrete gap, list relevant tests and limits, distinguish the underlying controller protocol from board and driver behavior. |
| Stop Conditions | No unsupported exact controller marking, protected asset publication, or second device implementation where the current owner already supplies the contract. |
| Exit Criteria | Sourced HP driver/protocol evidence, verified independent make/break and motion/button/reset tests on both host widths, recorded board requirements and pushed reviewed delivery. |
| Original Owner Request | Build the OmniBook 300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | Existing keyboard/aux forwarding, scan translation, typematic, controller handshakes and reset. |
