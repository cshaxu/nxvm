# Project Status

## Current Work

**Active: M6 T40 S1 executable-directory Base capability.**

## M6 T40 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved T40 to add a Shared Lib Base executable-directory capability and correct MyNES/NXVM adjacent-INI loading; SoftPC will adopt the shared revision separately. |
| Objective | Define, implement and test one Base API that returns the current executable directory in a caller buffer. |
| Non-goals | No Common or SoftPC change, no INI override/fallback policy, no product artifact refresh and no App adoption in S1. |
| Reference Baseline | MyNES calls `GetModuleFileNameA` in App; NXVM derives an INI path from `argv[0]`; Base has only clock/sync host capabilities. |
| Candidate Proposal | [M6 executable-directory Base capability](../proposals/m6-executable-directory-base.md) |
| Files And ABI Surface | Shared: `src/lib/base`, `src/lib/CMakeLists.txt`, `test/lib`, Lib manifest only. |
| Applicable Rules | Base depends only on Types; public API exposes a bounded copied result; platform headers stay private; one target-scoped Shared P. |
| Verification | x64/x86 Lib tests cover success and capacity rejection; Lib manifest/dependency checks pass. |
| Expected Markers | A caller receives a nonempty directory with no trailing separator, or a defined status without modifying a rejected result. |
| Asset Needs | None. |
| Reporting Requirements | Report Shared S1 source/test diff, retained owner, x64/x86 tests, P commit/push and transfers to S2/S3. |
| Stop Conditions | A portable Linux executable-directory contract cannot be defined without an unbounded or ambiguous fallback. |
| Exit Criteria | Shared API/implementations/tests pass and are pushed; MyNES/NXVM receive separate admitted P adoption steps. |
| Original Owner Request | Lib Base should expose executable location; correct NXVM and MyNES INI loading, with SoftPC later importing the same Lib. |
| Similar-Issue Sweep | All current executable-path/INI discovery references in Lib, MyNES and NXVM are inventoried; non-S1 product hits are transferred to S2/S3. |

### S1 Brief

Implement the shared owner first. The API returns only the directory of the
current executable, not an INI path; callers compose their product filename.
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
- Delivery kind: `product-execution`; current target: `mynes-0-0-0039`; artifacts
  are `assets/binary-mynes/mynes_0_0_0039_x64.exe` and
  `mynes_0_0_0039_x86.exe`. The one editable `mynes.ini` is adjacent; generated
  no manifest participates in the current delivery path.
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
