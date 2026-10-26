# Project Status

## Current Work

No S is active. M5 T580 remains open after accepted S53; this is not a
completed PC110 product or permission to bypass its remaining gates.

| Task | Progress |
| --- | --- |
| T580 S53 | Accepted bounded EC/power/retained-wake receiver: Shared `85a65c3a8`, NXVM `a8561555e`. Source/test and receiving artifacts are pushed; the closure governance commit records coordinator acceptance. |

## Verified Baseline

S53 adds one supported App EC reply/RAM receiver, Chip-owned suspend/button/
resume-event semantics and optional copied Machine physical-key wiring.
Both widths execute the unchanged BIOS through real F12 driver input,
retained processor reset, the same 512 CPU-save bytes and actual RSM. The
registered power integration passes in 186.98 s x64 and 248.64 s x86.
All 712 distinct repository units per width have passing results: the initial
parallel static-test timeouts and unchanged-budget serial corrections remain
explicit in the [S53 evidence](../etc/evidence/t580-s53-power-receiver.md).
Original FDC/timer/ASMI regression and all four deployed INI identities pass
on each width. No whole-tree/desktop qualification is inferred from them.

The eight current PC artifacts retain version 0.5.0580 under
`assets/{my5160,my5170,mydeskpro386,nxvm}/`, with their existing lowercase
INI files unchanged. The artifact target is `vm-0-5-0580`. Four rebuilt PC110
TFT/DSTN previews stay unpublished in the stable Ninja graphs. Lib, Emulator,
X86 Debug, X86 IBM-PC, MyNES and MyRaidenII source/test/artifacts are unchanged.
All 28 S53 temporary files/3,011,103 bytes are removed; both Ninja graphs and
ccache are retained. Working-tree and remote state are checked after closure.

## Remaining T580 Qualification

The [proposal](../proposals/m5-pc110-qualification.md) and its complete
[convergence ledger](../etc/evidence/t580-s1-pc110-qualification-ledger.md)
retain original Setup/boot, selected storage/guest, front panel/live input,
lifecycle, source-grade and publication gates. They are not replaced by a
passing wake transaction or a new terminal PC. The next coherent batch
reconciles the real post-restoration Setup/boot route before selecting any
further receiver. Source-labelled L2 remains permitted; no physical-unit dump
is imposed as a blanket prerequisite.

[Task history](../history/M5-T580-pc110-qualification.md) records acceptance.
[Retained prior status](../history/M5-T580-retained-status.md) preserves the
earlier packets, failures and baselines as history, not simultaneous active
tasks. [Queue](QUEUE.md) retains unrelated ordered candidates.
