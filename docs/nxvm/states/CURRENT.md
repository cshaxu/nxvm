# Project Status

## Current Work

## M5 T580 S62 — PC110 unmodified-Setup palette capture

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S62, next unused numeric S after S61's accepted external-asset transfer; one-session coordinator/executor. |
| Admission And Approval | The owner's continuing instruction to implement PC110 and explicit report of a blue ERROR/Easy-Setup display authorize a bounded diagnostic of the existing App-local real-firmware integration. No Shared, production, payload or external-project source change is admitted. |
| Objective | Establish whether the current, unmodified TFT BIOS path produces the reported blue Easy-Setup/ERROR frame, and record actual RGB frame data before selecting any colour or configuration receiver. |
| Non-goals | No BIOS/Setup patch, palette constant/theme override, BIOS-PC interception, synthetic theme selection, CMOS fabrication, new renderer, production interface, external payload import, product publication or guest qualification. Do not copy PC110-QEMU/PC110-EMU implementation. |
| Reference Baseline | Pushed `59fa5c354`. S57's existing F65535 RGB path resolves actual DAC values as `0x00RRGGBB`, while S61 confirms the real extended CMOS image is absent. PC110-QEMU reports direct-loader blue-theme behavior but fixes it by modifying the guest image, which is excluded. |
| Candidate Proposal | Extend only the existing `mypc110-real-firmware-post-integration` diagnostic with a bounded capture invocation that follows its actual F12 → power restore → BIOS Setup entry path without requiring the removed 232,185-byte comparison oracle. Emit a temporary PPM and palette/hash observation, then remove temporary-only diagnostic mechanics unless a reusable regression has a concrete owner. |
| Files And ABI Surface | Admitted: `test/app-mypc110/integration/real_firmware_post_integration.c` only if needed for the bounded diagnostic; `docs/nxvm/{states,etc/evidence,history}`. No `src/`, Lib, Emulator, X86 Core/Debug/NXVM, ROM, INI or external media change. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING,UI}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; `docs/nxvm/etc/operations/policy/source-policy.md`; T580 proposal and convergence ledger. Preserve the sole Core RTC owner, BYOB payload policy, inward dependencies and external-payload boundary. |
| Verification | Run the focused MyPC110 integration on x64 then x86 from the reusable Ninja trees. Verify capture follows real Setup entry, retains the existing production frame path, records image dimensions, dominant RGB values and a SHA-256, and does not change ROM/RAM/CMOS inputs. Run affected unit/manifest gates if a test source edit remains. |
| Expected Markers | One frame per host width after the same real BIOS entry, equal dimensions/content hash when deterministic, a direct statement whether the dominant background is rose (`BE9E96`) or blue, and clear distinction between observed output and external L2 comparison. |
| Asset Needs | Existing immutable TFT BIOS/font assets only. Temporary PPM/log belongs under a named `build/t580-s62-*` scratch directory and is deleted after its hash/counts are recorded. No new external file is fetched or retained. |
| Reporting Requirements | Record the exact invocation, observed pixels/hash, source grade and any mismatch. State whether a source-backed configuration/initialization receiver exists; do not call a blue frame a renderer defect without data. |
| Stop Conditions | Stop if capture requires a BIOS patch, address intercept, absent oracle, invented CMOS bytes, or a new production contract. Remove temporary diagnostics and report the exact blocker rather than leaving test-only behavior. |
| Exit Criteria | The real path either yields a deterministic frame disposition with no production change, or proves an exact blocking prerequisite. Temporary artifacts are removed, docs gate/review pass, workspace is clean or contains one reviewable P, and T580 remains open. |
| Original Owner Request | “老规矩实现PC110”, then the reported blue/pink PC110 screen discrepancy and “加油”. Owner permits autonomous in-scope evidence work, but not fabricated firmware/media behavior. |
| Similar-Issue Sweep | Confirm the diagnostic does not create a second Setup runner, output path or palette resolver; inspect both ERROR and Easy-Setup frame acquisition points and retain only the existing F65535 → Machine → Window route. |

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
