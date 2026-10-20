# M1 T5 S6 — Selected-set PCM Observation

## Scope

The existing selected-set startup probe now makes one copied read from the
existing `myraidenii_core_board_pcm_copy()` route after its bounded V30 run.
It reports only aggregate frame count, dropped count, nonzero-frame count and
a deterministic digest. It neither writes a PCM file nor changes any
production component, external image, timing budget or pass criterion.

## Result

The owner-supplied external archive was staged through the existing launcher
and removed on exit. Both cached probe widths produced the same neutral record:

| Width | Executed | Board ticks | PCM frames | Dropped | Nonzero | PCM digest |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| x64 | 12,721 | 205,420 | 616 | 0 | 0 | `e08f259c1f01e543` |
| x86 | 12,721 | 205,420 | 616 | 0 | 0 | `e08f259c1f01e543` |

The existing probe stopped at its established early guest result
(`reason=7`, `detail=3`) before any observed sound-producing command. The
zero nonzero-frame count therefore documents this bounded route only; it is
not evidence that the selected game has silent hardware, that the L1 mixer is
faithful, or that host playback works.

## Evidence Grade And Boundaries

- The aggregate result is an L1 runtime observation: it records one bounded
  run and makes no hardware claim.
- The probe does not import MAME behavior or tables, retain protected audio,
  produce a raw trace, or create a new Board/Sound path.
- Earlier L3/L2/L1 device claims remain exactly as recorded in S5.

## Verification

The x64 and x86 probe targets rebuilt with cached Ninja trees. The existing
launcher completed both width runs and removed its transient staging tree.
The subsequent S6 closure reruns the complete MyRaidenII unit selection on
both widths and documentation/whitespace checks. No desktop, host audio or
playable-game result is claimed.
