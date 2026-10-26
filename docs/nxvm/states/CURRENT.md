# M5 T580 S68 — Active: TFT Easy-Setup input/reset semantic boundary

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S68. |
| Admission And Approval | Owner approved autonomous continuation of the active PC110 implementation task. S67 is accepted through `9fdfc29d8`; this diagnostic consumes the next unresolved Easy-Setup input/reset member from the T580 ledger. |
| Objective | Establish, using the unchanged selected TFT firmware and existing external source evidence, what semantic path the current real Easy-Setup screen takes after ordinary keyboard input and its observed processor reset. Determine whether the result identifies a complete, source-backed receiver or must remain a bounded disposition. |
| Non-goals | No BIOS/ROM patch, forced F1/Enter outcome, fabricated CMOS values, host UI interception, model-selector, runtime media substitution, new execution path, or PC110 publication. No code change merely to make a chosen screen advance. |
| Reference Baseline | `9fdfc29d8`; S66 real Setup-loader handoff, S67 external TFT seed receiver, current `test/app-mypc110/integration/real_firmware_post_integration.c`, and owner-managed external TFT BIOS/seed. |
| Candidate Proposal | First audit the diagnostic's exact input injection point, post-input control flow, reset reason and persisted Board/CMOS effects. Correlate those facts with selected firmware/source records. If evidence identifies an already-owned defect with a complete contract, revise this packet before repair; otherwise record a source-bounded disposition and leave implementation unchanged. |
| Files And ABI Surface | Initially read-only: existing MyPC110 real-firmware diagnostic, App construction and external source/archive evidence. A later repair, if justified, requires an explicit packet revision naming only its owner, tests and ABI surface. |
| Applicable Rules | Docs guide Task Reading Set; execution lifecycle and evidence-led convergence; architecture's single RTC/configuration owner; coding/source rules; no firmware/media import into Git. |
| Verification | Reproduce the existing transfer on x64/x86; capture only existing CPU/Board/CMOS observations around each injected input and reset; inspect static source/firmware context; distinguish a checkpoint from successful Setup/persistence/boot. No benchmark or retirement bound is success by itself. |
| Expected Markers | Input make/break delivery point, guest PC/reset observation, reset vector/reason, relevant configuration deltas, and an evidence-backed disposition. |
| Asset Needs | Existing external TFT BIOS, 1 MiB font and optional external 128-byte seed only. No new protected asset is imported. |
| Reporting Requirements | Record exact input transaction and reset semantics, source grade, both-width result, unchanged paths and next owner/receiver if unresolved. State whether any proposed repair has a complete contract. |
| Stop Conditions | Stop before interpreting a reset as saved configuration, treating a display frame as normal Setup completion, inferring a hardware protocol from a single screen, or adding a synthetic route. If the remaining semantics are underdetermined, record and transfer them. |
| Exit Criteria | One evidence-backed disposition or a separately admitted, fully specified repair; task-owned scratch cleaned; no PC110 product/release claim. |
| Original Owner Request | Continue the PC110 T autonomously using existing external reference and BYOB assets, with normal compile/test/release governance. |
| Similar-Issue Sweep | Check both host widths and both injected Enter transactions; distinguish CMOS configuration, reset control and keyboard delivery owners rather than adding a PC110-only input path. |

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
