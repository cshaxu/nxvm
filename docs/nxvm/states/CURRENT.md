# Project Status

## Current Work

## M5 T580 S61 — PC110 extended-CMOS configuration receiver design

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S61, next unused numeric S after completed S60; one-session coordinator/executor. |
| Admission And Approval | The owner’s continuing instruction to implement PC110 permits autonomous in-scope research and a bounded App/X86-NXVM receiver when evidence supports it. This consumes T580's remaining CMOS/Setup-persistence receiver. Lib, Emulator, X86 Core/Debug, external payloads, selected guest and PCIC behavior remain excluded. |
| Objective | Establish whether the existing PC110 CMOS seed path can represent the source-recorded extended CMOS configuration (`40h`–`7Fh`) needed by the real BIOS/Easy-Setup, and either identify a minimal, source-backed receiver or retain a precise asset/contract prerequisite. |
| Non-goals | No invented CMOS bytes/checksum, BIOS patch, Setup-PC interception, generic persistence service, runtime configuration editor, PC Card/media behavior, selected guest claim, raw `.cmos` commit or product publication. Do not expand a 64-byte seed merely because a larger array is convenient. |
| Reference Baseline | Clean pushed `3731ece76` after S60. Existing Machine accepts an optional 64-byte asset and applies only `0Eh`–`3Fh`; Board already decodes PC110's extended CMOS index range through `7Fh`. Open-Source-PC110 documents real persisted settings/checksum in `40h`–`7Fh`, but its local checkout does not contain the cited raw `ports_indexed.json` payload. |
| Candidate Proposal | Reconcile the existing seed ownership, Board default capacity and external source facts. If a lawful, exact external seed is locally available, design one size/validation/embedding route that consumes it without duplicating RTC ownership. Otherwise document the missing source asset and transfer the receiver without fabricated defaults. Any production contract expansion requires this packet to be revised with exact files, ABI and regression plan before editing source. |
| Files And ABI Surface | Initial admitted surface is `docs/nxvm/{states,etc/evidence,history}` and read-only source/research inspection. No production or test file is admitted until the coordinator records a concrete source/contract decision in this packet. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING,UI}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; `docs/nxvm/etc/operations/policy/source-policy.md`; T580 proposal and convergence ledger. Preserve the sole Core RTC owner, BYOB payload policy, inward dependencies and external-payload boundary. |
| Verification | Inspect actual Machine seed copy/apply and Board extended-index/default rules; compare to the named Open-Source-PC110 live-dump and PS2 records. Verify raw-asset presence, byte count, provenance and allowed location before any use. Documentation gate and actual-change review apply to this evidence-only stage. |
| Expected Markers | Evidence identifies the exact current `0Eh`–`3Fh`/50-default mismatch against `40h`–`7Fh`, whether a source payload exists, and the narrow owner/contract required for any follow-up. No source citation is inflated into a synthetic full seed. |
| Asset Needs | Read-only `O:/repos.external/Open-Source-PC110` research and the external `nxvm-assets` profile archive. A protected/raw `.cmos` remains external and requires exact provenance/hash before an embedding route may consume it. |
| Reporting Requirements | Record the source grade, present/missing assets, actual owner and precise next disposition. State that Setup persistence, guest boot, PCIC mapping and publication remain unqualified. |
| Stop Conditions | Stop source changes if the raw seed is absent, ambiguous, protected without authorized external retention, or needs a second RTC/persistence owner. Do not use documentation prose to manufacture unobserved register values. |
| Exit Criteria | Evidence yields either an admitted minimal receiver with exact source/asset/contract/regressions or a clean transfer of the blocked configuration-seed prerequisite. The workspace is clean or has a reviewable P; T580 stays open. |
| Original Owner Request | “老规矩实现PC110”, followed by “加油”; owner permits source-labelled L2 model work and autonomous in-scope S admission, but not fabricated firmware/media behavior. |
| Similar-Issue Sweep | Inspect all optional CMOS-seed consumers and extended-index Board profiles. Confirm no other App silently needs PC110 extended defaults and no outer layer owns a duplicate live RTC image. |

## Verified Baseline

