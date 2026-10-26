# Project Status

## Current Work

## M5 T580 S59 — PC110 ATA two-device construction route

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation — M5 T580 S59, next unused numeric S after S58 executor delivery; one-session coordinator/executor. |
| Admission And Approval | The owner’s continuing instruction to implement PC110 through the open T580 qualification goal, including autonomous in-scope X86-NXVM/App repair. This consumes the S55 ledger’s explicit two-device ATA construction gap. Lib, Emulator, X86 Core, external media and PCIC behavior are excluded. |
| Objective | Make the already declared two-slot X86-NXVM fixed-disk configuration real: allocate, registry-bind, reset and startup-insert each declared slot through the existing single media/HDC route. Bind MyPC110’s standard ATA HDC to the existing secondary registry ID, so a valid `fixed_disk1` reaches ATA slave selection. |
| Non-goals | No config grammar or cross-package API; no new HDC, queue, storage backend, device manager, PCIC/CF mapping, fixed geometry guess, guest boot claim, media/ROM mutation, artifact publication or changes to Lib, Emulator, X86 Core, X86 Debug or IBM PC. |
| Reference Baseline | Clean pushed `ce05cf632` after S58. Product INI already parses `fixed_disk0` and `fixed_disk1`; Product forwards both slots; Board HDC already accepts master/slave media IDs. Machine rejects slot 1 and only services slot 0; MyPC110 currently leaves the Board HDC slave ID invalid. |
| Candidate Proposal | Make the exposed fixed-disk count an explicit X86-NXVM construction fact, exactly as floppy-slot count already is. Remove the slot-1 rejection and apply that count consistently in Machine allocation, registry binding, reset, validation and startup insertion. Each existing fixed-machine plan declares its actual one-slot or two-slot HDC topology; MyPC110 alone declares two and binds its already reserved secondary ID. Complete the existing ATA chip's Drive/Head slave branch where stale master-only guards reject it. Preserve the existing master-only fixed-geometry contract. No second controller or per-App media path is added. |
| Files And ABI Surface | Admitted production surface: `src/x86-nxvm/machine/{construction_interface.h,preparation.c,machine_devices.c,machine.c}`, `src/x86-nxvm/chips/hdc/hdc.c`; the four existing one-slot App plans `src/app-{my5160,my5170,mydeskpro386,nxvm}/machine_plan.c`; and `src/app-mypc110/pc110_plan.c`. Admitted test surface: existing `test/x86-nxvm/machine/{preparation_smoke.c,construction_smoke.c}`, `test/x86-nxvm/board-base/board_hdc_smoke.c` and `test/app-mypc110/unit/machine_construction_smoke.c`, their CMake/manifest only if required. Architecture/current/history/evidence updates are admitted. Existing public configuration fields and Board HDC contract are consumed unchanged. |
| Applicable Rules | `docs/nxvm/README.md`; `docs/nxvm/design/{ARCHITECTURE,CODING,UI}.md`; `docs/rules/{EXECUTION,ARCHITECTURE,CODING,DOCUMENT}.md`; `CONTRIBUTING.md`; `docs/nxvm/etc/operations/policy/source-policy.md`; `src/x86-nxvm/README.md`; T580 proposal and convergence ledger. Preserve one media registry and HDC route, inward dependencies, BYOB inputs, existing stable Ninja/ccache graphs and protected external inputs. |
| Verification | Direct tests prove slot 1 is accepted, allocated, bound to the secondary ID, reset and inserted without changing master behavior; MyPC110’s production plan exposes both IDs. Run affected x64/x86 X86-NXVM/MyPC110 units, the existing Board HDC dual-drive tests, and selected real-firmware integrations. No test is called guest/storage/product qualification beyond its actual checkpoint. |
| Expected Markers | Evidence maps every former slot-0-only site to the one counted path and identifies fixed geometry as master-only. Tests distinguish master/slave selection through the real Board HDC, not an App-only duplicate. |
| Asset Needs | Existing source and tests only. External BIOS/media are read-only and optional for integrations; no raw firmware, media, third-party source or Windows component enters the repository or executable. |
| Reporting Requirements | Report actual source/test delta, exact dual-slot behavior, test results, source ownership, and every retained limitation: authentic SunDisk/CF inputs, BIOS storage consumption, PCIC card mapping, selected guest and publication remain unqualified. |
| Stop Conditions | Stop before implementation if the existing Board HDC cannot address the secondary registry ID, if fixed geometry cannot be kept master-only without a new contract, or if a required change crosses the admitted component boundary. Do not fabricate a disk, geometry or boot success. |
| Exit Criteria | Both slots use the one declared Machine/registry/HDC route with no cross-package API expansion, direct x64/x86 proofs pass, actual changes are reviewed and documented, and the worktree is clean or a complete reviewable P remains. T580 and product publication remain open. |
| Original Owner Request | “老规矩实现PC110”, followed by “加油”; owner permits source-labelled L2 model work and autonomous in-scope S admission, but not fabricated firmware/media behavior. |
| Similar-Issue Sweep | Inspect every fixed-disk slot use in Machine, Product forwarding, board HDC master/slave topology and all fixed-machine plans. Do not repair only MyPC110 while retaining a contradictory generic two-slot declaration. |

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
