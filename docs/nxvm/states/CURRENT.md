# M5 T580 S65 — F65535 panel-table receiver audit

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T580 S65, next linear S after accepted S64. |
| Admission And Approval | The owner directed autonomous PC110 continuation on 2026-10-26 and specifically asked that the observed ERROR/Easy-Setup blue-versus-pink discrepancy be investigated. This packet authorizes a bounded real-firmware F65535 panel-table audit; it does not authorize an inferred colour swap. |
| Objective | Determine whether the selected TFT BIOS writes its F65535 XR panel table through the installed 3D6h/3D7h route, compare actual values with source-recorded PC110 panel facts, and establish whether the F65535 owner has a demonstrated receiver defect relevant to the reported colours. |
| Non-goals | No physical LCD/TFT/DSTN renderer, host/RDP/DPI modification, palette/RGB/BGR inversion, BIOS interception, ROM/media import, new display route, or PC110 publication claim. Do not modify standard VGA unless direct evidence identifies its owner defect. |
| Reference Baseline | Accepted S64 P1 `40be5c4f2`, P2 `a1814f953`. `src/x86-nxvm/chips/f65535` owns generic XR storage; `src/app-mypc110/pc110_display.c` maps 3D6h/3D7h. `Open-Source-PC110/Discovery/65535/readme.md` is L2 reference evidence for observed panel XR values and panel-table behavior. |
| Candidate Proposal | Extend only the existing real-firmware diagnostic's bounded trace selection to observe existing XR ports. If BIOS reaches them, record the actual index/data sequence and compare it with source-recorded values. Change production only if direct evidence proves the existing F65535 route loses, aliases, or corrupts an observed access. |
| Files And ABI Surface | Expected diagnostic/evidence scope: `test/app-mypc110/integration/real_firmware_post_integration.c`, its registration or supporting test only if required, `docs/nxvm/etc/evidence/`, history and this packet. No public interface or Shared component change is planned. A demonstrated production defect is limited to `src/x86-nxvm/chips/f65535` or the MyPC110 display adapter and requires a packet revision before expansion. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `ARCHITECTURE.md`, `CODING.md`, `DOCUMENT.md`; `docs/nxvm/design/ARCHITECTURE.md`, `CODING.md`; and `docs/nxvm/etc/operations/policy/source-policy.md`. Maintain dependency direction, one F65535 register owner and one standard-VGA rendering route. External repositories are L2 reference only; no protected firmware/source enters Git. |
| Verification | Inspect and run the existing embedded-TFT BIOS harness through a bounded background run. Prove whether XR writes were observed, retain exact port/index/value trace only in evidence, then run affected MyPC110 x64/x86 diagnostic/unit tests, manifests and documentation gate. If a production defect is fixed, run owner tests on both widths plus necessary regression. Separate unavailable RDP/physical-panel validation. |
| Expected Markers | Existing real-firmware diagnostic marker plus evidence that states `XR route observed and retained`, `XR route not reached within the named semantic checkpoint`, or a proven owner defect. No synthetic display-success marker. |
| Asset Needs | The already owner-supplied external TFT BIOS selected by the existing build may be executed but not copied or committed. Open-Source-PC110 remains read-only L2 research. |
| Reporting Requirements | Record source-evidence boundary, actual observed XR accesses or their absence, comparison result, whether production change was justified, source/test line delta, and executed versus unexecuted RDP/desktop validation. |
| Stop Conditions | Stop if proof requires a physical-panel model, guessed BIOS table, RGB swap without observed wrong register/data transformation, imported protected data, or any Lib/Emulator contract change. |
| Exit Criteria | The actual real-firmware route has a durable bounded disposition: observed and retained, proved defective and corrected with both-width regression, or not reached at the named semantic checkpoint with no speculative repair. Documentation, manifests and scoped gates pass; no display claim exceeds evidence. |
| Original Owner Request | Continue PC110 implementation toward actual behavior using permitted external PC110 research as L2 evidence; investigate reported ERROR/Easy-Setup colour discrepancy without papering it over. |
| Similar-Issue Sweep | Examine all F65535 XR index/data accesses and MyPC110 port adapter for width, aliasing or storage issues. Keep standard VGA Attribute/DAC/PEL colour ownership distinct from panel-extension storage. |

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
