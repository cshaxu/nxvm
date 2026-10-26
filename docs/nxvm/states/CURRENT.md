# M5 T580 S67 — Active: source-backed PC110 extended CMOS seed

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S67. |
| Admission And Approval | Owner approved autonomous continuation of PC110 implementation and external BYOB assets. S67 follows accepted S66 (`daf38c36e`); its newly located source is the locally retained historical Open-Source-PC110 commit `b0a5a3bcf88079dd53c13eb06136fcac04c95be6`, not an imported source dependency. |
| Objective | Qualify the existing single CMOS-seed route for the PC110's documented extended `70h/71h` address space using the exact 128-byte live capture, retaining Core as the sole live RTC owner, then determine by actual unchanged-firmware replay whether a valid configuration seed advances normal Setup. |
| Non-goals | No ROM/BIOS modification, PC hook, synthetic F1, host service, second RTC/register image, guest media substitution, PCIC implementation, display/panel override, raw protected payload in this repository, or PC110 release publication. DSTN does not inherit the TFT capture. |
| Reference Baseline | `5c18d73fe`; S61's prior source gate, S66 real Easy-Setup handoff evidence, current `x86-nxvm` 64-byte seed route, and the owner-managed `nxvm-assets/profiles-nxvm` archive. |
| Candidate Proposal | Extend the existing seed representation to accept only 64-byte classic or 128-byte extended payloads; preserve the input length through Machine construction; materialize defaults only for configuration registers `0Eh..last-byte`; reject a 128-byte seed on a non-extended board. The Board default array expands only to its real maximum of 114 writable configuration registers. TFT CMake input selects an external hash-checked seed; DSTN remains unseeded. |
| Files And ABI Surface | `src/x86-nxvm/machine/{construction_interface.h,preparation.c,machine.c}`, `src/x86-nxvm/board-base/machine_board_interface.h`, existing x86-nxvm tests/manifests, `src/app-mypc110/CMakeLists.txt`, PC110 tests/manifest, and external TFT profile manifest/seed. The existing asset view and construction structs gain the copied seed length; no Lib, Emulator, Core, ROM, INI, or runtime public API changes. |
| Applicable Rules | Docs guide Task Reading Set; execution lifecycle and actual-diff review; architecture's one writable RTC owner and BYOB packaging boundary; coding rules for `lib_*` vocabulary and minimal single path; source policy for read-only research and external raw payloads. |
| Verification | Prove 64-byte legacy and 128-byte extended acceptance; reject malformed lengths and extended data on classic decode; prove `0Eh`, `3Fh`, `40h`, and `7Fh` default delivery through the existing Board route; configure/build x64/x86; run affected x86-nxvm and MyPC110 units; replay unchanged TFT BIOS through existing real-firmware diagnostic and record exact checkpoints. |
| Expected Markers | Seed SHA-256, copied-length/default-count assertions, selected extended-register reads, and dual-width real-firmware disposition. A reachability limit is containment, never successful configuration or boot. |
| Asset Needs | One 128-byte TFT CMOS seed in external `nxvm-assets/profiles-nxvm/ibm-pc110-tft`, provenance and SHA-256 recorded there. It is materialized byte-for-byte from the local historical live capture; no raw payload enters Git. |
| Reporting Requirements | Record source provenance, exact diff counts, x64/x86 results, unchanged paths, and whether normal configuration progresses. State every remaining qualification gap explicitly. |
| Stop Conditions | Stop before inventing missing values, accepting a stale date/calendar as RTC ownership, applying TFT state to DSTN, or interpreting an execution budget as boot success. Keep scratch only under `build/t580-s67`; remove it after recording permissible evidence. |
| Exit Criteria | One reviewed implementation P with focused dual-width proof, asset-manifest validation, actual unchanged-firmware replay, evidence/current/history update, clean worktree and push. S67 closes only the seed receiver/disposition, not PC110 qualification or release. |
| Original Owner Request | Continue the PC110 T autonomously using existing external reference and BYOB assets, with normal compile/test/release governance. |
| Similar-Issue Sweep | Compare all seed consumers and Board RTC-default callers; preserve classic profiles, non-extended decode, RTC calendar ownership, and both PC110 variants. |

## S67 execution record

The implementation and source-boundary evidence are ready for owner review.
Both full unit suites pass (669/669 x64 and 669/669 x86); focused real TFT
firmware replay reaches the established Setup handoff/reset checkpoints on
both widths. Eight receiving-PC artifacts are rebuilt and PE-width checked.
The selected x86 DeskPro floppy matrix remains explicitly non-green after two
300,000 ms runs and one 330,000 ms diagnostic: it is the existing Model40 x86
throughput qualification concern, not a PC110-seed result. S67 therefore does
not claim a fully green all-profile integration sweep or PC110 publication.

# M5 T580 — Open after accepted S66

S66 is accepted at `daf38c36e`.  The unchanged selected TFT BIOS now
has direct dual-width evidence for full guest-side Easy-Setup decompression,
the original `9000:009E -> 5000:0000` handoff, a valid 640×480 graphics frame,
ordinary two-Enter delivery and the firmware's subsequent reset.  The
scratch-only comparison oracle, frames and logs are removed after hash review;
no production or Shared code changed.  See [S66 evidence](../etc/evidence/t580-s66-real-easysetup-handoff.md).

The result closes only that loader/first-frame receiver.  Normal F1 branch
selection and configuration persistence, selected guest/media boot, PCIC,
physical panel identity, live desktop behavior, original DSTN qualification,
and PC110 artifact publication remain unqualified.

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
