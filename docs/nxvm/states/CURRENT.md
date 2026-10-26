# Project Status

## Current Work

## M5 T580 S58 — PC110 dual-slot PCIC receiver reconciliation

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S58, next unused numeric S after accepted S57; one-session coordinator/executor. |
| Admission And Approval | The owner’s persistent instruction to implement PC110 through the open T580 qualification goal, including autonomous in-scope App/X86-NXVM research and implementation. Lib and Emulator are excluded. A necessary new Shared public boundary must be reported before implementation. |
| Objective | Reconcile the TFT BIOS’s actual 3E0h/3E1h PCIC probing with the documented two-slot Ricoh RB5C396 contract and the existing PC110 App/Core routes. Implement one complete, source-labelled receiver only if the evidence establishes its register, card, IRQ/window/reset and ownership behavior; otherwise preserve the explicit source-gated disposition. |
| Non-goals | No BIOS-PC intercept, one-off ID reply, CMOS success seed, ROM/media mutation, synthetic card insertion, host PCMCIA API, Lib/Emulator change, external source import, guessed error-code attribution, guest/DOS claim or PC110 artifact publication. |
| Reference Baseline | Clean pushed `bca752a03`, accepted S57. The unchanged TFT BIOS reaches real ERROR/date-time frames; S55 records BIOS index scans at 3E0h/3E1h and absent App PCIC routing. The frozen T580 ledger keeps PCIC/card as an open qualification row. |
| Candidate Proposal | Use the original Ricoh EA-028-9804 manual and allowed external references only as evidence. First inventory the BIOS access sequence, RB5C396 register spaces, two-socket identity, card-detect/status, memory/I/O windows, IRQ signalling and reset requirements against current `app-mypc110`, `x86-nxvm/board-base` and Core APIs. Select one existing owner or report the smallest required X86-NXVM contract before code. A receiver, if justified, owns no duplicated guest time/queue and is wired only through existing Core port/memory/IRQ routes. |
| Files And ABI Surface | Admitted production surface: `src/x86-nxvm/chips/rb5c396/{rb5c396.c,rb5c396.h,rb5c396_interface.h}`, `src/x86-nxvm/CMakeLists.txt`, `src/app-mypc110/{pc110_plan.c,pc110_pcic.c,pc110_pcic_interface.h,CMakeLists.txt}`. Admitted test surface: `test/x86-nxvm/chips/rb5c396/rb5c396_contract_smoke.c`, `test/x86-nxvm/CMakeLists.txt`, `test/app-mypc110/{unit/pc110_pcic_smoke.c,integration/real_firmware_post_integration.c,CMakeLists.txt}`, plus affected manifests/evidence/proposal/current/history. The receiver owns only indexed controller state and physical-slot inputs; App owns PC110 port decode. No Lib/Emulator/Core surface is admitted. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING,UI}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; `docs/nxvm/etc/operations/policy/source-policy.md`; `src/x86-nxvm/README.md`; T580 proposal and convergence ledger. Preserve inward dependencies, one owner per state/route, existing stable Ninja/ccache graphs, BYOB source policy and protected external inputs. |
| Verification | Before any implementation, record the complete BIOS probe transcript and a source-to-register/route matrix. If a receiver is admitted: direct chip/board/App tests cover both slots, absent/present card states, documented readonly identity, register selection, window ownership, IRQ/reset and real BIOS probe on x64/x86. Rebuild affected existing graphs and run affected unit/integration tests. No test or trace is called a guest/product qualification beyond its actual checkpoint. |
| Expected Markers | An evidence record names source grade, exact documented register facts, current route gaps, selected owner and each unmodelled contract member. A passing receiver, if any, is identified by real PCIC register/state checks rather than a BIOS-PC special case. |
| Asset Needs | Read-only external EA-028-9804 manual and current owner-provided TFT BIOS only. No ROM, media, Windows component or third-party source enters the repository or executable. |
| Reporting Requirements | Report the BIOS fact, exact receiver decision, proposed public boundary before any new Shared API, actual source/test diff, verification and every unproven PCIC/card/guest boundary. |
| Stop Conditions | Stop before implementation if the documented register/card/IRQ/window/reset contract cannot be completed, or if required ownership is outside App/X86-NXVM and has no approved public boundary. Do not replace missing evidence with a successful BIOS probe value. |
| Exit Criteria | Either: (a) one coherent two-slot receiver is implemented, source-labelled, tested on both host widths and mapped into the T580 ledger; or (b) the complete unresolved PCIC class is source-gated with evidence and transferred to the earliest legitimate receiver. In both cases, worktree is clean or only a reviewable complete P remains; whole T580/product publication stays open. |
| Original Owner Request | “老规矩实现PC110”, with autonomous S admission and explicit direction to use available source/reference evidence for functional L2 rather than block on physical hardware proof. |
| Similar-Issue Sweep | Audit both PCIC sockets and all BIOS-visible selector/data/IRQ/window paths, not only the first 3E0h ID read. Reconcile existing PC/AT port routes so PC110-specific state does not alter the other fixed PC Apps. |

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
