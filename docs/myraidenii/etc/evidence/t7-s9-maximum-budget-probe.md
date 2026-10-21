# M1 T7 S9 — Maximum-Budget Selected-Set Probe

## Scope and grade

S9 reused the one existing owner-ROM staging and Probe route.  The cap of
20,000,000 instructions is **L1 containment policy**.  Both observations below
are **L2** runtime observations: they do not prove hardware timing, title or
attract progression, accepted controls, visual fidelity, audio playback or
gameplay.

No production, test, build, Shared, ROM or asset file changed.  The archive
was read through the existing verifier and the wrapper removed each temporary
stage under `O:\assets\raidenii\myraidenii-diagnostics\` before returning.

## Bounded results

The existing probe ran once per host width with held Coin 1 and Start 1 and
`--instruction-budget=20000000`:

| Host | Reason / executed | Ticks | Frame nonblack / digest | PCM frames / dropped / nonzero / digest |
| --- | ---: | ---: | --- | --- |
| x64 | budget / 20,000,000 | 324,372,782 | 74,865 / `baabe5eae2033206` | 4,096 / 969,022 / 0 / `1d00c08067be8383` |
| x86 | budget / 20,000,000 | 324,372,782 | 74,865 / `baabe5eae2033206` | 4,096 / 969,022 / 0 / `1d00c08067be8383` |

Both invocations reported no CPU fault, COP failure or Board-control failure.
The identical results establish only that the current route remains
deterministic across host widths for this bounded interval.  It does **not**
turn the elapsed instruction count into a guest clock or a game checkpoint.

## Disposition

There is no new terminal receiver to implement from this run.  COPX-D2 remains
separately deferred by S5/S8, and the M1 T7 gameplay/release ledger remains
open for genuine title/attract, controls, audio, stage/end/restart and desktop
qualification.  No additional longer run is scheduled from this result alone:
the maximum budget has done its diagnostic job without identifying a finite
new ownership boundary.

