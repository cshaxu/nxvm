# M5 T580 S76 — Active: PC110 operating-system progression

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S76. |
| Admission And Approval | Owner authorized autonomous PC110 implementation and S75 was accepted at `57bae0c18`. |
| Objective | Follow the real TFT BIOS and validated loader beyond its first instruction, capture actual system output and execution state, and identify and fix any evidenced next owner defect. |
| Non-goals | No firmware interception, injected guest state, external image writes, Shared Lib/Emulator changes, or unsupported claim that a bounded run reached a DOS prompt. |
| Baseline | [S75 evidence](../etc/evidence/t580-s75-system-loader.md) establishes matching initial loader execution on both host widths. |
| Candidate Proposal | Continue the existing real-firmware diagnostic after loader handoff for a finite instruction budget, capture the actual guest display and machine stop reason, then use evidence to select any further repair. |
| Files And ABI Surface | Existing PC110 diagnostic and responsible App or X86-NXVM/Core code only if the observed behavior requires a repair. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden bounded background execution using stable Ninja trees; x64 first, matching x86 and affected regressions when justified. |
| Asset Needs | S73 task-owned derived test disk and Setup oracle, read-only inputs and guest overlays; retain external originals unmodified. |
| Reporting Requirements | Distinguish actual guest screen, loader progression, DOS prompt, display and desktop results; record exact bounds, hashes and remaining gaps. |
| Stop Conditions | Do not inject guest code, mutate external images, or treat reaching an instruction budget as operating-system success. |
| Exit Criteria | A real DOS boot checkpoint is independently demonstrated or a specific evidenced next failing owner is repaired and validated. |
| Original Owner Request | Complete PC110 implementation, original and TFT variants, and four published executables. |
| Similar-Issue Sweep | Check the actual responsible loader, disk and display routes without speculative special cases. |

The former F65535 status-port wait is corrected by Shared `e3bb44fc1`;
targeted dual-width controller and real-firmware FDC checks pass. The x64
post-loader replay passed that wait but later accumulated exceptions and
entered another loop; a bounded follow-up captures its exception context.
See [S76 evidence](../etc/evidence/t580-s76-status-port.md). S76 remains
active and no DOS or PC110 release qualification is claimed.

The subsequent context capture found an empty VM86 INT 1Ch IDT gate, so the
observed IDT-related #GP is consistent with the CPU check. The later loop
reads the BIOS timer byte at 0000:046Ch rather than polling the 02F8h port;
the 50-million-instruction run ended after only three ticks while the guest
waits for more than nineteen. A longer bounded real-guest replay is underway; this
does not yet establish DOS startup.

# M5 T580 S75 — Accepted: PC110 system-loader handoff

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S75. |
| Admission And Approval | Owner authorized autonomous PC110 implementation and S74 was accepted at `d9b4cf893`. |
| Objective | Follow the genuine selected TFT BIOS and disk boot chain through loading and executing the initial DOS system loader; independently compare guest-loaded bytes with the documented test disk. |
| Non-goals | No BIOS or disk interception, synthetic guest-memory injection, ROM modification, external media writes, Lib/Emulator change, or claim that loader entry proves DOS startup. |
| Baseline | [S74 evidence](../etc/evidence/t580-s74-partition-boot.md) proves both host widths reach the actual partition sector and x64 reaches the selected disk's system-file comparison. |
| Candidate Proposal | Extend the existing bounded diagnostic to observe the first system loader at its disk-defined handoff, with independently read expected bytes; investigate and repair only any evidenced receiver defect. |
| Files And ABI Surface | Existing MyPC110 diagnostic, responsible App or X86-NXVM/Core source and associated tests only when justified. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden bounded background execution using stable x64/x86 Ninja trees, independent disk evidence, affected regressions and manifests according to actual changes. |
| Asset Needs | Reuse the S73 task-owned reviewed test master and Setup oracle as read-only inputs with overlays; retain their external originals untouched. |
| Reporting Requirements | Distinguish partition entry, loader entry, DOS prompt and desktop; record input and output hashes, actual tests and remaining gaps. |
| Stop Conditions | No guest boot-code injection, external image mutation or unsupported physical-accuracy claim. |
| Exit Criteria | Real loader handoff proven at both widths or the specific next failure identified and repaired with relevant verification. |
| Original Owner Request | Complete PC110 implementation, both panel variants and four released executables. |
| Similar-Issue Sweep | Check disk geometry and shared receiver callers without speculative product-specific changes. |

S75 delivery: the unchanged TFT BIOS and selected disk loaded and executed
the independently checked 1,536-byte system loader on both host widths.
Both widths built, passed all 12 MyPC110 units and rejected missing media.
See [S75 evidence](../etc/evidence/t580-s75-system-loader.md); only the App
diagnostic and task records changed, with no production or release binary
changes. DOS prompt, original panel, Window colour and PC110 release remain
open under T580.

