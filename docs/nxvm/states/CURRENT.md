# M5 T580 — Open after accepted S64

| Field | Required record |
| --- | --- |
| Identifier Mode | Retained status — S64 implementation `40be5c4f2` is accepted; T580 remains open. |
| Admission And Approval | Owner has directed autonomous continuation of PC110 implementation (2026-10-26). The existing T580 source policy permits L2 external research as a behavioral reference, but no imported third-party source, firmware or media. |
| Objective | Model the source-recorded PC110 8042 power-on command-byte fact so the existing standard PS/2 TrackPoint route begins with AUX disabled and IRQ12 masked. |
| Non-goals | No TrackPoint firmware model, mouse protocol rewrite, private host input path, ROM/media import, persistence claim, desktop validation, release artifact publication or generic device framework. |
| Reference Baseline | Clean `07d942657`; S63 font-window receiver accepted. `Open-Source-PC110/Discovery/Trackpoint/readme.md` §§5/7a records command byte `65h`, AUX disabled, and standard PS/2 responses. Current generic reset produces `47h` when AUX is present. |
| Candidate Proposal | Extend the existing board-selected 8042 reset configuration family with an optional frozen command byte. Generic controller behavior remains its present default when unconfigured. MyPC110 selects `65h`; Board applies it after its ordinary 8042 reset. Existing AUX enable (`A8h`) and standard PS/2 commands remain the one route. |
| Files And ABI Surface | Expected: `src/x86-nxvm/chips/kbc8042/*`, `src/x86-nxvm/board-at/*`, `src/x86-nxvm/board-base/*`, `src/app-mypc110/pc110_machine.c`, existing owner tests under `test/x86-nxvm` and `test/app-mypc110`, manifests, evidence/history. This is an additive x86-nxvm public configuration field/setter; Lib and Emulator are untouched. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `ARCHITECTURE.md`, `CODING.md`, `DOCUMENT.md`; `docs/nxvm/design/ARCHITECTURE.md`, `CODING.md`; and `docs/nxvm/etc/operations/policy/source-policy.md`. Keep dependency direction; one owner/route; use `lib_*` types/status; source grade L2; no protected payload. |
| Verification | Prove generic unconfigured reset remains `47h`; prove PC110 reset reads `65h`, `A8h` enables AUX, `FFh/F2h/E9h/F4h` retain documented responses and mouse packets remain gated until reporting enabled. Run focused x86-nxvm and MyPC110 tests x64/x86, then full repository unit suites x64/x86; manifests, dependency/type/docs gates and diff review. Rebuild unpublished PC110 previews only. |
| Expected Markers | `PC110:BASE:composition:OK`; focused KBC reset configuration marker; full unit suites report all registered tests passing on both widths. |
| Asset Needs | Read-only external source text only; no ROM/font/media asset is consumed or altered. |
| Reporting Requirements | Record exact L2 source boundary, generic/default preservation, changed source/test line delta, focused and full verification separately, preview rebuild status, and remaining unqualified domains. |
| Stop Conditions | Stop if the required fact needs a second input path, changes ordinary PC reset semantics, requires invented TrackPoint firmware, source payload import, or a Lib/Emulator change. Report any public-contract conflict before expanding scope. |
| Exit Criteria | Met by implementation `40be5c4f2` and this governance record: one configured reset route owns the PC110 fact; generic and PC110 public-port proofs pass; unit, manifest, documentation and receiving-artifact gates pass. The external native-MIDI host integration remains explicitly non-passing because this host has no endpoint. T580 remains open. |
| Original Owner Request | Continue PC110 implementation toward real behavior; use the three external PC110 research repositories as L2 evidence without importing protected assets. |
| Similar-Issue Sweep | Review all KBC reset customization fields and all MyPC110 KBC/aux callers. Preserve generic defaults; do not create a competing mouse/keyboard initialization mechanism. |

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
