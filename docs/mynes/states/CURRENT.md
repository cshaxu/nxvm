# Project Status

## Current Work

**Active: M6 T39 S9 owner-reopen investigation, repair, and controlled-play
verification.** S8 restores Start-driven RGB change; this reopened receiver
must verify the full controlled-input path and repair any newly demonstrated
MyNES mechanism before a T39 closure is reconsidered.

## M6 T39 S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen |
| Admission And Approval | Owner reopened the immediately preceding T39 closure and authorized automatic probe-and-repair cycles to restore TMNT3 normal play; the owner expressly requires an S task that executes the investigation, repair and verification loop. S8 (`7ce7280de`) restores handler execution and Start-driven RGB change. |
| Objective | Use ignored production Driver input to establish controlled Start, D-pad and action behavior through sustained non-identical RGB frames while CPU stack/IRQ state remains healthy; when evidence finds a causal MyNES defect, repair it and repeat the same probe before closing. |
| Non-goals | No ROM import, shared change, gameplay script corpus, universal-game claim, manual Computer Use, or release-artifact refresh unless a repaired runnable path requires the governed artifact cut. |
| Reference Baseline | `7ce7280de`; direct Start changes published RGB after resolving IRQ re-entry, but it does not yet prove normal controlled play. |
| Candidate Proposal | [M6 TMNT3 Start-path diagnosis and correction](../proposals/m6-tmnt3-start-path.md) |
| Files And ABI Surface | Ignored build probes, MyNES App/Core and their owned tests. S9 may correct a test's declared desktop dependency when that dependency blocks the headless two-architecture automatic gate; documents/evidence record each bounded loop. |
| Applicable Rules | Driver owns input delivery, Core owns controller/CPU/PPU, owner-local media remains untracked. |
| Verification | Finite production-Driver baseline/Start/control trajectories compare RGB frames and rendered images at several post-input checkpoints; assert frame progress, no trap, no repeated IRQ-entry stack state and no stuck cartridge IRQ. For any repair, run its focused owned regression and the two-architecture complete non-desktop automatic suite; native desktop tests remain separately labeled rather than being mistaken for headless gates. Rebuild artifacts only if runnable product code changes, then repeat the production probe. |
| Expected Markers | Input-dependent RGB differences persist or evolve after Start and held movement/action; sampled rendered images represent an active game state rather than a static blank frame. |
| Asset Needs | Existing owner-local ROM only via ignored environment variable; no ROM facts are committed. |
| Reporting Requirements | Derived checkpoints, visual/CPU disposition, retained scope and any required next receiver. |
| Stop Conditions | No stable semantic checkpoint, protected-ROM-only claim, or a newly isolated implementation defect. |
| Exit Criteria | Direct automated evidence supports sustained controlled visual progression with no selected defect, or every newly selected mechanism is repaired, regression-covered and reprobed; an exact remaining mechanism is transferred only if its owner lies outside MyNES. |
| Original Owner Request | Repair TMNT3 normal play without manual verification. |
| Similar-Issue Sweep | Start, D-pad, A/B, controller latch, frame RGB, stack, IRQ and trap state. |

### S9 Brief

Run an ignored production-Driver controlled-input probe. Capture bounded
baseline, Start, and held movement/action RGB/image checkpoints plus CPU, stack,
IRQ and trap markers. If a checkpoint demonstrates an owned causal defect,
perform one smallest-viable repair and owned regression, rebuild both
architectures, and repeat the probe. Continue this investigate-repair-verify
cycle until the exit criterion is met or a non-MyNES owner is demonstrated.

## M6 T39 Progress

| S | Result |
| --- | --- |
| S1 | Accepted: Start delivery reaches and is consumed by the owner-local guest; the project-owned fixture covers A and Start. [Evidence](../etc/evidence/m6-t39-s1-start-path.md). |
| S2 | Reopened finding: its narrow A12 publication regression passes, but the later synchronized frame probe exposes a black-frame IRQ/PPU initialization gap outside that regression. |
| S3 | Accepted diagnosis: the bounded pre-S2 comparison does not select a reversion; an ignored A/B run selects the Cartridge MMC3 counter/reload/A12 contract as the next repair receiver. [Evidence](../etc/evidence/m6-t39-s3-irq-ppu-diagnosis.md). |
| S4 | Accepted repair: `$C001` clears the live MMC3 count and reloads only on the next qualified edge; owned x64/x86 regressions pass, while the synchronized visual probe remains black. [Evidence](../etc/evidence/m6-t39-s4-mmc3-reload.md). |
| S5 | Accepted diagnosis: the selected rendering layout yields zero qualified edges because unused sprite slots skip dummy pattern fetches; S6 receives the narrow PPU repair. [Evidence](../etc/evidence/m6-t39-s5-ppu-a12-schedule.md). |
| S6 | Accepted repair: dummy sprite reads plus raw A12 short-low retention restore one qualified edge on each observed visible scanline; owned regressions pass but the Start frame remains black. [Evidence](../etc/evidence/m6-t39-s6-empty-sprite-fetch.md). |
| S7 | Accepted diagnosis: accepted IRQs leave `irq_poll_i` stale, causing re-entry before handler execution and stack overflow; S8 receives the CPU repair. [Evidence](../etc/evidence/m6-t39-s7-irq-poll-diagnosis.md). |
| S8 | Accepted repair: accepted interrupts mask the poll state, avoiding handler re-entry; x64/x86 regression passes and Start now changes published RGB. [Evidence](../etc/evidence/m6-t39-s8-irq-poll.md). |

## Current Technical Baseline

- Product: MyNes; MIT. M0--M5 and M6 T36 are closed.
- Delivery kind: `product-execution`; current target: `mynes-0-1-0011`; artifacts
  are `assets/binary-mynes/mynes_0_1_0011_x64.exe` and
  `mynes_0_1_0011_x86.exe`.
- MyNes snapshots are private versioned `MNS1` state images. App owns command/file
  policy and direct writer lifetime; Core owns image state and cartridge identity;
  Common/Lib remain neutral. ROMs remain ignored and no remote is configured.

## Recent M6 Closures

| Task | Compact result |
| --- | --- |
| T36 | Closed: SoftPC-shaped `save`/`load`, Core/Driver state stream, matching-cartridge restore, rejected malformed/missing/truncated inputs and dual-architecture integration proof are recorded in [history](../history/M6-T36-snapshot-state.md). |
| T37 | Closed in `7666ddd`: the completed App/Core quality work and the owner-selected source, test and packaged-artifact move were retained together for T38 reconciliation. |
| T38 | Product-layout automation passed; owner binary review reported the TMNT3 title-flow regression. The layout result remains retained, and the runtime defect is now T39's bounded receiver. |
| T39 S9 | Owner-reopened: the original closure was insufficient to prove normal controlled play. S9 owns the bounded investigate-repair-verify loop before T39 can close again. |
| T35 | M5 closed in `d6da730`: six owner-local ROM roles passed both presenters/x64/x86. |

## Recent Governance

- **M6 Td S1:** Owner-approved snapshot milestone and proposal retained in [M6 snapshot state](../proposals/m6-snapshot-state.md).
