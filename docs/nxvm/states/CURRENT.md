# Project Status

## Current Work

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
