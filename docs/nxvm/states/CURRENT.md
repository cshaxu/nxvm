# Project Status

## Current Work

## M5 T580 S56 - System-control A20 and processor reset

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; numeric S56 after accepted S55, one-session coordinator/executor. |
| Admission And Approval | Owner's standing instruction to implement PC110 through T closure, automatic in-scope S admission and source-labelled L2 repair approval. The necessary public Board configuration field and complete mechanism were reported before implementation. Lib/Emulator remain excluded; no new CPU operation or public function/enum is proposed. |
| Objective | Consume S55's complete system-control/A20/reset receiver: real 92h decode, independent KBC/fast A20 inputs, rising reset request and cold-versus-processor-only retention. Connect PC110 and verify original Setup's save/restart request through the existing Core reset owner. |
| Non-goals | No BIOS-PC hook, forced PC/CMOS success, firmware/media edit, new reset executor, clock, pulse timer, queue or recovery framework. Do not claim selected storage/guest, PCIC, panel/DSTN or product publication from a successful reset. |
| Reference Baseline | Clean pushed b2d7942b7; accepted S55 implementation 6964e8280. Both host widths reproduce the unchanged BIOS 92h request followed by IF-clear HLT; KBC ACK/BAT/ID and all 712 units per width are verified. |
| Candidate Proposal | ../proposals/m5-pc110-qualification.md, ../etc/evidence/t580-s1-pc110-qualification-ledger.md and S55's final failure-class disposition; system-control is the next earliest Board/App receiver, not a new T-level completion gate. |
| Files And ABI Surface | X86-NXVM Board config appends lib_bool system_control_port_present, with one existing port route/handler and two private physical latches. Existing PC/AT decode remains; explicit presence selects the same route on a neutral board. PC110's two constructors opt in. Expected source +50--90/-10--25, existing test extensions +70--120/-5--20; update source/test manifests and receiving artifacts. No Lib/Emulator, CPU/FPU or unrelated App implementation change. |
| Applicable Rules | NXVM guide Task Reading Set, CONTRIBUTING, shared EXECUTION/ARCHITECTURE/CODING/DOCUMENT, NXVM architecture/coding, source-policy and X86-NXVM owner guidance; architecture-governance then coding-governance. One Board-owned effective A20 path, existing Core reset, declared L2 behavior and immutable external inputs. |
| Verification | Extend existing Board instance/neutral-controller and PC110 composition/construction tests. Cover independent gate readback/OR, reserved bits, reset edge/rearm, CPU-only RAM/device retention, cold reset, absent decode and invalid presence. Run focused dual-width units before full cached builds/units and affected integration. Rebuild/deploy eight admitted PC EXEs, rebuild four unpublished previews, verify PE/version/identity and unchanged owner INI. Original BIOS Setup/restart replay uses the existing harness; no decoded payload injection. |
| Expected Markers | Direct port and actual OUT92 code request CPU reset before terminal HLT, preserving board/RAM/CMOS ownership. Original BIOS's observed save/restart request must leave ABC2 through real reset, or the full remaining class must have an evidenced receiver without a false save/guest claim. |
| Asset Needs | Existing admitted TFT BIOS/font and immutable external media; no acquisition/import or external repository write. Source pins and input hashes are retained from S55 and rechecked for actual replay. |
| Reporting Requirements | Record before-change design, public surface, sources/model grade, each shared/receiving diff, exact tests/artifacts and unresolved T580 gates in ../etc/evidence/t580-s56-system-control.md. Report actual source/test delta, retained owners, no new framework and unexecuted desktop/RDP/panel/guest coverage. |
| Stop Conditions | Diagnose before repair; no speculative register success. Scratch only build/t580-s56-control, S56-owned, 12 MiB; archive facts/hashes and stop owned tasks before safe cleanup. Reuse stable Ninja/ccache graphs and j8, background hidden native work. Real replay has the measured 660 s x64/900 s x86 full Setup containment; a necessary additional restart phase needs a prior evidence-based packet revision. No unchanged failed retry or silent budget relaxation. |
| Exit Criteria | Complete system-control mechanism and all callers/variants reviewed; focused and required double-width regressions pass, manifests current, eight admitted receiving EXEs deployed/verified, four previews remain unpublished, external/owner inputs unchanged, actual-diff review and cleanup complete, scoped P commits pushed and coordinator acceptance recorded. Original T580 gates remain explicit. |
| Original Owner Request | Continue PC110 implementation through qualified product completion, using justified external-emulator L2 where necessary, without repeated requests for in-scope authorization. |
| Similar-Issue Sweep | Audit every Board write to the effective address-clear mask, existing 92h users and KBC output, reset initialization versus processor-only reset, route presence/freeze and old App assumptions. Replace last-writer-wins at its owner; do not create a separate PC110 reset/A20 handler. |

