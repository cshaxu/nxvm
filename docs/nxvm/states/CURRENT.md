# Project Status

## Current Work

## M5 T580 S63 — PC110 Font-ROM window receiver

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S63, next unused numeric S after S62's evidence-only closure; one-session coordinator/executor. |
| Admission And Approval | The owner’s standing instruction to continue PC110 implementation authorizes a source-backed App-local receiver. It changes neither Shared components nor external payloads. |
| Objective | Complete the observed 1 MiB PC110 font-ROM banking window: 7-bit bank readback, fixed segment/readback, enable state and the conditional DE000h aperture. |
| Non-goals | No font bytes, BIOS/ROM modification, invented 1161h scratch semantics, remappable segment, generic ROM-window framework, guest hook, panel/scanout change or PC110 publication. |
| Reference Baseline | Pushed `5d406de16`. Current `mypc110_font_rom` writes only 1160h and permanently claims DE000h; Open-Source-PC110 live documentation records the full 1160h–1163h geometry and disabled-window FF behavior. |
| Candidate Proposal | Keep one App-local font object and one existing port/memory provider. Add only source-observed register facts: 1160h bank R/W with bit 7 ignored; 1161h reads FF; 1162h reads DE; 1163h is bit-0 enable/readback. The provider owns disabled reads and returns their observed FF open-bus value, avoiding an unrelated ordinary-RAM fallback. |
| Files And ABI Surface | `src/app-mypc110/pc110_font.{c,h}`; `test/app-mypc110/unit/font_rom_smoke.c`; their CMake/manifest only if required; evidence/history/current. No Lib, Emulator, X86 Core/Debug/IBMPC/NXVM public interface, firmware, INI or external assets. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; source policy; T580 proposal/ledger. Preserve immutable external font ownership, inward dependencies and one provider route. |
| Verification | Expand the existing font unit to prove bank aliases/readback, fixed unsupported/segment reads, enable readback and enabled versus disabled DE000h open-bus behavior. Run MyPC110 units x64/x86, repository units x64/x86, manifests and documentation gate. Rebuild PC110 previews only if production input changes; do not publish an unqualified PC110 product. |
| Expected Markers | One port-provider installation handles the four-byte range; bank 80h aliases 00h; disabled reads return the observed open-bus FF value; enabling restores the selected immutable bank. The source records post-initialization live values, not an independent reset contract. |
| Asset Needs | Existing external immutable TFT font only; no acquisition, write or payload retention. Scratch, if used, is `build/t580-s63-*` and is deleted before closure. |
| Reporting Requirements | Record source grade, exact register behavior, production/test diff, two-width results and retained limits. State that this does not qualify DOS/V use, panel output, configuration persistence, guest boot or release. |
| Stop Conditions | Stop if correct behavior requires undocumented 1161h writes, an invented remappable physical address, modified font data, Shared interface or a second memory route. |
| Exit Criteria | Existing unit ownership proves all observed register and aperture states, affected gates pass in both widths, documentation/change review completes, artifacts stay correctly unmodified or are recorded, and one reviewable P is pushed. T580 remains open. |
| Original Owner Request | “老规矩实现PC110” and continuing “加油”; routine source-backed App-local receivers are autonomously admitted. |
| Similar-Issue Sweep | Inspect every PC110 memory/port aperture for an always-enabled or unsupported-control analogue; repair only the same fully observed Font-ROM mechanism, not unrelated windows. |

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
