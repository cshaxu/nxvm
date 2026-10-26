# Project Status

## Current Work

## M5 T580 S57 - F65535 planar access and programmed colour

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; next numeric S57 after accepted S56, one-session coordinator/executor. |
| Admission And Approval | Owner's standing PC110 completion, automatic in-scope S and source-labelled L2 repair authorization; owner explicitly reports wrong blue/cyan versus pink ERROR/Easy-Setup pages. Necessary Shared chip read-contract change reported before implementation. Lib/Emulator remain excluded. |
| Objective | Complete the retained standard-VGA planar/colour receiver: CPU read latches/compare and four writes, map masking/addressing, programmed Attribute/DAC/PEL scanout and side-effect-free inspection. Verify original BIOS ERROR/Easy-Setup colours through the existing Machine frame, without screen-specific swaps. |
| Non-goals | No host colour inversion, ROM/media edit, decoded payload injection, second renderer, guest clock, worker, pipeline or panel timing claim. Other graphics/panel modes, PCIC, storage/guest and complete product qualification retain their original ledger gates. |
| Reference Baseline | Clean pushed 16aad0e66, accepted S56. Original BIOS Setup/reset works both widths; F65535 currently stores programmable palettes but captures fixed RGBI, and CPU memory copies bytes without VGA latch/rotate/ROP/write modes. |
| Candidate Proposal | ../proposals/m5-pc110-qualification.md and ../etc/evidence/t580-s1-pc110-qualification-ledger.md; S55 full display receiver and owner's explicit colour feedback. |
| Files And ABI Surface | Existing X86-NXVM chips/f65535 implementation/private state/interface, existing chip test and manifests; App PC110 display memory adapter and existing display integration/unit tests as required. Add observe_only to the existing chip memory-read signature and migrate all callers; no new public function/enum/layout. Expected production +180--260/-35--60, existing tests +140--220/-10--25. |
| Applicable Rules | NXVM guide Task Reading Set, CONTRIBUTING, shared EXECUTION/ARCHITECTURE/CODING/DOCUMENT, NXVM design architecture/coding, source-policy, X86-NXVM owner guidance and architecture-governance/coding-governance. Chip owns physical register/latch/VRAM state; App only forwards Core access semantics; copied snapshot is sole scanout. |
| Verification | Read original C&T F65535 reference and pinned reference behavior, extend existing chip fixture for complete read/write/mask/address/colour/inspection variants, use focused dual-width tests before cached full units and receiving regressions. Actual BIOS ERROR and Setup frame must consume programmed colours, not fixed table. Rebuild eight affected admitted PC EXEs and four unpublished previews, verify identities/inputs, run affected original-firmware integration. |
| Expected Markers | Programmed DAC/attribute/mask changes copied colour; four write modes consume the correct hardware latch; inspect returns read data without altering subsequent write behavior. Original BIOS output provides evidence of actual colour consumer, with no hardcoded pink or BIOS PC behavior. |
| Asset Needs | Existing original ct-f65535-register-reference-1994.pdf revision 2.0 and unchanged admitted TFT BIOS/font/media; read-only external source references. No acquisition/import or external write. |
| Reporting Requirements | ../etc/evidence/t580-s57-f65535-planar-colour.md retains sources/grades, actual mechanism, before/after evidence, similar-issue sweep, actual deltas, tests/artifacts and unverified domains. Distinguish ordinary guest output from synthetic tests and physical-panel/RDP verification. |
| Stop Conditions | Scratch only build/t580-s57-video, S57-owned, 16 MiB; archive facts/hash summaries and verify owned task exit before official cleanup. Stable Ninja/ccache graphs and j8 only, hidden background native work. Existing Setup replay native bounds 660 s x64/900 s x86 and registered budgets remain. No speculative screen/firmware shortcut or unchanged failed retry. |
| Exit Criteria | Complete named planar/colour/inspection mechanism reviewed against source and all callers; dual-width focused/full units and affected integrations pass; manifests and eight receiver artifacts current; original BIOS actual colour proof recorded, previews remain unpublished; scoped commits pushed, scratch cleared and actual-change coordinator acceptance recorded. Whole-T original gates remain. |
| Original Owner Request | Continue old-rule PC110 implementation; ERROR and Easy-Setup should show their programmed pink background rather than the current blue/cyan. |
| Similar-Issue Sweep | Both text/mode-12 capture palettes, all CPU aperture addressing/read/write modes and raw VRAM versus inspection callers; font-slot/character-map addressing and programmed register masks at the same VGA boundary. Do not borrow another chip's private implementation or add a parallel scanout. |

