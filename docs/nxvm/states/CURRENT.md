# Project Status

## Current Work

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