# M5 T580 S74 — Accepted: PC110 partition boot receiver

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S74. |
| Admission And Approval | Owner authorized autonomous PC110 implementation, source-labelled L2 references, and S73 closed at `a916220f9`. |
| Objective | Continue the genuine TFT BIOS handoff through the actual MBR and its active partition boot sector, identify and repair any evidenced device, board or CPU receiver defect, and retain the remaining DOS/Personaware path for T580. |
| Non-goals | No host BIOS/disk interception, ROM patch, synthetic guest memory, external media writes, Lib/Emulator modification, or claim that reaching the partition sector proves DOS startup. |
| Baseline | [S73 evidence](../etc/evidence/t580-s73-pc110-ata-slave.md): dual-width real BIOS to matching primary MBR, validated ATA Recalibrate, and identified task-local geometry-adjusted test image. |
| Candidate Proposal | Extend the existing diagnostic after its verified MBR entry, follow the guest's ordinary INT 13h and the second 7C00h handoff, and independently compare the selected partition sector. Then observe the next finite DOS execution checkpoint and repair only a demonstrated existing owner. |
| Files And ABI Surface | Existing MyPC110 diagnostics, affected App or X86-NXVM/Core implementation and tests only if evidence requires; no new public contract without the existing owner authorization. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. |
| Verification | Hidden, finite background work in stable Ninja/ccache trees; x64/x86 direct guest and independent disk evidence; affected receiver regressions, manifests and artifacts as required by actual changes. |
| Asset Needs | Reuse S73's reviewed task-local test image and Setup oracle for this immediate continuation, each as a read-only input with overlay. Preserve and hash the source external image; remove task-local inputs after the receiver no longer needs them. |
| Reporting Requirements | Distinguish MBR, partition boot sector, DOS and desktop results; record source/test delta, provenance, budget, output hashes, failures and retained qualification gaps. |
| Stop Conditions | Never alter external master media or inject guest boot code; do not treat a timeout or sector handoff as OS success. |
| Exit Criteria | Actual MBR-to-partition transfer is proven on both widths, or a specific next failing owner is established and repaired within this bounded receiver; record subsequent DOS checkpoint honestly. |
| Original Owner Request | Complete actual PC110 implementation, both hardware variants and four published executables. |
| Similar-Issue Sweep | Check boot read geometry and shared ATA callers without changing other machines speculatively. |

S74 delivery: the independently compared MBR and active partition boot sector
were executed under the real TFT BIOS on both host widths. The x64 follow-up
also reached the selected disk's successful system-file-name check. Both
widths built, MyPC110 units passed 12/12 each, and missing-media preflight
failed correctly on each. See [S74 evidence](../etc/evidence/t580-s74-partition-boot.md)
for exact binary boundaries, hashes, provenance and remaining limitations.
Only the App diagnostic and task documentation changed; no production source
or released product binary changed. DOS, DSTN, Window colour and PC110
publication remain open under T580.

# M5 T580 — Open after S73

S73 is accepted: Shared `43f87360a` implements ATA Recalibrate and NXVM
`2056af519` records real-firmware dual-width BIOS-to-MBR handoff, receiving
regressions and eight rebuilt PC artifacts. The accepted pair uses an
explicitly documented, resized test master and the original read-only slave;
this is not DOS or desktop qualification. Further boot, RTC, display and
PC110 release work remains open. See [S73 evidence](../etc/evidence/t580-s73-pc110-ata-slave.md).

S72 Shared `54a2518de` and NXVM `9c0a31a54` are accepted for configuration
RAM persistence. Real TFT Setup/save and separate original-BIOS checksum
replays pass on both host widths; Product file handling is exercised by both
variants. The affected units, PC identity receivers, eight rebuilt PC EXEs
and specialized gates are recorded in the
[S72 evidence](../etc/evidence/t580-s72-cmos-persistence.md). The existing
Storage operation is not crash-atomic, and RTC calendar/off-time persistence
remains unqualified. T580 stays open for real guest boot, both panel models,
desktop behavior and PC110 publication.

S71 implementation `f23baaa30` is accepted for the bounded real-firmware
post-Setup power path and optimized x86 Release correction. Both widths reach
the real INT 19h handoff after the second wake; 669 x86 units and the affected
PC receivers pass, and the four optimized x86 PC artifacts are published.
The [S71 evidence](../etc/evidence/t580-s71-postsetup-power.md) preserves the
exact timeout and remaining qualification boundaries. T580 remains open.

S70 is accepted: Shared `3d692e367` corrects the 8259 input lifetime and
NXVM `3a1c703a0` records dual-width receiver evidence and eight rebuilt PC
artifacts. X86-NXVM units pass 233/233 per width, both real-firmware FDC and
boot-handoff checkpoints pass per width, and all eight PC profile identity
and boot receivers pass under their final measured budgets. Earlier timeouts
and the old-PIC control remain documented, not relabelled as passes.

The unchanged TFT BIOS passed the previously observed `102` branch and the
subsequent power wait on both widths in S71. RTC calendar persistence, main
Setup, actual guest/media boot, PCIC, panel identity and color, DSTN, desktop
behavior and PC110 product publication remain unqualified. See
[S70 evidence](../etc/evidence/t580-s70-reset-interrupt-investigation.md).

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
