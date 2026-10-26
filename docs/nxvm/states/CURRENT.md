# Project Status

## Current Work

## M5 T580 S55 — Actual ATA bootstrap and selected guest route

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; next numeric S55 after accepted S54, one-session coordinator/executor. |
| Admission And Approval | Owner's standing instruction to implement PC110 through T closure, automatic in-scope S admission and source-labelled L2 repair approval. Shared Lib/Emulator remain excluded; a necessary public-contract change must be reported before implementation. |
| Objective | Consume the ledger's selected storage/guest batch through the unchanged TFT BIOS, actual configured ATA medium, BIOS disk/bootstrap services and a named DOS checkpoint; reconcile all observed controller, CMOS, boot selection, sector and terminal failure paths before selecting a repair. |
| Non-goals | No ROM edits, BIOS interception, host boot-sector injection, forced CMOS success/signature, synthetic INT13/19, new executor/clock, copied third-party code, unrelated App or Shared changes, PC110 publication or claim that one guest proves every storage/PCMCIA mode. |
| Reference Baseline | Clean pushed 5e85b10de; S54 proves real retained wake/RSM, ordinary POST and INT19 on both widths, with original T580 product gates intact. |
| Candidate Proposal | ../proposals/m5-pc110-qualification.md and ../etc/evidence/t580-s1-pc110-qualification-ledger.md; reconcile the existing App ATA topology and selected external guest with the original BIOS service route. |
| Files And ABI Surface | Existing App BIOS harness and NXVM packet/evidence; initial estimates and each revision remain in S55 evidence. Diagnostic footprint expected +240--300/-15--35. The real keyboard transcript now admits a PC110-only construction repair: pc110_plan.c, pc110_machine.c and pc110_clock.h select the existing timed KBC response, expected +8--12/-2--4 production lines. Extend existing base-composition/machine-construction tests, expected +20--60/-0--8. No Shared implementation, public ABI or new input/timing path. |
| Applicable Rules | NXVM Task Reading Set, CONTRIBUTING, shared EXECUTION/ARCHITECTURE/CODING/DOCUMENT, NXVM design ARCHITECTURE/CODING and source-policy; architecture-governance then coding-governance skills. One execution/media owner, protected immutable masters and explicit source grades. |
| Verification | Validate selected external media against its owner manifest; use existing Lib Storage snapshot mode without changing the master. Reuse the single BIOS harness, actual Core execute watchpoints, public copied frame/debug observations and existing driver input. Begin one bounded x64 diagnostic after observer review; compare x86 when a meaningful functional predicate is established. Complete affected owner tests/static checks and required repository units before acceptance; rebuild receiving artifacts only if production changes. |
| Expected Markers | Real ATA/bootstrap-sector execution with independently checked bytes and a selected DOS prompt or a complete evidenced failure-class disposition. A further BIOS PC, elapsed budget or medium-open success is not guest qualification. |
| Asset Needs | Existing admitted TFT BIOS/font and owner-provided media-nxvm/hdd_50m_win31.img, verified against its external manifest. No acquisition, master writes, protected payload commitment or external repository modification. |
| Reporting Requirements | Record selected medium hash/geometry, actual service/sector/guest evidence, complete route disposition, planned/actual diff, regressions, unchanged inputs and unexecuted Setup/panel/desktop/other-storage gates. Evidence: ../etc/evidence/t580-s55-ata-guest-route.md. |
| Stop Conditions | Diagnose before changing production; conflicting sources stop speculative repair, not independent research. Scratch only build/t580-s55-guest, S55-owned, 16 MiB. Initial and failed budgets remain in evidence. Timed-response full UI/input replay uses 660 s x64/900 s x86: genuine Setup handoff within 120M instructions, then 20M initialization and two ordinary Enter/10M follow-through stages. The previous 110M ceiling terminated the original loader in progress; the new ceiling includes its measured 9.82M work after the later POST path. Native bounds scale the observed 475 s/132M workload, not guest timing. These are containment, never success predicates. No unchanged retry after timeout, fake success or external medium writes. |
| Exit Criteria | Complete selected ATA/BIOS/bootstrap/guest batch has source, owner and actual-execution disposition; any admitted repair has direct and production-chain regression, required double-width tests, diff review, safe cleanup, scoped commit/push and coordinator acceptance. All other original T580 gates stay named and open. |
| Original Owner Request | Continue implementing PC110 to its qualified product end state, using justified emulator-reference L2 where exact hardware evidence is unavailable, without repeated requests for in-scope authorization. |
| Similar-Issue Sweep | Reconcile absent/present media, ATA identify/task-file width/IRQ, BIOS disk enumeration/CHS/bootstrap, ordinary reset versus retained wake, no-medium and error returns as one batch. Do not add a guessed controller/CMOS reply for a single observed instruction. |

