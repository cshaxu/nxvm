# M1 T7 S30 — Sound Transport Observation

## Scope

S25 reconciled the Board's fixed Sound transport byte lanes.  S29 then showed
that the real Sound program writes only OKI stop commands.  This batch answers
the narrower question left between those facts: does the real Sound program
actually receive and service the existing Board handoff?

## Local observation contract

Sound now counts only successful uses of its existing transport callbacks:

| Counter | Existing owner |
| --- | --- |
| command reads 0/1 | the existing `4010`/`4011` Sound read cases |
| command acknowledgements | the existing `4000` Sound write case |
| transport accepts | the existing RST18 interrupt-accept branch |

The existing Board observation copies these aggregate facts.  No command byte,
register state, ROM content, queue or new handoff is retained.  The existing
Sound unit exercises both command reads and acknowledgement, then separately
exercises RST18 acceptance.  The existing Board unit verifies the copied
aggregate starts and resets cleanly without adding a test-only route.

## Selected-set observation

The existing verified wrapper ran once per host width with its existing
2,000,000-instruction cap.  The archive was read-only and each staging child,
then its empty parent directory, was removed after completion.

| Aggregate | x64 | x86 |
| --- | ---: | ---: |
| Guest ticks | 33,283,441 | 33,283,441 |
| Final frame digest | `a06b9c216e1ce034` | `a06b9c216e1ce034` |
| Sound command-0 / command-1 reads | 138 / 1 | 138 / 1 |
| Sound command acknowledgements | 138 | 138 |
| RST18 transport accepts | 211 | 211 |
| YM address/data writes | 1,135 / 1,135 | 1,135 / 1,135 |
| OKI-1 / OKI-2 writes | 373 / 373 | 373 / 373 |
| OKI sample commands / selections / starts | 0 / 0 / 0 | 0 / 0 / 0 |
| OKI stop commands | 373 per device | 373 per device |
| Generated nonzero PCM | 0 | 0 |

The complete neutral summaries and all frame checkpoints are identical across
host widths.  These are L2 observations of the owner-selected archive; they
do not identify command payload, guest state, title progress, audible output
or physical hardware behavior.

## Disposition

The existing Sound program is not starved of the existing Board handoff: it
reads, acknowledges and accepts it repeatedly.  S25's physical mapping and
the existing interrupt path therefore remain supported by direct local
evidence.  Combined with S29, the stop-only OKI traffic cannot justify a
transport or local decoder rewrite.

The remaining uncertainty is upstream program/game-state qualification: the
bounded boot path has not reached a state that emits sample selections, or a
different already-unmodelled device/behavior controls that state.  A later S
must investigate that boundary without exposing payload or inventing audio.

## Verification

- Focused Sound/Board units passed on x64 and x86.
- The complete MyRaidenII unit label passed 12/12 on x64 and 12/12 on x86.
- The selected-set Probe built and completed once per width with equal neutral
  summaries.
- `git diff --check` passed.  No Shared component, public cross-product API,
  executable, protected media or Assets release changed.
