# Project Status

## Current Work

## M5 T580 S60 — PC110 ATA master CHS construction fact

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S60, next unused numeric S after the completed S59 construction route; one-session coordinator/executor. |
| Admission And Approval | The owner’s continuing instruction to implement PC110 through open T580 permits autonomous, in-scope App/X86-NXVM repair. This consumes S55's retained primary-geometry receiver. Lib, Emulator, X86 Core, X86 Debug, external media and PCIC behavior remain excluded. |
| Objective | Make the selected PC110 plan consume the existing master-only fixed-geometry construction contract for a correctly sized primary SunDisk image: 123 cylinders, two heads and 32 sectors per track. Demonstrate that a matching BYOB image reaches the existing HDC path with that CHS geometry, while the second ATA slot remains independently unconfigured. |
| Non-goals | No configuration grammar or public API change; no new HDC, queue, storage backend, device manager, PCIC/CF mapping, CMOS seed, guest boot claim, external media/ROM mutation, artifact publication or fixed geometry for the slave. Do not treat an arbitrary 50 MB image as a SunDisk or relax the existing exact-size geometry check. |
| Reference Baseline | Clean pushed `b0cf67865` after S59. S59 makes both declared disk slots real and routes the secondary ID to ATA slave. The existing Machine contract can set an exact CHS geometry only for physical master slot zero, but MyPC110 currently does not select it and therefore falls back to generic 16x63 translation for every attached master. |
| Candidate Proposal | In `app-mypc110` only, declare the source-recorded 123x2x32 master geometry through the existing construction fields. Extend the existing MyPC110 construction test with a disposable, exact-size synthetic medium and prove the resulting real Machine/Board ATA master geometry; retain existing two-slot and absent-slave assertions. No change to the generic slot route or ATA chip is required. |
| Files And ABI Surface | Admitted production surface: `src/app-mypc110/pc110_plan.c` only. Admitted test surface: existing `test/app-mypc110/unit/machine_construction_smoke.c`, its manifest only if required. Evidence/current/history updates are admitted. The existing X86-NXVM construction and HDC interfaces are consumed unchanged. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING,UI}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; `docs/nxvm/etc/operations/policy/source-policy.md`; `src/x86-nxvm/README.md`; T580 proposal and convergence ledger. Preserve one media registry and HDC route, inward dependencies, BYOB inputs, existing stable Ninja/ccache graphs and protected external inputs. |
| Verification | Direct tests prove plan declaration, exact geometry publication through the real Machine/Board ATA master route, fixed-size rejection, and unchanged secondary absence. Run changed App/X86-NXVM x64/x86 units and manifests, then review actual diff. An external-BIOS regression is non-regression evidence only and is run only if the matching external prerequisites are present. |
| Expected Markers | Evidence cites the directly inspected Open-Source-PC110 live ATA/INT13 findings, distinguishes its L2 provenance from firmware-manual facts, and proves the selected geometry is never applied to the slave or an incorrectly sized image. |
| Asset Needs | Existing source/tests only. A test-local synthetic 4,030,464-byte medium is disposable; it is not a SunDisk image. External BIOS/media stay read-only; no raw firmware, media, third-party source or Windows component enters the repository or executable. |
| Reporting Requirements | Report actual source/test delta, exact master geometry behavior, test results, source ownership and retained limits: authentic SunDisk/CF inputs, BIOS storage consumption, CMOS/Setup persistence, PCIC mapping, selected guest and publication remain unqualified. |
| Stop Conditions | Stop before implementation if exact master geometry needs a new cross-package contract, if the actual geometry source contradicts the current master-only contract, or if a real media input is required to make the unit proof meaningful. Do not fabricate an image, CMOS result or boot success. |
| Exit Criteria | MyPC110 selects the pre-existing exact master geometry route; direct x64/x86 proof and required source/test manifest gates pass; actual changes are reviewed/documented; worktree is clean or a complete reviewable P remains. T580 and product publication remain open. |
| Original Owner Request | “老规矩实现PC110”, followed by “加油”; owner permits source-labelled L2 model work and autonomous in-scope S admission, but not fabricated firmware/media behavior. |
| Similar-Issue Sweep | Inspect all MyPC110 construction paths and every master-only geometry application. Confirm this consumes rather than duplicates the existing Machine contract, leaves slave geometry untouched, and does not alter other PC plans. |

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
