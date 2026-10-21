# M1 T7 S7 — Bounded Selected-Set Budget Diagnostic

## Scope and evidence grade

The existing selected-set Probe was changed only to make its existing bounded
instruction limit an explicit diagnostic argument.  The default remains
2,000,000 instructions.  A caller may select from 2,000,000 through
20,000,000; no other argument is accepted except the existing held Coin 1 and
Start 1 experiment.

Those limits are **L1 containment policy**, not Raiden II timing constants.
The external-ROM runs below are **L2 observations**.  Neither result proves
physical timing, title/attract progression, accepted coin/start cadence,
graphics fidelity, sound, protection correctness or gameplay.

## Implementation boundary

The Probe parses its own arguments before opening a configured image.  It
prints the requested budget beside the already published aggregate execution,
frame and PCM observations.  The existing PowerShell wrapper passes the same
bounded value only in its Probe mode.  Product mode is unchanged.

The normal default route remains one deterministic two-million-instruction
diagnostic.  The maximum prevents accidental open-ended execution; it does not
create a guest clock, change a Core timing model or alter any Product, Machine
or Board interface.

## Results

The default x64 invocation retained the previous aggregate result:

```text
coin-start-held=0 instruction-budget=2000000 executed=2000000
ticks=33301143 frame-width=320 frame-height=240 nonblack=74795
frame-digest=a06b9c216e1ce034 pcm-frames=4096 pcm-dropped=95807
pcm-nonzero=0 pcm-digest=1d00c08067be8383
```

The invalid `--instruction-budget=0` and duplicate-budget invocations each
emitted only the existing usage marker and exited with code 2 on both x64 and
x86, before any image path was opened.

One five-million-instruction held-control run was performed for each width:

| Host | Stop | Budget / executed | Ticks | Nonblack / frame digest | PCM frames / dropped / nonzero / digest |
| --- | --- | ---: | ---: | --- | --- |
| x64 | budget | 5,000,000 / 5,000,000 | 82,661,662 | 74,863 / `d7e4cff81e7dabcd` | 4,096 / 243,888 / 0 / `1d00c08067be8383` |
| x86 | budget | 5,000,000 / 5,000,000 | 82,661,662 | 74,863 / `d7e4cff81e7dabcd` | 4,096 / 243,888 / 0 / `1d00c08067be8383` |

There was no CPU, COP or Board-control terminal fault on either run.  A
20-million-instruction attempt exceeded this interactive executor's 30-second
observation interval; its owned wrapper eventually removed the temporary stage
and no result from that interrupted observation is retained as evidence.

## Verification and disposition

- The existing Probe builds on x64 and x86.
- Both widths reject the invalid budget as above.
- The external wrapper verified the archive before each run and the diagnostic
  root was empty afterward.
- The next hardware/product receiver remains independent of this budget work;
  an aggregate frame or zero PCM counter cannot be promoted into a gameplay or
  device conclusion.