Coordinator admits the existing complete colour/planar receiver. Executor
first rechecks original register formulas and the actual App/Core read
contract. No production modification begins before that source review.

The source/dependency review confirms that F65535 is an actual PC110-only
consumer. All relevant stable targets are rebuilt and eight published PC
identities checked; unchanged receiving EXEs do not need a timestamp-only
refresh. Four affected PC110 previews remain unpublished. The existing
32-byte text glyph-slot and A/B font-map semantics are reconciled within
the named text-addressing sweep, without extending the snapshot interface.

The initial S57 candidate passes both 712-unit suites, gates, eight INI
identities and both real BIOS replays; its ERROR/date-time frames use
programmed BE9E96 background. Actual-change review then removes an
unsupported mode-3 logical-operation exemption, sharing the existing GR03
stage across modes 0/2/3 and correcting/extending the same small test table.
Old results remain explicitly initial-candidate evidence. The corrected
source is undergoing complete double-width requalification under the same
budgets; no old pass is counted as final source proof. See S57 evidence
for exact changes, source grades, owned handles and unresolved whole-T gates.
No S57 commit or closure is claimed at this checkpoint.

Final S57 verification is now complete. Actual-change review additionally
reproduces and fixes overflowing aperture lengths at the same preflight
owner; exact final-tree full units, gates, rebuilt previews and dual-width
FDC firmware regressions pass. The unchanged normal-domain colour/boot
replays are retained with their precise before-guard boundary and mathematical
equivalence rationale, rather than relabelled as byte-identical final builds.
All jobs exit, artifact/input hashes are verified, and owned scratch is
ready for reviewed cleanup. Executor delivery awaits scoped submission and
coordinator acceptance; the original whole-T gates stay open.

| Task | Progress |
| --- | --- |
| M5 T580 S56 | Accepted Shared `dde5da6de` and NXVM `88124376c`: Board system-control A20/reset, PC110 real BIOS restart request and receiving qualifications. No active S packet between acceptance and the next admission. T580 remains open. |

## Verified Baseline

Both host widths execute the unchanged TFT BIOS's complete Setup loader and
ordinary Enter/restart request, then reach the actual processor reset vector
FFFF:FFF0 after 144,097,170 instructions and five resets, with four known
exceptions and no Core fault. Board owns the independent KBC/fast A20 inputs
and their OR; the existing Core reset retains Board/RAM/RTC.
This does not establish saved configuration or post-restart guest boot.

All 712 repository units per width pass. Four profile boots, four receiving
PC INI identities and the four affected integrations pass on each width.
Strict compilation retains 498 rows, zero deferred. The
[S56 record](../etc/evidence/t580-s56-system-control.md) preserves actual
diffs, source grades, failed attempts, exact results and limits.

Eight admitted PC EXEs are rebuilt/deployed; four PC110 previews remain
unpublished. The [inventory](../etc/evidence/t580-s56-artifacts.json) records
hashes, PE widths and embedded runtime version 0.5.0580. MyNES/MyRaidenII,
owner INI, external ROM/media and snapshots remain unchanged. All 98 owned
scratch files/7,013,740 bytes are removed after task-exit and protected-file
hash review; stable Ninja graphs and ccache remain.

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and complete
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain normal Setup/configuration persistence, per-slot SunDisk/CF storage
and selected guest, PCIC/card handling, F65535 planar/Attribute/DAC/PEL
scanout and inspection, panel/TFT/DSTN identity, live input/lifecycle,
desktop and four-artifact publication gates.

The owner's ERROR/Easy-Setup pink-versus-blue observation is an explicit
F65535 display acceptance item. Current fixed RGBI scanout ignores the
programmed palette. The next coherent display receiver must repair guest
colour/planar semantics at the chip owner, without a host colour swap or
Lib/Window change. Source-labelled L2 is permitted; no physical-unit dump
or firmware patch is a prerequisite.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained status](../history/M5-T580-retained-status.md) preserves prior
packets as history, not concurrent active work.
[Queue](QUEUE.md) retains unrelated ordered candidates.
