# M1 T5 S14 — Control-Input Sound Diagnostic

## Scope And Grade

S14 uses the already public Board control operation in the existing selected-set
startup probe.  The normal probe remains unchanged by default.  Its new opt-in
`--coin-start-held` diagnostic holds Coin 1 and Start 1 from Board creation
through the declared instruction budget.  This is not a product control path,
a physical switch-timing claim, or a retained input script.

The Board's active-low Coin 1 and Start 1 mapping remains the existing **L2**
board-map fact recorded in `t3-s3-interrupt-input-sound.md`.  Holding both
controls for the whole diagnostic is an **L1** experiment: no inspected primary
manual PDF supplies an applicable boot-time pulse duration, and no external
reference is used to claim that this is how an operator presses either switch.
No L3 formula, constant, register rule, ROM payload, trace, screen or sample
data is adopted.

## Result

Both widths ran the existing bounded probe against the owner-supplied external
set.  The staging tool verified its existing inventory, staged only the runtime
images in a disposable task directory, and removed that directory afterward.
The two runs produced the same neutral aggregate result:

| Host | Stop | Executed | Ticks | Coin/start held | PCM frames | Dropped | Nonzero PCM |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| x64 | budget | 2,000,000 | 33,301,143 | 1 | 4,096 | 95,807 | 0 |
| x86 | budget | 2,000,000 | 33,301,143 | 1 | 4,096 | 95,807 | 0 |

The run has no CPU fault, COP failure or Board-control failure observation.
It establishes only that this one held-control experiment does not make the
existing bounded probe emit nonzero PCM.  It does not establish selected-set
startup, accepted coin/start cadence, valid sound-chip output, host playback,
Window presentation, controls or gameplay.

## Ownership And Transfer

The existing Main-to-Sound latch route is still the sole Board owner, and the
private Z80/device/PCM route is still the sole Sound owner.  S14 adds no
production behavior, state, interface or second path.  The new optional probe
argument is a diagnostic-only invocation of the public Board control API.

The next useful receiver is the queued Product/Machine configuration and real
input route: it must define lifetime-correct user input, lifecycle and Window
behavior before any real command-path audio conclusion.  T5 remains open for
later qualified sound work; this diagnostic neither expands the Sound model
nor treats zero PCM as a component failure.

## Verification

- Cached Ninja builds: `myraidenii-selected-set-startup-probe` for x64 and x86.
- External bounded diagnostic: one `--coin-start-held` run per width, with the
  aggregate result above and cleanup by the existing launcher.
- Repository-only unit suite: 8/8 passed on x64 and 8/8 passed on x86.
- `Verify-DocumentationGovernance.ps1 -Product myraidenii` and
  `git diff --check` passed.
