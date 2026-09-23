# Project Status

## Current Work

**Active: M6 T40 S2 MyNES adjacent-INI adoption.**

## M6 T40 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T40 to add a Shared Lib Base executable-directory capability and correct MyNES/NXVM adjacent-INI loading; Shared S1 is complete in `4913410fb`; SoftPC will adopt the shared revision separately. |
| Objective | Replace MyNES App's direct Win32 executable-path lookup with the S1 Base contract while preserving adjacent `mynes.ini` behavior. |
| Non-goals | No Common or SoftPC change, no INI override/fallback policy and no NXVM adoption. This App startup change publishes the required `0_0_0040` MyNES x64/x86 pair. |
| Reference Baseline | `4913410fb`; Base owns actual executable-directory discovery, while MyNES still calls `GetModuleFileNameA`. |
| Candidate Proposal | [M6 executable-directory Base capability](../proposals/m6-executable-directory-base.md) |
| Files And ABI Surface | MyNES: `src/app-mynes/product/config.c`, App CMake, task evidence and the `0_0_0040` x64/x86 executable pair. |
| Applicable Rules | App depends directly on Base for its host query; no Windows SDK include remains in the App config path; MyNES owns appending its INI filename. |
| Verification | x64/x86 App config and Base process regressions pass; static sweep finds no MyNES `GetModuleFileName`/`windows.h` startup lookup; both `0_0_0040` artifacts are valid PE executables. |
| Expected Markers | MyNES constructs `mynes.ini` from the Base directory and rejects insufficient buffer capacity without fallback to the working directory. |
| Asset Needs | None. |
| Reporting Requirements | Report MyNES S2 source/test diff, removed direct host dependency, x64/x86 tests, P commit/push and transfer to NXVM S3. |
| Stop Conditions | The Base directory cannot safely append the fixed INI name within MyNES's declared path capacity. |
| Exit Criteria | MyNES consumes Base, direct Win32 lookup is removed, focused dual-architecture tests pass, the current dual artifacts are published and the MyNES P is pushed. |
| Original Owner Request | Lib Base should expose executable location; correct NXVM and MyNES INI loading, with SoftPC later importing the same Lib. |
| Similar-Issue Sweep | MyNES startup/config source and tests are inspected for direct module-path/working-directory fallback; NXVM remains the named S3 receiver. |

### S2 Brief

MyNES consumes the shared owner. It appends only its own fixed filename and
retains no direct platform lookup.

## M6 T40 Progress

| S | Result |
| --- | --- |
| S1 | Shared Base contract, Win32/Linux implementations and bounded-buffer regression passed in `4913410fb`. |
| S2 | MyNES now composes its adjacent INI from Base; x64/x86 focused regressions and valid `0_0_0040` artifact headers passed. [Evidence](../etc/evidence/m6-t40-s2-mynes-adoption.md). |

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
- Delivery kind: `product-execution`; T40 S2 publishes `mynes-0-0-0040`; its
  artifacts are `assets/binary-mynes/mynes_0_0_0040_x64.exe` and
  `mynes_0_0_0040_x86.exe`. The one editable `mynes.ini` is adjacent; no
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
| T35 | M5 closed in `d6da730`: six owner-local ROM roles passed both presenters/x64/x86. |

## Recent Governance

- **M6 Td S1:** Owner-approved snapshot milestone and proposal retained in [M6 snapshot state](../proposals/m6-snapshot-state.md).