Coordinator admits S55 under the unchanged T580 scope. Executor confirms
that only the admitted App KBC configuration changes production; public
contracts and owner inputs remain unchanged. Detailed
observations, superseded hypotheses, source hashes and failed budgets belong
in [S55 evidence](../etc/evidence/t580-s55-ata-guest-route.md), not a second
active packet.

Both host widths now execute the original loader to 5000:0000 with all
232,185 bytes matching the comparison-only reference. The nominal L2 100 us
response fixes the actual early-ACK/BAT/ID ordering and removes 00301 from
the real Setup error list. The zero-delay negative control fails as intended;
deadline-driven keyboard/AUX delivery and all eleven App units pass.

Two ordinary Enter make/break pairs reach Date/Time and the original BIOS
software-restart request. Both widths stop at the same IF-clear HLT after
writing the missing 92h port: 144,097,172 instructions, four resets, four known
exceptions and no Core fault. This is not saved-configuration or guest-boot
qualification. The retained images and transcripts have independently checked
hashes in S55 evidence; all prior failed budgets and observer mistakes remain.

All 712 repository units per width pass. Four unpublished previews are rebuilt
and inventoried; published Apps and owner inputs remain unchanged. x64's
registered FDC/boot-handoff integrations pass in both widths. Coordinator
actual-diff review and owned scratch cleanup follow the completed results.
No Shared file, manifest, public API, second clock or queue changes in S55.

The complete remaining failure class is explicitly retained at T580 receivers:
Board-owned system control/A20/reset and App presence; Machine per-slot
storage and real SunDisk/CF selection; empty PCIC versus inserted cards;
F65535 planar/DAC/inspection and independent panel identity. All original
guest, lifecycle, DSTN and four-product publication gates remain unaccepted.

| Task | Progress |
| --- | --- |
| M5 T580 S54 | Accepted ordinary post-restoration POST/bootstrap batch: Shared `48fe7a544`, NXVM `d7fd63518`, MyRaidenII receiving `47a4c9092`. No active S packet between acceptance and the next admission. T580 remains open. |

## Verified Baseline

S54 corrects the CPU-owned 486SX absent-FPU ESC class and successful-ESC
L2 cost allocation without an FPU, firmware shortcut, public interface or
Lib/Emulator change. Both host widths execute the unchanged TFT BIOS through
actual suspend/wake, the same 512 retained CPU bytes, RSM and ordinary POST
to real INT19 F000:E6F2: 56,679,591 instructions, four processor resets and
the same four delivered exceptions.

Registered boot-handoff tests pass in 224.16 s x64 / 302.96 s x86 under
the measured 360 s whole-route budget; earlier 300 s failures remain recorded.
Existing FDC/timer/ASMI tests pass both widths. All 712 distinct repository
units per width have passing evidence, including the initial x64 executor
assertion failure and its corrected retest, not a fictional all-green first run.
See [S54 evidence](../etc/evidence/t580-s54-post-restoration-route.md).

The eight PC artifacts retain 0.5.0580; the two receiving MyRaidenII
artifacts retain 0.0.0007. PE widths, versions, hashes and all eight PC INI
identity tests are verified in the [artifact inventory](../etc/evidence/t580-s54-artifacts.json).
Four PC110 TFT/DSTN previews are rebuilt but unpublished. MyNES artifacts,
owner INI, external firmware/media and snapshots remain unchanged. Only
Shared X86 Core production/test and the existing NXVM BIOS harness change;
no other Shared production package or App production code changes.

Actual C/H +342/-32 (net +310) and App test CMake +6/-5 are reviewed.
All 41 S54 scratch files/399,400 bytes are removed with the official cleaner;
the original Ninja graphs and ccache remain. Final incremental builds report
no work on both widths. No desktop, RDP, DOS/Setup or whole-T qualification
is inferred from these results.

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and complete
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain real Setup/loading/UI, selected storage/guest, display/panel/live input,
original/derived DSTN identity, lifecycle/desktop and final four-artifact
publication gates. Those rows remain unaccepted at their original owners.
The next coherent batch must consume the actual bootstrap/storage or Setup
route, not invent a new miniature BIOS-PC completion gate. Source-labelled
L2 remains permitted; no physical-unit dump is a blanket prerequisite.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained status](../history/M5-T580-retained-status.md) preserves prior
packets and attempts as history, not concurrent active work.
[Queue](QUEUE.md) retains unrelated ordered candidates.
