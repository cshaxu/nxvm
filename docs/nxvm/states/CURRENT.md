# Project Status

## Current Work

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
