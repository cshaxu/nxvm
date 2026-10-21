# M1 T7 S34 — Vblank-Aligned Input Sequence

## Scope

S24's MAME L2 black-box run visibly changed after Coin 1 was active on frames
300--329 and Start 1 on frames 360--389. The former local sequence was
relative to an instruction budget, so it was not time-equivalent. This batch
adds one exclusive Probe-only mode that uses the existing Board vblank
observation to deliver the same four threshold edges through the existing
Machine driver.

The thresholds are local vblank counts, not a claim that a local instruction
or host interval equals a MAME frame. No Product input, Board device, renderer,
ROM or Shared behavior changed.

## Control contract

`--coin-start-frame-sequence` is mutually exclusive with the existing held,
pulse and instruction-relative sequence options. Its only edges are:

| Local Board vblank threshold | Existing mapped control | Value |
| ---: | --- | --- |
| 300 | Coin (`5`) | press |
| 330 | Coin (`5`) | release |
| 360 | Start (`1`) | press |
| 390 | Start (`1`) | release |

The Probe polls the existing read-only Board Video aggregate between bounded
CPU quanta; it owns no second clock or input path. Its output identifies the
time unit as `vblank`, prints the thresholds, and preserves the four existing
delivery flags.

## Bounded selected-set result

Each host width ran one no-input baseline and one vblank-aligned sequence with
the existing 20,000,000-instruction containment cap. Every run terminated at
the budget without CPU, COP or Board-control fault. x64 and x86 match exactly
within each case.

| Aggregate | Baseline | Vblank sequence |
| --- | ---: | ---: |
| Executed / guest ticks | 20,000,000 / 324,088,982 | 20,000,000 / 324,088,505 |
| Final digest / nonblack | `ccad0e0867e39b9c` / 74,865 | `ccad0e0867e39b9c` / 74,865 |
| Frame samples / transitions | 4 / 3 | 4 / 3 |
| Vblank events | 1,122 | 1,122 |
| CRTC writes / sprite latches | 13,052 / 1,077 | 13,052 / 1,077 |
| Palette / tilemap commits | 1,077 / 1,077 | 1,077 / 1,077 |
| Coin/Start delivery flags | 0 / 0 / 0 / 0 | 1 / 1 / 1 / 1 |

The four sampled frame digests and nonblack counts are also equal between the
two cases. Sound transport totals vary slightly with the different guest work
around input delivery, but both remain stop-only OKI activity with zero sample
commands, voice selections, starts and nonzero PCM.

## Disposition

The prior local no-visible-change result is not explained by an
instruction-relative Coin/Start timing mismatch. The local sequence now uses
the same named reference-frame ordering and still has no bounded frame-summary
difference. This remains a diagnostic result, not proof of a missing hardware
receiver or of an external/local emulator discrepancy: frame digest cannot
identify a credit, title, gameplay or visible-pixel state.

The next receiver remains T7's explicit selected-set gameplay qualification:
obtain a bounded, non-payload checkpoint that can distinguish the relevant
local presentation/control state before assigning another Board, COPX, Video
or Sound implementation change. No guessed repair is admitted here.

## Verification and cleanup

- Probe builds pass on stable x64 and x86 Ninja trees.
- The mutually exclusive mode pair returns the existing usage failure on both
  widths.
- Full MyRaidenII unit label passes 12/12 on each width.
- The wrapper removed all external staging after the four runs. No raw ROM,
  RAM, trace, screenshot or log is retained.
- `git diff --check` passed.

