# M1 T7 S19 — Selected-Set Input Pulse Diagnostic

## Scope and grade

S19 extends the existing selected-set Probe only. The old optional input mode
held Coin 1 and Start 1 for the entire run; the new mutually exclusive pulse
mode presses both controls after one quarter of the unchanged budget and
releases them after one half. It uses the existing Board active-low control API
and the existing run loop. It adds no guest-input path, scheduler, production
state, or host automation.

The resulting selected-set observations are **L2**. The 20M instruction cap
is **L1 containment**, not a coverage or timing claim.

## Result

The owner-supplied selected set ran once per host width at the unchanged
20,000,000-instruction cap. Both runs applied the scheduled edge and produced
identical output:

| Field | x64 | x86 |
| --- | ---: | ---: |
| Press / release threshold | 5,000,000 / 10,000,000 | 5,000,000 / 10,000,000 |
| Both transitions applied | yes | yes |
| Executed / ticks | 20,000,000 / 324,372,782 | 20,000,000 / 324,372,782 |
| Final frame digest / nonblack | `baabe5eae2033206` / 74,865 | same |
| Frame transitions | 3 | 3 |
| Sound command bytes at every sample | `0x00`, `0x00` | same |
| Sound interrupt pending at every sample | no | no |
| Copied PCM / nonzero frames | 4,096 / 0 | same |

The Probe parser also rejects `--coin-start-held` combined with
`--coin-start-pulse` before attempting image loading.

## Disposition

The existing Board electrical input route accepts a bounded press/release
sequence deterministically on both widths. This does not establish that the
selected title accepts coin/start, that Window/RDP keyboard input reaches the
Board, or that gameplay has started. The unchanged command, PCM and frame
summaries leave the broader gameplay/title receiver open without inventing a
new emulator path.

## Verification

- Rebuilt the existing Probe in stable x64 and x86 Ninja trees.
- Ran the owner-ROM wrapper once per width; staging was removed by its existing
  `finally` cleanup.
- Ran all 12 registered MyRaidenII unit tests on each width: all passed.
- Ran documentation governance and `git diff --check`.
