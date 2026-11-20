# M5 T589: OmniBook 300 basic hardware closure

Coordinator accepted NXVM 862f5684f. S1–S7 were reconciled against the
admitted hardware proposal. The device implementations and qualification
are complete for this stage.

| Hardware exit condition | Evidence |
| --- | --- |
| Establish CPU contract and reset/address width | [S1](M5-T589-S1-cpu-contract.md) and [S2](M5-T589-S2-sxlv-execution.md) |
| 16-bit bus and prefetch | [S3](M5-T589-S3-sxlv-word-bus.md) |
| Management entry and separate bus space | [S4](M5-T589-S4-sxlv-management.md) |
| 65510 registers, VRAM and frame output | [S5](M5-T589-S5-ct65510-display.md) |
| Keyboard and pointing-device protocols | [S6](M5-T589-S6-ob300-input.md) |
| Device regression and board signal handoff | [S7 evidence](../etc/evidence/t589-s7-ob300-hardware.md). Both widths passed the specialized gates and 65 selected unit registrations. |

The final S7 delivery changes only NXVM repository checks and evidence.
The S5 published receiver artifacts remain the verified binaries.
Physical board decode, interrupt and reset wiring, real System Card boot,
product assembly, performance and ROM Windows/Office remain distinct
requirements. S7 describes all receiving boundaries. The S7 scratch
directory was removed after process exit and hash retention.

## Archived S7 packet

# M5 T589 S7 - Delivered: Basic hardware qualification

[S6 is closed](../history/M5-T589-S6-ob300-input.md).
[S5 display evidence](../etc/evidence/t589-s5-ct65510-display.md) records
the two stale NXVM repository timing checks found during display regression.
The [hardware qualification](../etc/evidence/t589-s7-ob300-hardware.md)
records the verified device contracts and board-wiring handoff.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S7 after S6 qualification at 15039a56f. |
| Admission And Approval | Owner authorized sequential single-session coordinator/executor admission. |
| Objective | Verify the basic CPU, video, input and PIC/PIT/DMA/RTC device contracts for the HP board receiver, and repair stale NXVM qualification checks. |
| Non-goals | No HP board wiring, product binary, original ROM boot, speculative silicon precision or CPU timing changes to appease static checks. |
| Reference Baseline | T589 S1-S6 delivered contracts, the HP service and compatibility evidence, existing dual-width cached integration inventory. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S7 and proposals/m5-ob300-architecture.md. |
| Files And ABI Surface | NXVM static checks and evidence. Change a shared device only if verification demonstrates a real missing board-facing contract, with a recorded scope revision and receiver tests. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Verify runtime chip contracts on both cached Release widths and NXVM manifest and boundary gates. Replace stale timing source probes with assertions against current dispatch and retirement behavior. Record test-backed capabilities and board signal ownership. |
| Expected Markers | Verified independent hardware contracts and a concrete HP board handoff with retained model limits. |
| Asset Needs | Existing fixture data and the read-only manual. Temporary work in build/t589-s7, up to 64 MiB; 300 seconds for focused tests or one aggregate. |
| Reporting Requirements | Record real coverage, corrected check results, tested widths and the remaining board/ROM gaps. |
| Stop Conditions | Preserve all established timing and device semantics. Distinguish proven behavior from unanswered board strap and decode questions. |
| Exit Criteria | Verified CPU/display/input and support-device contracts, corrected checks, complete board handoff, reviewed pushed deliverable and clean worktree. |
| Original Owner Request | Build the OmniBook 300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | NXVM timing inventories, retirement boundary probes, device phase/IRQ, reset and DMA ownership. |