Both host widths run the unchanged TFT BIOS's complete Setup loader and
ordinary input/reset transaction. ERROR and subsequent date/time frames are
byte-identical across host widths with programmed BE9E96 background.
Both reach the fifth real reset at FFFF:FFF0 after 144,110,532 instructions,
with four known exceptions and no Core fault. Normal main Setup,
configuration persistence and selected guest boot remain unqualified.

All 712 exact final-tree units per width and specialized gates pass.
Main-feature real FDC/boot regressions pass at 40.54/215.85 s x64 and
53.38/321.83 s x86; after the final preflight-only range guard, rebuilt
firmware FDC regressions pass at 43.99/63.34 s. The
[S57 record](../etc/evidence/t580-s57-f65535-planar-colour.md) distinguishes
these exact source boundaries, the mathematical normal-domain equivalence,
failed candidates, source grades and unverified domains. It does not
relabel pre-guard executable bytes as the final build.

Eight PC receiving builds and adjacent INI identities pass; their published
relinks change only PE timestamp/checksum. Four PC110 previews are rebuilt
but unpublished. The [inventory](../etc/evidence/t580-s57-artifacts.json)
records actual hashes, PE widths and embedded 0.5.0580. Lib, Emulator,
X86 Core/Debug/IBMPC, MyNES/MyRaidenII, owner INI and external ROM/font/media
are unchanged. All 90 owned scratch files/13,302,526 bytes are removed after
task-exit/hash review; stable Ninja/ccache remain.

S58 adds the source-labelled empty two-slot RB5C396 controller at the actual
3E0h/3E1h route, with no card, storage, IRQ/window or guest claim. Final-tree
unit qualification is 714/714 on each width. Existing real-firmware
FDC-reset/boot-handoff integrations pass x64 in 42.24/225.02 s and x86 in
54.59/306.85 s; they remain bounded non-regression evidence, not PCIC
consumption or product qualification. See the [S58 record](../etc/evidence/t580-s58-pcic-receiver.md)
and [task history](../history/M5-T580-pc110-qualification.md).

S59 completes the declared ATA two-device construction route.  After a fresh
Ninja relink of every unit target, the complete repository-only unit suite
passes **714/714** on both x64 and x86 (212.88 s and 73.32 s respectively).
That qualification includes the focused ATA slave construction proofs and
the existing real-firmware FDC-reset/boot-handoff non-regressions recorded in
the S59 evidence.  It does not qualify authentic SunDisk/CF inputs, BIOS
storage consumption, PCIC storage, Setup persistence, selected-guest boot,
panel behavior, desktop use, or product publication.

S60 selects the source-recorded primary SunDisk geometry through the existing
master-only Machine construction route: **123x2x32**.  Its disposable
exact-size/short-size proof and full MyPC110 labelled regression pass on x64
and x86; the unchanged real-firmware FDC-reset/boot-handoff checks pass in
45.91/231.73 seconds x64 and 53.51/318.12 seconds x86.  After fresh relinks,
the full repository-only unit suite passes **714/714** on x64 (61.36 seconds)
and x86 (59.03 seconds).  This does not qualify real SunDisk/CF media, BIOS
storage consumption, Setup persistence, PCIC storage, selected guest or a
PC110 release; no PC110 executable is published.  See the
[S60 evidence](../etc/evidence/t580-s60-pc110-master-geometry.md).

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain normal Setup/configuration persistence and post-reset progression,
per-slot SunDisk/CF storage and selected guest, PCIC/card handling, display
enable/blanking and panel/TFT/DSTN identity, live input/lifecycle, desktop and
four-artifact publication. Restored ERROR/date-time colours close only the
named standard planar/colour receiver.

Next admission must consume a coherent remaining receiver with actual source
and direct proof; do not seed successful CMOS, hook BIOS PCs, force guest
flags or introduce host display swaps. Source-labelled functional L2 is
permitted; physical-unit measurement is not a prerequisite to model work.
No remaining original domain is removed or called complete.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained status](../history/M5-T580-retained-status.md) preserves prior
packets as history, not concurrent active work.
[Queue](QUEUE.md) retains unrelated ordered candidates.
