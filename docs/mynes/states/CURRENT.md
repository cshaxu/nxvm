# Project Status

## Current Work

**Active: M6 T41 S1 Shared Types and MyNES vocabulary cleanup.**

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved M6 T41 on 2026-09-23: first fill current Shared Types gaps and clean MyNES; then adopt existing Types in NXVM without Lib changes; finally submit the remaining NXVM-facade replacement design for owner review before implementation. |
| Objective | Add the one demonstrated shared scalar gap, then remove direct C-runtime/type vocabulary from the admitted MyNES production and test surfaces. |
| Non-goals | No NXVM source/CMake/test change in S1, no SoftPC change, no new assertion facade, no replacement of native Windows integration probes, and no `type.h` retirement implementation. |
| Reference Baseline | `src/lib/types/types_interface.h` lacks `lib_i8`; MyNES production has direct `<string.h>`, `memcmp`, `memcpy`, and `int8_t`; MyNES tests use direct memory/string/allocation operations. |
| Candidate Proposal | [M6 T41 shared type vocabulary convergence](../proposals/m6-t41-type-vocabulary-convergence.md) |
| Files And ABI Surface | Shared: `src/lib/types`, `test/lib`, Lib manifest. MyNES: `src/app-mynes`, `test/app-mynes`, MyNES task evidence/status. |
| Applicable Rules | Shared Types remains header-only and neutral; App/Core/tests use `lib_*` vocabulary; no product imports a platform SDK; replacement removes obsolete direct calls and includes in the changed mechanism. |
| Verification | x64/x86 Lib and MyNES focused/full repository tests pass; static sweeps find no direct C memory/string/allocation call or raw fixed-width type in MyNES production/test scope, except documented test assertions and native Windows presenter probes. |
| Expected Markers | `lib_i8` is available to both products; MyNES uses Lib memory/type vocabulary exclusively for its admitted operations. |
| Asset Needs | None. |
| Reporting Requirements | Report Shared and MyNES source/test add/remove/net counts separately, every retained test-only C/platform boundary, x64/x86 checks, P commits/pushes, and transfer to NXVM S2. |
| Stop Conditions | A MyNES direct runtime operation lacks a neutral Lib equivalent and adding it would duplicate Storage/Base/Common ownership. |
| Exit Criteria | Shared scalar addition and MyNES cleanup pass dual-architecture verification, direct-call sweep is clean within stated exceptions, evidence is committed/pushed, and S2 has an exact remaining NXVM ledger. |
| Original Owner Request | Audit and eliminate direct C standard-library/type/platform use: first Shared Types plus MyNES, then existing Types adoption in NXVM, then owner-reviewed complete retirement of NXVM `type.h`/`type.c`. |
| Similar-Issue Sweep | Inventory every direct C header/function/raw scalar across MyNES source/test and classify it as migrated, test-only retained, platform-probe retained, or transferred to a later S. |

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