The directly affected family A20/fallback static gate is also updated:
replace its obsolete effective-line readback assertion with independent
system-control readback and enforce one Core mask publication. Estimated
check delta +8/-1, no new verifier or framework.

Coordinator admits this bounded receiver. Executor confirms the existing
Core reset changes only processor state and retains Board/RAM/RTC; no new
Core contract is needed. Two private Board fields represent physical input
latches, not mirrored CPU state. The one Board helper derives the effective
mask, and the existing route consumes it. The reset edge uses the same
executor boundary as the already wired KBC reset. The model is explicitly
functional L2, not physical pulse timing. S55's remaining receiver class
stays unchanged until its own direct proof.

| Task | Progress |
| --- | --- |
| M5 T580 S55 | Accepted loader/keyboard ordering and complete observed failure-class disposition in NXVM `6964e8280`. No active S packet between acceptance and the next admission. T580 remains open; the original product gates are unchanged. |

## Verified Baseline

Both host widths execute the original TFT BIOS through retained wake/RSM,
the complete 232,185-byte Setup loader and ordinary Enter input. The App's
existing timed KBC response selection removes 00301 with no Shared/API change.
Both stop at the same original software-reset request after 144,097,172
instructions, four resets, four known exceptions and no Core fault. No
configuration-save, selected guest or normal Setup claim follows that stop.

All 712 repository units pass per width. Registered FDC reset and real boot
handoff pass without changed budgets: 52.29/236.77 s x64, 54.92/352.18 s x86.
The exact source grades, negative control, failed attempts and result hashes
remain in [S55 evidence](../etc/evidence/t580-s55-ata-guest-route.md).

All ten published identities from S54 remain exact, and MyNES, owner INI,
ROM/media and snapshots are unchanged. Four PC110 TFT/DSTN previews are
rebuilt but unpublished; [S55 inventory](../etc/evidence/t580-s55-previews.json)
records their PE widths and hashes. S55 changes only three App configuration
files and three existing tests, +310/-23 C/H lines, with production +6/-0.
All 128 owned scratch files/14,332,369 bytes are safely removed; stable
Ninja graphs and ccache remain.

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and complete
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain all original guest, controller, display/panel, live input, DSTN,
lifecycle/desktop and four-artifact gates. S55's complete route disposition
names the next owners: Board system-control/A20/reset and App presence,
Machine per-slot SunDisk/CF storage, PCIC versus card insertion, and F65535
planar/DAC/inspection versus panel identity. No remaining member is hidden
by the loader or keyboard proof.

The next coherent receiver is the actual software-reset/A20 mechanism and
ordinary guest configuration/restart path, followed by the retained storage,
PCIC and video receivers. Source-labelled L2 is permitted; no firmware patch,
forced PC/CMOS success or physical-unit dump prerequisite is introduced.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained status](../history/M5-T580-retained-status.md) preserves prior
packets as history, not concurrent active work.
[Queue](QUEUE.md) retains unrelated ordered candidates.
