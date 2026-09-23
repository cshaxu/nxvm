# Project Status

## Current Work

**No active implementation packet.** M6 T41 remains open with S1 accepted;
the next admitted subtask is NXVM-only S2.

## M6 T41 Progress

| S | Result |
| --- | --- |
| S1 | Accepted: Shared `343edeb64` and `74a069246` add the required neutral Types vocabulary and declaration ownership; MyNES `3d81d2277` removes its admitted direct vocabulary, passes x64/x86 Lib and 53/53 MyNES suites, and publishes the verified `0_0_0041` pair. [Evidence](../etc/evidence/m6-t41-s1-types-mynes-cleanup.md) |

## M6 T40 Progress

| S | Result |
| --- | --- |
| S1 | Shared Base contract, Win32/Linux implementations and bounded-buffer regression passed in `4913410fb`. |
| S2 | MyNES now composes its adjacent INI from Base; x64/x86 focused regressions and valid `0_0_0040` artifact headers passed. [Evidence](../etc/evidence/m6-t40-s2-mynes-adoption.md). |
| S3 | NXVM now composes adjacent `NXVM.ini` from Base with no `argv[0]`/working-directory fallback; x64/x86 focused regressions and current product links passed in `fb461ee21`. [Evidence](../../nxvm/etc/evidence/t40-s3-adjacent-ini-adoption.md). |

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
| S9 | Reopened automatic acceptance: production Driver input reaches a sustained nonblank game scene with healthy CPU/IRQ markers; non-desktop x64/x86 automatic suites pass. [Evidence](../etc/evidence/m6-t39-s9-controlled-play.md). |
| S10 | Accepted delivery correction: task-39 artifacts are the `0_0_0039` x64/x86 pair beside the sole editable INI; obsolete `0011` outputs and MyNES manifest flow are retired. Owner RDP acceptance confirms TMNT3 gameplay. |

## Current Technical Baseline

- Product: MyNes; MIT. M0--M5 and M6 T36 are closed.
- Delivery kind: `product-execution`; T41 S1 publishes `mynes-0-0-0041`; its
  artifacts are `assets/binary-mynes/mynes_0_0_0041_x64.exe` and
  `mynes_0_0_0041_x86.exe`. The one editable `mynes.ini` is adjacent; no
  generated manifest participates in the delivery path.
- MyNes snapshots are private versioned `MNS1` state images. App owns command/file
  policy and direct writer lifetime; Core owns image state and cartridge identity;
  Common/Lib remain neutral. ROMs remain ignored and no remote is configured.

## Recent M6 Closures

| Task | Compact result |
| --- | --- |
| T36 | Closed: SoftPC-shaped `save`/`load`, Core/Driver state stream, matching-cartridge restore, rejected malformed/missing/truncated inputs and dual-architecture integration proof are recorded in [history](../history/M6-T36-snapshot-state.md). |
| T37 | Closed in `7666ddd`: the completed App/Core quality work and the owner-selected source, test and packaged-artifact move were retained together for T38 reconciliation. |
| T38 | Product-layout automation passed; owner binary review reported the TMNT3 title-flow regression. The layout result remains retained, and the runtime defect is now T39's bounded receiver. |
| T39 | Closed in `9093f4014` plus this closure record: Core/PPU/MMC3/IRQ repairs, automated controlled-play evidence, task-39 dual artifacts, NXVM-aligned delivery and owner RDP acceptance establish the requested TMNT3 result. |
| T40 | Closed across Shared `4913410fb`, MyNES `246e45a6c`, and NXVM `fb461ee21`: one Lib Base executable-directory owner now feeds product-owned adjacent INI composition. [History](../history/M6-T40-executable-directory-base.md). |
| T35 | M5 closed in `d6da730`: six owner-local ROM roles passed both presenters/x64/x86. |

## Recent Governance

- **M6 Td S1:** Owner-approved snapshot milestone and proposal retained in [M6 snapshot state](../proposals/m6-snapshot-state.md).
