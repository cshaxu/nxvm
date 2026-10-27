# M5 T580 S73 — Active: PC110 ATA slave boot receiver

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S73. |
| Admission And Approval | Owner approved autonomous PC110 implementation and reference-based L2 modeling; S72 accepted in `4ecacbb9f`. |
| Objective | Exercise genuine PC110 BIOS discovery and bootstrap with the available owner-supplied Personaware disk attached through the existing ATA slave route; identify and repair any evidenced receiver defect in this bounded boot path. |
| Non-goals | No firmware patch, host BIOS/disk interception, synthetic guest state, modification or distribution of the supplied disk, Lib/Emulator changes, or claim of a booted guest from sector entry alone. |
| Reference Baseline | S55 BIOS/ATA evidence, S59 two-unit construction, S60 physical master geometry, S72 CMOS persistence and the current original firmware. |
| Candidate Proposal | Extend the existing real-firmware diagnostic to attach an explicitly selected disk in slot 1 using overlay, independently read its first sector and observe real BIOS ATA/INT13 and execution. Compare the slave-only run with a separately identified blank, geometry-correct temporary master in the existing hardware path; neither substitutes for original factory media. Repair only the responsible existing device or board owner if a reproduced mismatch is found. |
| Files And ABI Surface | Existing MyPC110 diagnostic and App/ X86-NXVM receivers and corresponding tests as evidence requires; no Lib/Emulator change. |
| Applicable Rules | NXVM Task Reading Set, execution, architecture, coding and source/research policy. External reference code is behavioral evidence, not copied implementation. |
| Verification | Run bounded hidden background work in stable Ninja/ccache trees. Record source-labelled media size and hashes before and after, actual ATA and BIOS checkpoints, and guest-visible output; verify both widths for the selected receiver and relevant regressions. Use overlay and task-local output only. |
| Expected Markers | Real guest BIOS selects a genuine configured storage unit and executes its independently verified boot sector, or the first reproducible owner-specific storage failure is located with direct evidence; this marker is not a DOS/Personaware completion claim. |
| Asset Needs | Existing owner-supplied PC110 BIOS, CMOS and local Personaware disk; all raw firmware and guest media remain external and unmodified. |
| Reporting Requirements | Record provenance, selected slot/geometry, source/test delta, validation and unresolved firmware, disk, RTC, panel and product-qualification limits. |
| Stop Conditions | Never write the external image or inject guest boot code; do not misrepresent PC110-QEMU's host POST/INT13 hooks as a native BIOS boot. |
| Exit Criteria | The boot-sector receiver is directly validated or its specific next failing owner is established, with source and dual-width evidence appropriate to the finding. |
| Original Owner Request | Complete actual PC110 implementation, including both hardware variants and four published executables. |
| Similar-Issue Sweep | Check primary and secondary ATA slot geometry and selected boot policy without changing other machines. |

# M5 T580 — Open after S72

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
