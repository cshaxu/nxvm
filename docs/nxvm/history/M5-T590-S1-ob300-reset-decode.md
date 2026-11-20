# M5 T590 S1: OmniBook reset and chipset decode

Coordinator accepted NXVM 81b4a899a. Both cached widths executed the
original reset bytes through the first cold POST marker or the seeded
SMM transfer. Review checked the ROM identity, FBh enable strobe, ECh/EDh
selection, read-only SMMACT and local test limits. The original firmware's
creation of the management signature, general board routes and final ROM
decode remain later receiver work. The task-owned ROM slice and other
scratch files were removed after the probes exited and digests were kept.

[S1 qualification record](../etc/evidence/t590-s1-ob300-reset-decode.md).

## Archived packet

# M5 T590 S1 - Delivered: OmniBook 300 reset and board decode

[T589 is closed](../history/M5-T589-ob300-hardware-closure.md).
The [hardware handoff](../etc/evidence/t589-s7-ob300-hardware.md)
lists the qualified components and board-level receiver conditions.
The [S1 reset and chipset evidence](../etc/evidence/t590-s1-ob300-reset-decode.md)
records the original cold and management branches executed on both widths.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T590 S1. Next board candidate after T589 closure a50285848. |
| Admission And Approval | Owner authorized the OmniBook sequence and sequential coordinator/executor execution. |
| Objective | Derive the initial cold and management board paths and the handoff for App-owned chipset decode, RAM and clock configuration. |
| Non-goals | No invented register responses, whole original-firmware boot, production binaries, host UI, replacement firmware or protected asset publication. |
| Reference Baseline | T589 hardware closure; hash-verified System 1.0 ABA dump; HP service publication 5965-0244 and the VLSI SCAMP II register reference. |
| Candidate Proposal | proposals/m5-ob300-board.md S1; architecture O02/O04/O05/O08. |
| Files And ABI Surface | NXVM evidence and queue records. Any bounded diagnostic uses existing X86 Core execution and bus interfaces. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy and shared Execution/Documentation/Architecture/Coding. |
| Verification | Trace reachable original reset accesses beyond FBh and through the cold and management branch. Classify register purpose and distinguish exact chipset facts from OB300 assumptions. |
| Expected Markers | Explicit reset route, initial port and memory inventory, refusal boundary and candidate board ownership without an additional execution path. |
| Asset Needs | Owner-provided immutable card dump and manuals. Temporary work in build/t590-s1 up to 32 MiB with a 300-second total run limit. |
| Reporting Requirements | Record original code offsets and route evidence, known bit semantics, modeled values, missing sources and S2 acceptance signals. |
| Stop Conditions | Never claim that an unknown write or a guessed read response proves silicon behavior or full boot. |
| Exit Criteria | Reproducible HP branch trace, source-classed receiver contract, reviewed and pushed evidence, and clean worktree. |
| Original Owner Request | Build the OmniBook 300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | Original reset windows, FBh and ECh/EDh accesses, cold/SMM selection, IRQ setup, task-owned data and no whole-F mapping assumption. |
