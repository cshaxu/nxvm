# M5 T580 S71 — Active: post-Setup power and boot continuation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S71. |
| Admission And Approval | Owner approved autonomous PC110 implementation and automatic S sequencing; S70 accepted in `33435d7ac`. |
| Objective | Follow the unchanged TFT firmware after Setup saves CMOS and resets, determine why it enters the second power wait, and continue the actual user power and guest boot path to the next semantic boundary. Correct the newly discovered unoptimized x86 Release configuration and requalify its affected artifacts and receivers. |
| Non-goals | No automatic product key injection, firmware interception or patched ROM, invented CMOS contents, Lib/Emulator changes, false boot or release claim. |
| Reference Baseline | S69 real Setup/reset and S70 PIC evidence; real suspend/button/RSM receiver evidence from S53/S54 and current owner-managed TFT firmware and CMOS. |
| Candidate Proposal | First classify the second wait from actual CPU, suspend-refresh and firmware state. If it is a real suspend, extend the existing integration diagnostic to deliver an explicit second physical F12 make/break only after that state has been observed; verify the existing receiver and actual firmware restoration before drawing any conclusion. Repair an actual receiver only if the observed path and source justify it. |
| Files And ABI Surface | Existing MyPC110 diagnostic, NXVM task records and the actual App or X86-NXVM receiver only if evidence requires; NXVM build qualification and four x86 PC artifacts for the Release correction. No new public API or shared Lib/Emulator modification without separate review. |
| Applicable Rules | NXVM Task Reading Set, source/research, architecture, coding and execution rules. Preserve owner INI and media. |
| Verification | Use bounded, hidden background work and stable Ninja/ccache trees. S71 owns `build/t580-s71`: x64/x86 replay has respective 1,800/3,600-second wall limits, a 300-second no-progress limit and a 64 MiB output limit; stop and wait for the owned process on excess. Verify real x86 Release compile commands use optimization, requalify its affected unit, firmware and product/profile receivers, and rebuild the four x86 PC artifacts. Record source-labelled checkpoints, actual inputs and failures. |
| Expected Markers | The real reason for the second wait and the next genuine firmware/boot outcome, or a specific unqualified receiver with evidence; no forced program-counter transition. |
| Asset Needs | Existing owner-managed TFT firmware, CMOS and the reviewed Setup comparison oracle; never commit protected firmware, ROM, media or oracle. |
| Reporting Requirements | Record actual source/test delta, timing, commands, exit statuses, remaining DSTN/panel/color, persistence, media and desktop boundaries. |
| Stop Conditions | Do not infer successful restoration from a sent key; do not mutate ROM/CMOS to bypass a firmware error. |
| Exit Criteria | Dispose this bounded post-Setup power/boot batch with a real verified receiver or an explicitly located remaining issue; requalify the x86 optimized Release artifacts and receivers, and preserve the complete T580 qualification ledger. |
| Original Owner Request | Complete PC110 implementation through real dual-model product qualification; use other emulators as L2 where appropriate. |
| Similar-Issue Sweep | Distinguish cold boot, first resume and post-Setup reset; do not regress the other PC models. |

# M5 T580 — Open after S70

S70 is accepted: Shared `3d692e367` corrects the 8259 input lifetime and
NXVM `3a1c703a0` records dual-width receiver evidence and eight rebuilt PC
artifacts. X86-NXVM units pass 233/233 per width, both real-firmware FDC and
boot-handoff checkpoints pass per width, and all eight PC profile identity
and boot receivers pass under their final measured budgets. Earlier timeouts
and the old-PIC control remain documented, not relabelled as passes.

The unchanged TFT BIOS passed the previously observed `102` branch in the
x64 post-Setup replay but later reached another power wait. The x86 replay of
that particular post-Setup path, configuration persistence across processes,
main Setup, actual guest/media boot, PCIC, panel identity and color, DSTN,
desktop behavior and PC110 product publication remain unqualified. T580
remains open. See [S70 evidence](../etc/evidence/t580-s70-reset-interrupt-investigation.md).

## Verified Baseline

S65 is accepted at `aba6329a8`. It traced the selected TFT BIOS through its
ordinary recovery, POST and `INT 19h` route and found no F65535 XR index/data
write. The bounded C000 replay also found none. The existing standard VGA
palette route remains the sole evidenced colour owner; the reported blue frame
does not authorize an RGB/BGR or panel-table patch. See
[S65 evidence](../etc/evidence/t580-s65-f65535-panel-table-audit.md).

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

## S64 acceptance

Implementation `40be5c4f2` is accepted after actual-change review. The
optional reset byte stays in the existing board-to-KBC configuration route;
the generic `43h` default is preserved, and PC110's `65h` disables AUX until
the guest sends the ordinary `A8h` command. All 714 unit tests pass on both
widths; complete 755-test host runs retain only the unrelated
`lib.integration/native_midi_output` host-endpoint failure. Eight existing PC
artifacts are rebuilt and PE-width checked; four MyPC110 previews stay
unpublished. The active packet will be replaced only when the next bounded
T580 receiver is admitted.
