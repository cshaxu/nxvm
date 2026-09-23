# Project Status

## Current Work

**Active: M6 T39 S10 release-artifact correction.** The owner reports that the
deployed `0011` executable remains black; S10 must ship the repaired source as
the task-39 artifact before any gameplay conclusion can stand.

## M6 T39 S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen |
| Admission And Approval | Owner rejected T39 closure because only old `0011` binaries were deployed and explicitly directs that task-39 dual executables be placed in `assets/binary-mynes`; the owner also directs removal of `0011`. |
| Objective | Build and deploy the repaired MyNES product as `mynes_0_0_0039_x64.exe` and `mynes_0_0_0039_x86.exe` beside the existing editable `mynes.ini`. |
| Non-goals | No ROM import, no change to user `mynes.ini`, no shared-component change and no new emulator mechanism repair in this S. |
| Reference Baseline | `3588fe945`; source repairs exist but deployed `0011` artifacts predate task-39 identity. |
| Candidate Proposal | [M6 TMNT3 Start-path diagnosis and correction](../proposals/m6-tmnt3-start-path.md) |
| Files And ABI Surface | MyNES product CMake, release-manifest tool, artifact directory and MyNES state/evidence only. |
| Applicable Rules | Task revision is the four-digit task number; each runnable product task supplies optimized x64/x86 artifacts in `assets/binary-mynes`; owner-local media stays untracked. |
| Verification | Build both architectures, verify PE machine type and versioned filenames, generate the ignored build manifest, and run focused Core/Driver regressions. |
| Expected Markers | `assets/binary-mynes` contains exactly the `0039` executable pair plus the sole editable `mynes.ini`; no manifest or `0011` executable remains there. |
| Asset Needs | Existing local `mynes.ini` is retained unchanged; no ROM is read or committed. |
| Reporting Requirements | Report S10 diff add/remove/net, files and rationale, build/test outcome, pushed commit, and clickable x64/x86 paths. |
| Stop Conditions | A required toolchain cannot build a valid PE or artifact placement would overwrite the user configuration. |
| Exit Criteria | Both versioned executables are present, architecture-verified, the ignored manifest is generated, old executables/asset manifest are removed, and the result is committed and pushed. |
| Original Owner Request | Put the latest dual EXEs in assets; task 39 uses version `0_0_0039`; remove the old `0011` executables. |
| Similar-Issue Sweep | Product target, artifact copy name, manifest name/version and CURRENT artifact baseline use the same task revision. |

### S10 Brief

Correct the task-39 artifact identity and deploy both built architectures to
the one product artifact directory. Preserve the owner-managed INI verbatim.
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
- Delivery kind: `product-execution`; current target: `mynes-0-0-0039`; artifacts
  are `assets/binary-mynes/mynes_0_0_0039_x64.exe` and
  `mynes_0_0_0039_x86.exe`. The one editable `mynes.ini` is adjacent; generated
  manifests stay ignored under `build/`.
- MyNes snapshots are private versioned `MNS1` state images. App owns command/file
  policy and direct writer lifetime; Core owns image state and cartridge identity;
  Common/Lib remain neutral. ROMs remain ignored and no remote is configured.

## Recent M6 Closures

| Task | Compact result |
| --- | --- |
| T36 | Closed: SoftPC-shaped `save`/`load`, Core/Driver state stream, matching-cartridge restore, rejected malformed/missing/truncated inputs and dual-architecture integration proof are recorded in [history](../history/M6-T36-snapshot-state.md). |
| T37 | Closed in `7666ddd`: the completed App/Core quality work and the owner-selected source, test and packaged-artifact move were retained together for T38 reconciliation. |
| T38 | Product-layout automation passed; owner binary review reported the TMNT3 title-flow regression. The layout result remains retained, and the runtime defect is now T39's bounded receiver. |
| T39 | Closed in `9fd404baa`: S9 production-Driver evidence confirms sustained controlled TMNT3 gameplay; x64/x86 non-desktop automatic suites pass. |
| T35 | M5 closed in `d6da730`: six owner-local ROM roles passed both presenters/x64/x86. |

## Recent Governance

- **M6 Td S1:** Owner-approved snapshot milestone and proposal retained in [M6 snapshot state](../proposals/m6-snapshot-state.md).
